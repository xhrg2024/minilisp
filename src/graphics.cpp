#include "./graphics.h"

#include <Windows.h>
#include <windowsx.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "./error.h"
#include "./value.h"

namespace {

struct GraphicsEvent {
    std::string name;
    std::vector<ValuePtr> payload;
};

struct GraphicsState {
    std::mutex mutex;
    std::condition_variable readyChanged;
    std::condition_variable eventChanged;
    std::queue<GraphicsEvent> events;
    std::thread uiThread;
    HWND hwnd = nullptr;
    HDC backDc = nullptr;
    HBITMAP backBitmap = nullptr;
    HBITMAP oldBitmap = nullptr;
    COLORREF drawColor = RGB(0, 0, 0);
    int width = 0;
    int height = 0;
    bool ready = false;
    bool open = false;
};

GraphicsState gGraphics;

ValuePtr makeNil() {
    return std::make_shared<NilValue>();
}

ValuePtr makeBool(bool value) {
    return std::make_shared<BooleanValue>(value);
}

ValuePtr makeSymbol(const std::string& name) {
    return std::make_shared<SymbolValue>(name);
}

ValuePtr makeNumber(double value) {
    return std::make_shared<NumericValue>(value);
}

ValuePtr makeList(const std::vector<ValuePtr>& values) {
    ValuePtr result = makeNil();
    for (auto iter = values.rbegin(); iter != values.rend(); ++iter) {
        result = std::make_shared<PairValue>(*iter, result);
    }
    return result;
}

double expectNumber(const ValuePtr& value, const char* message) {
    auto number = value->asNumber();
    if (!number) {
        throw LispError(message);
    }
    return *number;
}

int expectInt(const ValuePtr& value, const char* message) {
    return static_cast<int>(std::lround(expectNumber(value, message)));
}

std::string expectString(const ValuePtr& value, const char* message) {
    auto string = std::dynamic_pointer_cast<StringValue>(value);
    if (string == nullptr) {
        throw LispError(message);
    }
    return string->getValue();
}

bool isFalseValue(const ValuePtr& value) {
    auto boolean = std::dynamic_pointer_cast<BooleanValue>(value);
    return boolean != nullptr && !boolean->getValue();
}

void requireArgsSize(const std::vector<ValuePtr>& args, std::size_t expected, const char* name) {
    if (args.size() != expected) {
        throw LispError(std::string(name) + " expects " + std::to_string(expected) + " argument(s).");
    }
}

void requireWindowOpen() {
    std::scoped_lock lock(gGraphics.mutex);
    if (!gGraphics.open || gGraphics.hwnd == nullptr || gGraphics.backDc == nullptr) {
        throw LispError("graphics window is not open.");
    }
}

COLORREF expectColor(const std::vector<ValuePtr>& args, std::size_t start, const char* name) {
    auto r = std::clamp(expectInt(args[start], name), 0, 255);
    auto g = std::clamp(expectInt(args[start + 1], name), 0, 255);
    auto b = std::clamp(expectInt(args[start + 2], name), 0, 255);
    return RGB(r, g, b);
}

void pushEvent(std::string name, std::vector<ValuePtr> payload = {}) {
    {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.events.push(GraphicsEvent{std::move(name), std::move(payload)});
    }
    gGraphics.eventChanged.notify_all();
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

LRESULT CALLBACK graphicsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(hwnd, &paint);
            {
                std::scoped_lock lock(gGraphics.mutex);
                if (gGraphics.backDc != nullptr) {
                    BitBlt(dc, 0, 0, gGraphics.width, gGraphics.height,
                           gGraphics.backDc, 0, 0, SRCCOPY);
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
            gGraphics.open = false;
            gGraphics.hwnd = nullptr;
            gGraphics.eventChanged.notify_all();
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

void cleanupBackBuffer() {
    if (gGraphics.backDc != nullptr) {
        SelectObject(gGraphics.backDc, gGraphics.oldBitmap);
        DeleteObject(gGraphics.backBitmap);
        DeleteDC(gGraphics.backDc);
        gGraphics.backDc = nullptr;
        gGraphics.backBitmap = nullptr;
        gGraphics.oldBitmap = nullptr;
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

    HWND hwnd = CreateWindowExA(0, className, title.c_str(), WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                rect.right - rect.left, rect.bottom - rect.top,
                                nullptr, nullptr, instance, nullptr);
    if (hwnd == nullptr) {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.ready = true;
        gGraphics.open = false;
        gGraphics.readyChanged.notify_all();
        return;
    }

    HDC windowDc = GetDC(hwnd);
    HDC backDc = CreateCompatibleDC(windowDc);
    HBITMAP bitmap = CreateCompatibleBitmap(windowDc, width, height);
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(backDc, bitmap));
    ReleaseDC(hwnd, windowDc);

    RECT clearRect{0, 0, width, height};
    FillRect(backDc, &clearRect, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    SetBkMode(backDc, TRANSPARENT);

    {
        std::scoped_lock lock(gGraphics.mutex);
        gGraphics.hwnd = hwnd;
        gGraphics.backDc = backDc;
        gGraphics.backBitmap = bitmap;
        gGraphics.oldBitmap = oldBitmap;
        gGraphics.width = width;
        gGraphics.height = height;
        gGraphics.ready = true;
        gGraphics.open = true;
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
    gGraphics.open = false;
    gGraphics.hwnd = nullptr;
}

void invalidateWindow() {
    HWND hwnd = nullptr;
    {
        std::scoped_lock lock(gGraphics.mutex);
        hwnd = gGraphics.hwnd;
    }
    if (hwnd != nullptr) {
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

void closeWindow() {
    HWND hwnd = nullptr;
    {
        std::scoped_lock lock(gGraphics.mutex);
        hwnd = gGraphics.hwnd;
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
        gGraphics.ready = false;
        while (!gGraphics.events.empty()) {
            gGraphics.events.pop();
        }
    }

    gGraphics.uiThread = std::thread(uiThreadMain, width, height, std::move(title));
    {
        std::unique_lock lock(gGraphics.mutex);
        gGraphics.readyChanged.wait(lock, [] { return gGraphics.ready; });
        if (!gGraphics.open) {
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
    auto color = expectColor(args, 0, "graphics-clear expects numeric RGB values.");
    {
        std::scoped_lock lock(gGraphics.mutex);
        HBRUSH brush = CreateSolidBrush(color);
        RECT rect{0, 0, gGraphics.width, gGraphics.height};
        FillRect(gGraphics.backDc, &rect, brush);
        DeleteObject(brush);
    }
    return makeNil();
}

ValuePtr graphicsColor(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 3, "graphics-color");
    auto color = expectColor(args, 0, "graphics-color expects numeric RGB values.");
    std::scoped_lock lock(gGraphics.mutex);
    gGraphics.drawColor = color;
    return makeNil();
}

ValuePtr graphicsLine(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 4, "graphics-line");
    requireWindowOpen();
    auto x1 = expectInt(args[0], "graphics-line expects numeric coordinates.");
    auto y1 = expectInt(args[1], "graphics-line expects numeric coordinates.");
    auto x2 = expectInt(args[2], "graphics-line expects numeric coordinates.");
    auto y2 = expectInt(args[3], "graphics-line expects numeric coordinates.");
    {
        std::scoped_lock lock(gGraphics.mutex);
        HPEN pen = CreatePen(PS_SOLID, 1, gGraphics.drawColor);
        auto oldPen = SelectObject(gGraphics.backDc, pen);
        MoveToEx(gGraphics.backDc, x1, y1, nullptr);
        LineTo(gGraphics.backDc, x2, y2);
        SelectObject(gGraphics.backDc, oldPen);
        DeleteObject(pen);
    }
    return makeNil();
}

ValuePtr graphicsRect(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 5, "graphics-rect");
    requireWindowOpen();
    auto x = expectInt(args[0], "graphics-rect expects numeric coordinates.");
    auto y = expectInt(args[1], "graphics-rect expects numeric coordinates.");
    auto width = expectInt(args[2], "graphics-rect expects numeric size.");
    auto height = expectInt(args[3], "graphics-rect expects numeric size.");
    auto filled = !isFalseValue(args[4]);
    {
        std::scoped_lock lock(gGraphics.mutex);
        HPEN pen = CreatePen(PS_SOLID, 1, gGraphics.drawColor);
        HBRUSH brush = filled ? CreateSolidBrush(gGraphics.drawColor)
                              : static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        auto oldPen = SelectObject(gGraphics.backDc, pen);
        auto oldBrush = SelectObject(gGraphics.backDc, brush);
        Rectangle(gGraphics.backDc, x, y, x + width, y + height);
        SelectObject(gGraphics.backDc, oldBrush);
        SelectObject(gGraphics.backDc, oldPen);
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
    auto x = expectInt(args[0], "graphics-circle expects numeric coordinates.");
    auto y = expectInt(args[1], "graphics-circle expects numeric coordinates.");
    auto radius = expectInt(args[2], "graphics-circle expects numeric radius.");
    auto filled = !isFalseValue(args[3]);
    {
        std::scoped_lock lock(gGraphics.mutex);
        HPEN pen = CreatePen(PS_SOLID, 1, gGraphics.drawColor);
        HBRUSH brush = filled ? CreateSolidBrush(gGraphics.drawColor)
                              : static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        auto oldPen = SelectObject(gGraphics.backDc, pen);
        auto oldBrush = SelectObject(gGraphics.backDc, brush);
        Ellipse(gGraphics.backDc, x - radius, y - radius, x + radius, y + radius);
        SelectObject(gGraphics.backDc, oldBrush);
        SelectObject(gGraphics.backDc, oldPen);
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
    auto x = expectInt(args[0], "graphics-text expects numeric coordinates.");
    auto y = expectInt(args[1], "graphics-text expects numeric coordinates.");
    auto text = expectString(args[2], "graphics-text expects a string.");
    {
        std::scoped_lock lock(gGraphics.mutex);
        SetTextColor(gGraphics.backDc, gGraphics.drawColor);
        SetBkMode(gGraphics.backDc, TRANSPARENT);
        TextOutA(gGraphics.backDc, x, y, text.c_str(), static_cast<int>(text.size()));
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
    if (gGraphics.events.empty()) {
        return makeNil();
    }
    auto event = std::move(gGraphics.events.front());
    gGraphics.events.pop();
    std::vector<ValuePtr> values{makeSymbol(event.name)};
    values.insert(values.end(), event.payload.begin(), event.payload.end());
    return makeList(values);
}

ValuePtr graphicsWaitEvent(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 0, "graphics-wait-event");
    std::unique_lock lock(gGraphics.mutex);
    gGraphics.eventChanged.wait(lock, [] {
        return !gGraphics.events.empty() || !gGraphics.open;
    });
    if (gGraphics.events.empty()) {
        return makeNil();
    }
    auto event = std::move(gGraphics.events.front());
    gGraphics.events.pop();
    std::vector<ValuePtr> values{makeSymbol(event.name)};
    values.insert(values.end(), event.payload.begin(), event.payload.end());
    return makeList(values);
}

ValuePtr graphicsSleep(const std::vector<ValuePtr>& args, EvalEnv&) {
    requireArgsSize(args, 1, "graphics-sleep");
    auto milliseconds = expectInt(args[0], "graphics-sleep expects milliseconds.");
    if (milliseconds < 0) {
        throw LispError("graphics-sleep expects non-negative milliseconds.");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    return makeNil();
}
