; Playable Othello/Reversi implemented in Mini-Lisp.
; Click a highlighted square to place a piece. Close the window to exit.

(define origin-x 40)
(define origin-y 40)
(define cell 60)
(define board-size 8)
(define black 1)
(define white 2)
(define empty 0)

(define (opponent player)
  (if (= player black) white black))

(define (player-name player)
  (if (= player black) "Black" "White"))

(define (piece-at-start index)
  (cond
    ((= index 27) white)
    ((= index 28) black)
    ((= index 35) black)
    ((= index 36) white)
    (else empty)))

(define (make-board index)
  (if (= index 64)
      '()
      (cons (piece-at-start index) (make-board (+ index 1)))))

(define (list-ref items index)
  (if (= index 0)
      (car items)
      (list-ref (cdr items) (- index 1))))

(define (list-set items index value)
  (if (= index 0)
      (cons value (cdr items))
      (cons (car items) (list-set (cdr items) (- index 1) value))))

(define (board-index row col)
  (+ (* row board-size) col))

(define (board-ref board row col)
  (list-ref board (board-index row col)))

(define (board-set board row col value)
  (list-set board (board-index row col) value))

(define (on-board? row col)
  (and (>= row 0) (< row board-size) (>= col 0) (< col board-size)))

(define directions
  '((-1 -1) (-1 0) (-1 1)
    (0 -1)          (0 1)
    (1 -1)  (1 0)  (1 1)))

(define (dir-row dir) (car dir))
(define (dir-col dir) (car (cdr dir)))

