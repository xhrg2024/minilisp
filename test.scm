(define (f x) (* x x))
(f 4)
(define (adder n) (lambda (m) (+ m n)))
((adder 5) 3)
