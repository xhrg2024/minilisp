#include "./graphics.h"

#include <Windows.h>
#include <windowsx.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "./error.h"
#include "./lisp_utils.h"
#include "./value.h"

namespace {

/*
 * 图形模块内部实现命名空间。
 * 这里封装 Win32 窗口状态、双缓冲资源、事件队列以及参数转换工具，避免把
 * 平台相关细节暴露到解释器其他模块。
 */

using LispUtils::expectInt;
using LispUtils::expectString;
using LispUtils::isFalseValue;
using LispUtils::makeBool;
using LispUtils::makeList;
using LispUtils::makeNil;
using LispUtils::makeNumber;
using LispUtils::makeSymbol;
using LispUtils::requireArgsSize;

/*
 * 窗口客户区中的整数坐标点。
 * 图形绘制函数和鼠标事件都会使用该结构传递 x/y 坐标，避免在内部接口中
 * 反复传递含义不明确的两个整数。
 */
struct Point {
    int x = 0;
    int y = 0;
};

/*
 * 二维尺寸结构。
 * 用 width/height 明确表示宽高，区分于 Point 的绝对坐标语义。
 */
struct Size {
    int width = 0;
    int height = 0;
};

/*
 * 图形窗口事件。
 * name 会转换为 Lisp 符号，payload 保存已经构造成 ValuePtr 的事件参数，
 * 最终通过 graphics-poll-event 或 graphics-wait-event 返回给脚本。
 */
struct GraphicsEvent {
    std::string name;
    std::vector<ValuePtr> payload;
};

/*
 * Win32 窗口生命周期状态。
 * hwnd 保存窗口句柄，ready 表示 UI 线程是否完成创建流程，open 表示窗口当前
 * 是否仍处于可用状态。
 */
struct WindowState {
    HWND hwnd = nullptr;
    bool ready = false;
    bool open = false;
};

/*
 * 离屏双缓冲绘图资源。
 * Lisp 的绘图过程先画到 backDc 上，graphics-refresh 再触发窗口重绘，一次性
 * 拷贝到真实窗口，减少频繁擦除导致的闪烁。
 */
struct BackBuffer {
    HDC backDc = nullptr;
    HBITMAP backBitmap = nullptr;
    HBITMAP oldBitmap = nullptr;
    int width = 0;
    int height = 0;
    COLORREF drawColor = RGB(0, 0, 0);
};

/*
 * 图形子系统的完整共享状态。
 * mutex 保护窗口、缓冲区和事件队列；条件变量用于等待窗口创建完成或新事件
 * 到达；uiThread 独立运行 Win32 消息循环。
 */
struct GraphicsState {
    std::mutex mutex;
    std::condition_variable readyChanged;
    std::condition_variable eventChanged;
    std::queue<GraphicsEvent> events;
    std::thread uiThread;
    WindowState window;
    BackBuffer buffer;
};

/*
 * 单例图形状态。
 * 当前解释器一次只维护一个图形窗口，因此用文件内静态状态集中管理资源。
 */
GraphicsState gGraphics;

void requireWindowOpen() {
    std::scoped_lock lock(gGraphics.mutex);
    if (!gGraphics.window.open || gGraphics.window.hwnd == nullptr ||
        gGraphics.buffer.backDc == nullptr) {
        throw LispError("graphics window is not open.");
    }
}

COLORREF expectColor(const std::vector<ValuePtr>& args, std::size_t start,
                     const char* name) {
    auto r = std::clamp(expectInt(args[start], name), 0, 255);
    auto g = std::clamp(expectInt(args[start + 1], name), 0, 255);
    auto b = std::clamp(expectInt(args[start + 2], name), 0, 255);
    return RGB(r, g, b);
}

Point expectPoint(const std::vector<ValuePtr>& args, std::size_t start,
                  const char* name) {
    return {
        expectInt(args[start], name),
        expectInt(args[start + 1], name),
    };
}

Size expectSize(const std::vector<ValuePtr>& args, std::size_t start,
                const char* name) {
    return {
        expectInt(args[start], name),
        expectInt(args[start + 1], name),
    };
}

void pushEvent(std::string name, std::vector<ValuePtr> payload = {}) {
    {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.events.push(
            GraphicsEvent{std::move(name), std::move(payload)});
    }
    gGraphics.eventChanged.notify_all();
}

std::optional<GraphicsEvent> popEventLocked() {
    if (gGraphics.events.empty()) {
        return std::nullopt;
    }
    auto event = std::move(gGraphics.events.front());
    gGraphics.events.pop();
    return event;
}

ValuePtr eventToValue(GraphicsEvent&& event) {
    std::vector<ValuePtr> values{makeSymbol(event.name)};
    values.insert(values.end(), event.payload.begin(), event.payload.end());
    return makeList(values);
}

std::string mouseButtonName(UINT message) {
    if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) {
        return "right";
    }
    if (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP) {
        return "middle";
    }
    return "left";
}

