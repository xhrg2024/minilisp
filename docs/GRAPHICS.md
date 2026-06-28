# Mini-Lisp Graphics

This project includes a small Windows 2D graphics layer exposed as Lisp builtins.

## Procedures

- `(graphics-open width height title)` opens a window.
- `(graphics-close)` closes the current window.
- `(graphics-clear r g b)` clears the window with an RGB color.
- `(graphics-color r g b)` sets the current drawing color.
- `(graphics-line x1 y1 x2 y2)` draws a line.
- `(graphics-rect x y width height filled?)` draws a rectangle.
- `(graphics-circle x y radius filled?)` draws a circle.
- `(graphics-text x y text)` draws text.
- `(graphics-refresh)` requests a repaint.
- `(graphics-poll-event)` returns `()` or an event list.
- `(graphics-wait-event)` waits until an event is available, then returns it.
- `(graphics-sleep milliseconds)` pauses the current Lisp script.

Events are lists such as `(close)`, `(mouse-down x y left)`, `(mouse-up x y left)`,
`(mouse-move x y)`, and `(key-down code)`.

Run `scripts/graphics_demo.scm` to play Othello. Click a highlighted square to move;
the script flips pieces, skips players with no legal moves, and shows the winner
when both players are out of moves.