(define (captures-in-direction board player row col dr dc)
  (define (walk r c seen)
    (if (not (on-board? r c))
        '()
        (let ((value (board-ref board r c)))
          (cond
            ((= value (opponent player))
             (walk (+ r dr) (+ c dc) (cons (board-index r c) seen)))
            ((= value player)
             (if (null? seen) '() seen))
            (else '())))))
  (walk (+ row dr) (+ col dc) '()))

(define (captures-from-directions board player row col dirs)
  (if (null? dirs)
      '()
      (append
        (captures-in-direction board player row col
                               (dir-row (car dirs))
                               (dir-col (car dirs)))
        (captures-from-directions board player row col (cdr dirs)))))

(define (captures-for-move board player row col)
  (if (and (on-board? row col) (= (board-ref board row col) empty))
      (captures-from-directions board player row col directions)
      '()))

(define (valid-move? board player row col)
  (not (null? (captures-for-move board player row col))))

(define (valid-move-index? board player index)
  (valid-move? board player (quotient index board-size) (modulo index board-size)))

(define (valid-moves-from board player index)
  (if (= index 64)
      '()
      (if (valid-move-index? board player index)
          (cons index (valid-moves-from board player (+ index 1)))
          (valid-moves-from board player (+ index 1)))))

(define (valid-moves board player)
  (valid-moves-from board player 0))

(define (has-valid-move? board player)
  (not (null? (valid-moves board player))))

(define (game-over? board)
  (and (not (has-valid-move? board black))
       (not (has-valid-move? board white))))

(define (flip-indices board indices player)
  (if (null? indices)
      board
      (flip-indices (list-set board (car indices) player) (cdr indices) player)))

(define (apply-move board player row col)
  (let ((captured (captures-for-move board player row col)))
    (if (null? captured)
        board
        (flip-indices (board-set board row col player) captured player))))

(define (count-pieces board player)
  (if (null? board)
      0
      (+ (if (= (car board) player) 1 0)
         (count-pieces (cdr board) player))))

(define (score-text board)
  (string-append "Black " (number->string (count-pieces board black))
                 "  White " (number->string (count-pieces board white))))

(define (winner-text board)
  (let ((black-score (count-pieces board black)))
    (let ((white-score (count-pieces board white)))
      (cond
        ((> black-score white-score) "Game over: Black wins")
        ((< black-score white-score) "Game over: White wins")
        (else "Game over: Draw")))))

(define (cell-x col) (+ origin-x (* col cell)))
(define (cell-y row) (+ origin-y (* row cell)))
(define (piece-x col) (+ (cell-x col) 30))
(define (piece-y row) (+ (cell-y row) 30))

(define (draw-grid-line n)
  (if (> n board-size)
      '()
      (begin
        (graphics-line origin-x (+ origin-y (* n cell))
                       (+ origin-x (* board-size cell)) (+ origin-y (* n cell)))
        (graphics-line (+ origin-x (* n cell)) origin-y
                       (+ origin-x (* n cell)) (+ origin-y (* board-size cell)))
        (draw-grid-line (+ n 1)))))

(define (draw-piece row col player)
  (if (= player empty)
      '()
      (begin
        (if (= player black)
            (graphics-color 20 20 20)
            (graphics-color 246 246 246))
        (graphics-circle (piece-x col) (piece-y row) 24 #t)
        (graphics-color 20 20 20)
        (graphics-circle (piece-x col) (piece-y row) 24 #f))))

(define (draw-pieces board index)
  (if (= index 64)
      '()
      (begin
        (draw-piece (quotient index board-size)
                    (modulo index board-size)
                    (list-ref board index))
        (draw-pieces board (+ index 1)))))

(define (draw-move-hint index)
  (let ((row (quotient index board-size)))
    (let ((col (modulo index board-size)))
      (graphics-circle (piece-x col) (piece-y row) 7 #t))))

(define (draw-move-hints moves)
  (if (null? moves)
      '()
      (begin
        (draw-move-hint (car moves))
        (draw-move-hints (cdr moves)))))

(define (draw-board-background)
  (begin
    (graphics-clear 235 239 233)
    (graphics-color 26 132 77)
    (graphics-rect origin-x origin-y (* board-size cell) (* board-size cell) #t)
    (graphics-color 15 70 45)
    (draw-grid-line 0)))

(define (draw-status board player message)
  (begin
    (graphics-color 30 30 30)
    (graphics-text 40 535 (score-text board))
    (graphics-text 40 560 (if (game-over? board)
                              (winner-text board)
                              (string-append (player-name player) " to move")))
    (graphics-text 250 560 message)))

(define (draw-game board player message)
  (begin
    (draw-board-background)
    (graphics-color 245 216 95)
    (draw-move-hints (valid-moves board player))
    (draw-pieces board 0)
    (draw-status board player message)
    (graphics-refresh)))

(define (inside-board-pixel? x y)
  (and (>= x origin-x)
       (< x (+ origin-x (* board-size cell)))
       (>= y origin-y)
       (< y (+ origin-y (* board-size cell)))))

(define (pixel-row y)
  (quotient (- y origin-y) cell))

(define (pixel-col x)
  (quotient (- x origin-x) cell))

(define (next-state board player)
  (let ((other (opponent player)))
    (cond
      ((has-valid-move? board other)
       (list board other ""))
      ((has-valid-move? board player)
       (list board player (string-append (player-name other) " has no legal move")))
      (else
       (list board player "")))))

(define (state-board state) (car state))
(define (state-player state) (car (cdr state)))
(define (state-message state) (car (cdr (cdr state))))

(define (handle-click board player row col)
  (if (valid-move? board player row col)
      (next-state (apply-move board player row col) player)
      (list board player "Illegal move")))

(define (handle-event board player event)
  (if (and (pair? event) (eq? (car event) 'mouse-down))
      (let ((x (car (cdr event))))
        (let ((y (car (cdr (cdr event)))))
          (if (inside-board-pixel? x y)
              (handle-click board player (pixel-row y) (pixel-col x))
              (list board player ""))))
      (list board player "")))

(define (event-changes-board? event)
  (and (pair? event) (eq? (car event) 'mouse-down)))

(define (read-game-event)
  (let ((event (graphics-wait-event)))
    (if (and (pair? event) (eq? (car event) 'mouse-move))
        (read-game-event)
        event)))

(define (wait-close)
  (let ((event (read-game-event)))
    (if (and (pair? event) (eq? (car event) 'close))
        (graphics-close)
        (wait-close))))

(define (event-loop board player message)
  (if (game-over? board)
      (wait-close)
      (let ((event (read-game-event)))
        (cond
          ((null? event)
           (event-loop board player message))
          ((eq? (car event) 'close)
           (graphics-close))
          ((event-changes-board? event)
           (let ((state (handle-event board player event)))
             (begin
               (draw-game (state-board state)
                          (state-player state)
                          (state-message state))
               (event-loop (state-board state)
                           (state-player state)
                           (state-message state)))))
          (else
           (event-loop board player message))))))

(define (game-loop board player message)
  (begin
    (draw-game board player message)
    (event-loop board player message)))

(graphics-open 560 590 "Mini-Lisp Othello")
(game-loop (make-board 0) black "")