LRESULT CALLBACK graphicsWndProc(HWND hwnd, UINT message, WPARAM wParam,
                                 LPARAM lParam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(hwnd, &paint);
            {
                std::scoped_lock lock(gGraphics.mutex);
                if (gGraphics.buffer.backDc != nullptr) {
                    BitBlt(dc, 0, 0, gGraphics.buffer.width,
                           gGraphics.buffer.height, gGraphics.buffer.backDc, 0,
                           0, SRCCOPY);
                }
            }
            EndPaint(hwnd, &paint);
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            SetFocus(hwnd);
            pushEvent("mouse-down", {
                                        makeNumber(GET_X_LPARAM(lParam)),
                                        makeNumber(GET_Y_LPARAM(lParam)),
                                        makeSymbol(mouseButtonName(message)),
                                    });
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            pushEvent("mouse-up", {
                                      makeNumber(GET_X_LPARAM(lParam)),
                                      makeNumber(GET_Y_LPARAM(lParam)),
                                      makeSymbol(mouseButtonName(message)),
                                  });
            return 0;
        }
        case WM_MOUSEMOVE: {
            if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) != 0) {
                pushEvent("mouse-move", {
                                            makeNumber(GET_X_LPARAM(lParam)),
                                            makeNumber(GET_Y_LPARAM(lParam)),
                                        });
            }
            return 0;
        }
        case WM_KEYDOWN: {
            pushEvent("key-down", {makeNumber(static_cast<double>(wParam))});
            return 0;
        }
        case WM_CLOSE:
            pushEvent("close");
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY: {
            std::scoped_lock lock(gGraphics.mutex);
            gGraphics.window.open = false;
            gGraphics.window.hwnd = nullptr;
            gGraphics.eventChanged.notify_all();
            PostQuitMessage(0);
            return 0;
        }
        default: return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

void cleanupBackBuffer() {
    if (gGraphics.buffer.backDc != nullptr) {
        SelectObject(gGraphics.buffer.backDc, gGraphics.buffer.oldBitmap);
        DeleteObject(gGraphics.buffer.backBitmap);
        DeleteDC(gGraphics.buffer.backDc);
        gGraphics.buffer.backDc = nullptr;
        gGraphics.buffer.backBitmap = nullptr;
        gGraphics.buffer.oldBitmap = nullptr;
    }
}

