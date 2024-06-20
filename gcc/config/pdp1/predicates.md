;; -------------------------------------------------------------------------
;; Predicates
;; -------------------------------------------------------------------------

(define_predicate "pdp1_comparison_operator"
  (match_code "ne,eq,lt,gt"))

(define_predicate "pdp1_addsrc_operand"
  (ior (match_operand 0 "nonimmediate_operand")
       (and (match_code "const_int")
	    (match_test "INTVAL (op) == 1"))))

;; TODO: how to handle immediate operands outside this range?
(define_predicate "pdp1_movsrc_operand"
  (ior (match_operand 0 "nonimmediate_operand")
       (and (match_code "const_int")
	    (match_test "IN_RANGE (INTVAL (op), -4095, 4095)"))
       (ior (and (match_code "const_int")
		 (match_test "IN_RANGE (INTVAL (op), -4095, 4095)"))
	    (match_code "symbol_ref,const"))))

;; TODO: how to handle larger shifts?
(define_predicate "pdp1_shift_rotate_amount"
  (and (match_code "const_int")
       (match_test "IN_RANGE (INTVAL (op), 0, 7)")))
