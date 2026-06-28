(define (sort lst)
  (if (null? lst)
      '()
      (if (null? (cdr lst))
          lst
          (append
            (sort (filter (lambda (x) (< x (car lst))) (cdr lst)))
            (cons (car lst) (sort (filter (lambda (x) (>= x (car lst))) (cdr lst))))))))

(displayln (sort '(12 71 2 15 29 82 87 8 18 66 81 25 63 97 40 3 93 58 53 31 47)))
