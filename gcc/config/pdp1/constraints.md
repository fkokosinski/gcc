;; -------------------------------------------------------------------------
;; Constraints
;; -------------------------------------------------------------------------

(define_register_constraint "a" "ACC_REG" "Accumulator register ($acc)")

(define_constraint "U"
  "An immediate value of 1"
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 1, 1)")))