void uiThreadMain(int width, int height, std::string title) {
    HINSTANCE instance = GetModuleHandle(nullptr);
    constexpr const char* className = "MiniLispGraphicsWindow";

    WNDCLASSA wc{};
    wc.lpfnWndProc = graphicsWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    RECT rect{0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExA(
        0, className, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
        CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr,
        nullptr, instance, nullptr);
    if (hwnd == nullptr) {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.window.ready = true;
        gGraphics.window.open = false;
        gGraphics.readyChanged.notify_all();
        return;
    }

    HDC windowDc = GetDC(hwnd);
    HDC backDc = CreateCompatibleDC(windowDc);
    HBITMAP bitmap = CreateCompatibleBitmap(windowDc, width, height);
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(backDc, bitmap));
    ReleaseDC(hwnd, windowDc);

    RECT clearRect{0, 0, width, height};
    FillRect(backDc, &clearRect,
             static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    SetBkMode(backDc, TRANSPARENT);

    {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.window.hwnd = hwnd;
        gGraphics.buffer.backDc = backDc;
        gGraphics.buffer.backBitmap = bitmap;
        gGraphics.buffer.oldBitmap = oldBitmap;
        gGraphics.buffer.width = width;
        gGraphics.buffer.height = height;
        gGraphics.window.ready = true;
        gGraphics.window.open = true;
    }
    gGraphics.readyChanged.notify_all();

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    std::scoped_lock lock(gGraphics.mutex);
    cleanupBackBuffer();
    gGraphics.window.open = false;
    gGraphics.window.hwnd = nullptr;
}

void invalidateWindow() {
    HWND hwnd = nullptr;
    {
        std::scoped_lock lock(gGraphics.mutex);
        hwnd = gGraphics.window.hwnd;
    }
    if (hwnd != nullptr) {
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

void closeWindow() {
    HWND hwnd = nullptr;
    {
        std::scoped_lock lock(gGraphics.mutex);
        hwnd = gGraphics.window.hwnd;
    }
    if (hwnd != nullptr) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
    if (gGraphics.uiThread.joinable()) {
        gGraphics.uiThread.join();
    }
}

}  // namespace

ValuePtr graphicsOpen(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 3, "graphics-open");
    auto width = expectInt(args[0], "graphics-open expects numeric width.");
    auto height = expectInt(args[1], "graphics-open expects numeric height.");
    auto title = expectString(args[2], "graphics-open expects a string title.");
    if (width <= 0 || height <= 0) {
        throw LispError("graphics-open expects positive width and height.");
    }

    closeWindow();

    {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.window.ready = false;
        while (!gGraphics.events.empty()) {
            gGraphics.events.pop();
        }
    }

    gGraphics.uiThread =
        std::thread(uiThreadMain, width, height, std::move(title));
    {
        std::unique_lock lock(gGraphics.mutex);
        gGraphics.readyChanged.wait(lock,
                                    [] { return gGraphics.window.ready; });
        if (!gGraphics.window.open) {
            lock.unlock();
            if (gGraphics.uiThread.joinable()) {
                gGraphics.uiThread.join();
            }
            throw LispError("failed to create graphics window.");
        }
    }
    return makeBool(true);
}

ValuePtr graphicsClose(const std::vector<ValuePtr>&, EvalEnv&) {
    closeWindow();
    return makeNil();
}

ValuePtr graphicsClear(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 3, "graphics-clear");
    requireWindowOpen();
    auto color =
        expectColor(args, 0, "graphics-clear expects numeric RGB values.");
    {
        std::scoped_lock lock(gGraphics.mutex);
        HBRUSH brush = CreateSolidBrush(color);
        RECT rect{0, 0, gGraphics.buffer.width, gGraphics.buffer.height};
        FillRect(gGraphics.buffer.backDc, &rect, brush);
        DeleteObject(brush);
    }
    return makeNil();
}

ValuePtr graphicsColor(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 3, "graphics-color");
    auto color =
        expectColor(args, 0, "graphics-color expects numeric RGB values.");
    std::scoped_lock lock(gGraphics.mutex);
    gGraphics.buffer.drawColor = color;
    return makeNil();
}

ValuePtr graphicsLine(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 4, "graphics-line");
    requireWindowOpen();
    auto from =
        expectPoint(args, 0, "graphics-line expects numeric coordinates.");
    auto to =
        expectPoint(args, 2, "graphics-line expects numeric coordinates.");
    {
        std::scoped_lock lock(gGraphics.mutex);
        HPEN pen = CreatePen(PS_SOLID, 1, gGraphics.buffer.drawColor);
        auto oldPen = SelectObject(gGraphics.buffer.backDc, pen);
        MoveToEx(gGraphics.buffer.backDc, from.x, from.y, nullptr);
        LineTo(gGraphics.buffer.backDc, to.x, to.y);
        SelectObject(gGraphics.buffer.backDc, oldPen);
        DeleteObject(pen);
    }
    return makeNil();
}

