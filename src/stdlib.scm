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

(define (apply f a) (__apply f a))
(define (eval ex en) (__eval ex en))
(define current-environment cur-env)
(define interaction-environment base-env)
(define (defined? a e) (not (eq? #f (memq a (env-names e)))))

(define integer? exact?)
(define real? inexact?)
(define (equal? x y)
  (if (pair? x)
    (and (pair? y)
	 (equal? (car x) (car y))
	 (equal? (cdr x) (cdr y)))
    (eqv? x y)))
(define (null? x) (eq? x ()))

(define (zero? x) (= x 0))

(define (identity x) x)

(define (unzip1-with-cdr . lists)
  (unzip1-with-cdr-iterative lists '() '()))

(define (unzip1-with-cdr-iterative lists cars cdrs)
  (if (null? lists)
      (cons (reverse-in-place! cars) (reverse-in-place! cdrs))
      (let ((car1 (caar lists))
            (cdr1 (cdar lists)))
        (unzip1-with-cdr-iterative
          (cdr lists)
          (cons car1 cars)
          (cons cdr1 cdrs)))))

(define (map proc . lists)
  (if (null? lists)
      (apply proc)
      (if (null? (car lists))
        '()
        (let ((unz (apply unzip1-with-cdr lists)))
          (cons (apply proc (car unz)) (apply map (cons proc (cdr unz))))))))

(define (for-each f l)
  (cond
    ((null? l) ())
    (else (f (car l))
	  (for-each f (cdr l)))))

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

(define (reverse-in-place! l)
  (__ripi! '() l))

(define (__ripi! new cur)
  (if (null? cur)
    new
    (begin
      (define rest (cdr cur))
      (set-cdr! cur new)
      (__ripi! cur rest))))

(define (append xs ys)
  (foldl (lambda (x y) (cons y x)) ys (reverse xs)))

(define (length l)
  (foldl (lambda (x y) (+ x 1)) 0 l))

(define (list-tail l n)
  (if (zero? n)
    l
    (list-tail (cdr l) (- n 1))))

(define (list-ref l n)
  (car (list-tail l n)))

(define (str->list-helper s i acc)
  (if (= i 0)
    (cons (string-ref s 0) acc)
    (str->list-helper s (- i 1) (cons (string-ref s i) acc))))

(define (string->list s)
  (define len (string-length s))
  (if (= len 0)
    ()
    (str->list-helper s (- len 1) ())))

(define (list->str-helper l s i)
  (cond
    ((null? l)
     s)
    (else
      (string-set! s i (car l))
      (list->str-helper (cdr l) s (+ i 1)))))

(define (list->string l)
  (list->str-helper l (make-string (length l)) 0))

(define (pretty-env-helper names vals acc)
  (if (null? names)
    acc
    (pretty-env-helper (cdr names) (cdr vals) (cons (cons (car names) (car vals)) acc))))

(define (pretty-env e)
  (pretty-env-helper (env-names e) (env-values e) ()))

(define (parent-env e)
  (if (pair? e)
    (car e)
    #f))

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
  (g-member equal? e l))

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
 
