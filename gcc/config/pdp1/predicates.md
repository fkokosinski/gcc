;; -------------------------------------------------------------------------
;; Predicates
;; -------------------------------------------------------------------------

(define_predicate "pdp1_comparison_operator"
  (match_code "ne,eq,lt,gt"))

(define_predicate "pdp1_addsrc_operand"
  (ior (match_operand 0 "nonimmediate_operand")
       (and (match_code "const_int")
	    (match_test "INTVAL (op) == 1"))))
