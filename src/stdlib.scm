(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))
(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdaar x) (cdr (car (car x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))
(define (caaaar x) (car (car (car (car x)))))
(define (caaadr x) (car (car (car (cdr x)))))
(define (caadar x) (car (car (cdr (car x)))))
(define (caaddr x) (car (car (cdr (cdr x)))))
(define (cadaar x) (car (cdr (car (car x)))))
(define (cadadr x) (car (cdr (car (cdr x)))))
(define (caddar x) (car (cdr (cdr (car x)))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))
(define (cdaaar x) (cdr (car (car (car x)))))
(define (cdaadr x) (cdr (car (car (cdr x)))))
(define (cdadar x) (cdr (car (cdr (car x)))))
(define (cdaddr x) (cdr (car (cdr (cdr x)))))
(define (cddaar x) (cdr (cdr (car (car x)))))
(define (cddadr x) (cdr (cdr (car (cdr x)))))
(define (cdddar x) (cdr (cdr (cdr (car x)))))
(define (cddddr x) (cdr (cdr (cdr (cdr x)))))

(define integer? exact?)
(define real? inexact?)
(define equal? eqv?) ;needs real implementation
(define (null? x) (eq? x ()))

(define (zero? x) (= x 0))

(define (identity x) x)

(define (foldl f x l)
  (if (null? l)
    x
    (foldl f (f x (car l)) (cdr l))))

(define (foldr f x l)
  (if (null? l)
    x
    (f (car l) (foldr f x (cdr l)))))

(define (reverse l)
  (foldl (lambda (x y) (cons y x)) () l))

(define (length l)
  (foldl (lambda (x y) (+ x 1)) 0 l))

(define (list-tail l n)
  (if (zero? n)
    l
    (list-tail (cdr x) (- k 1))))

(define (list-ref l n)
  (car (list-tail l n)))

(define (g-member cmp e l)
  (cond
    ((null? l) #f)
    ((cmp e (car l)) l)
    (else (g-member cmp e (cdr l)))))

(define (memq e l)
  (g-member eq? e l))
(define (memv e l)
  (g-member eqv? e l))
(define (member e l)
  (g-member e l))

(define (g-assoc cmp e l)
  (cond
    ((null? l) #f)
    ((cmp e (caar l)) (car l))
    (else (g-assoc cmp e (cdr l)))))

(define (assq obj alst)
  (g-assoc eq? obj alst))
(define (assv obj alst)
  (g-assoc eqv? obj alst))
(define (assoc obj alst)
  (g-assoc equal? obj alst))