ValuePtr graphicsRect(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 5, "graphics-rect");
    requireWindowOpen();
    auto origin =
        expectPoint(args, 0, "graphics-rect expects numeric coordinates.");
    auto size = expectSize(args, 2, "graphics-rect expects numeric size.");
    auto filled = !isFalseValue(args[4]);
    {
        std::scoped_lock lock(gGraphics.mutex);
        HPEN pen = CreatePen(PS_SOLID, 1, gGraphics.buffer.drawColor);
        HBRUSH brush = filled ? CreateSolidBrush(gGraphics.buffer.drawColor)
                              : static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        auto oldPen = SelectObject(gGraphics.buffer.backDc, pen);
        auto oldBrush = SelectObject(gGraphics.buffer.backDc, brush);
        Rectangle(gGraphics.buffer.backDc, origin.x, origin.y,
                  origin.x + size.width, origin.y + size.height);
        SelectObject(gGraphics.buffer.backDc, oldBrush);
        SelectObject(gGraphics.buffer.backDc, oldPen);
        if (filled) {
            DeleteObject(brush);
        }
        DeleteObject(pen);
    }
    return makeNil();
}

ValuePtr graphicsCircle(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 4, "graphics-circle");
    requireWindowOpen();
    auto center =
        expectPoint(args, 0, "graphics-circle expects numeric coordinates.");
    auto radius = expectInt(args[2], "graphics-circle expects numeric radius.");
    auto filled = !isFalseValue(args[3]);
    {
        std::scoped_lock lock(gGraphics.mutex);
        HPEN pen = CreatePen(PS_SOLID, 1, gGraphics.buffer.drawColor);
        HBRUSH brush = filled ? CreateSolidBrush(gGraphics.buffer.drawColor)
                              : static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        auto oldPen = SelectObject(gGraphics.buffer.backDc, pen);
        auto oldBrush = SelectObject(gGraphics.buffer.backDc, brush);
        Ellipse(gGraphics.buffer.backDc, center.x - radius, center.y - radius,
                center.x + radius, center.y + radius);
        SelectObject(gGraphics.buffer.backDc, oldBrush);
        SelectObject(gGraphics.buffer.backDc, oldPen);
        if (filled) {
            DeleteObject(brush);
        }
        DeleteObject(pen);
    }
    return makeNil();
}

ValuePtr graphicsText(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 3, "graphics-text");
    requireWindowOpen();
    auto origin =
        expectPoint(args, 0, "graphics-text expects numeric coordinates.");
    auto text = expectString(args[2], "graphics-text expects a string.");
    {
        std::scoped_lock lock(gGraphics.mutex);
        SetTextColor(gGraphics.buffer.backDc, gGraphics.buffer.drawColor);
        SetBkMode(gGraphics.buffer.backDc, TRANSPARENT);
        TextOutA(gGraphics.buffer.backDc, origin.x, origin.y, text.c_str(),
                 static_cast<int>(text.size()));
    }
    return makeNil();
}

ValuePtr graphicsRefresh(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 0, "graphics-refresh");
    requireWindowOpen();
    invalidateWindow();
    return makeNil();
}

ValuePtr graphicsPollEvent(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 0, "graphics-poll-event");
    std::scoped_lock lock(gGraphics.mutex);
    auto event = popEventLocked();
    if (!event) {
        return makeNil();
    }
    return eventToValue(std::move(*event));
}

ValuePtr graphicsWaitEvent(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 0, "graphics-wait-event");
    std::unique_lock lock(gGraphics.mutex);
    gGraphics.eventChanged.wait(lock, [] {
        return !gGraphics.events.empty() || !gGraphics.window.open;
    });
    auto event = popEventLocked();
    if (!event) {
        return makeNil();
    }
    return eventToValue(std::move(*event));
}

ValuePtr graphicsSleep(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 1, "graphics-sleep");
    auto milliseconds =
        expectInt(args[0], "graphics-sleep expects milliseconds.");
    if (milliseconds < 0) {
        throw LispError("graphics-sleep expects non-negative milliseconds.");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    return makeNil();
}
