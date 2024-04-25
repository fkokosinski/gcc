;; -------------------------------------------------------------------------
;; PDP1 specific constraints, predicates and attributes
;; -------------------------------------------------------------------------

(include "constraints.md")
(include "predicates.md")

(define_mode_iterator PDP1_MODE [QI HI SI])
(define_constants [
  (PDP1_SP  9)
  (PDP1_ACC 10)
  (PDP1_CC  15)])

;; -------------------------------------------------------------------------
;; nop instruction
;; -------------------------------------------------------------------------

(define_insn "nop"
  [(const_int 0)]
  ""
  "nop")

;; -------------------------------------------------------------------------
;; mov instruction
;; -------------------------------------------------------------------------

(define_insn "mov<mode>"
  [(set (match_operand:PDP1_MODE 0 "nonimmediate_operand" "=a,a,rm")
	(match_operand:PDP1_MODE 1 "general_operand" "rm,i,a"))]
  ""
  "@
  lac\\t%1
  law\\t%1
  dac\\t%0")

(define_expand "mov<mode>_push"
  [(set (mem:PDP1_MODE (pre_dec:PDP1_MODE (reg:PDP1_MODE PDP1_SP)))
  	(match_operand:PDP1_MODE 0 "general_operand" "a"))]
  ""
  "
{
  rtx sp = gen_rtx_REG (<MODE>mode, PDP1_SP);
  rtx mem = gen_rtx_MEM (<MODE>mode, sp);
  rtx incr = gen_rtx_CONST_INT (<MODE>mode, -1);
  rtx acc = operands[0];

  /* push the actual value */
  emit_move_insn (mem, acc);

  /* decrement the sp */
  emit_move_insn (acc, incr);
  emit_insn (gen_add<mode>3 (acc, acc, sp));
  emit_move_insn (sp, acc);

  DONE;
}")

(define_expand "mov<mode>_pop"
  [(set (match_operand:PDP1_MODE 0 "nonimmediate_operand" "r")
  	(mem:PDP1_MODE (post_inc:PDP1_MODE (reg:PDP1_MODE PDP1_SP))))]
  ""
  "
{
  rtx sp = gen_rtx_REG (<MODE>mode, PDP1_SP);
  rtx acc = gen_rtx_REG (<MODE>mode, PDP1_ACC);
  rtx mem = gen_rtx_MEM (<MODE>mode, sp);
  rtx incr = gen_rtx_CONST_INT (<MODE>mode, 1);

  /* increment the sp */
  emit_move_insn (acc, incr);
  emit_insn (gen_add<mode>3 (acc, acc, sp));
  emit_move_insn (sp, acc);

  /* pop the actual value */
  emit_move_insn (acc, mem);
  emit_move_insn (operands[0], acc);

  DONE;
}")

;; -------------------------------------------------------------------------
;; arith instruction
;; -------------------------------------------------------------------------

(define_insn "add<mode>3"
  [(set (match_operand:PDP1_MODE 0 "register_operand" "=a")
	(plus:PDP1_MODE
	  (match_operand:PDP1_MODE 1 "register_operand" "0")
	  (match_operand:PDP1_MODE 2 "nonimmediate_operand" "g")))]
  ""
  "add\\t%2")

(define_insn "sub<mode>3"
  [(set (match_operand:PDP1_MODE 0 "register_operand" "=a")
	(minus:PDP1_MODE
	  (match_operand:PDP1_MODE 1 "register_operand" "0")
	  (match_operand:PDP1_MODE 2 "nonimmediate_operand" "g")))]
  ""
  "sub\\t%2")

;; -------------------------------------------------------------------------
;; logic instruction
;; -------------------------------------------------------------------------

(define_insn "one_cmpl<mode>2"
  [(set (match_operand:PDP1_MODE 0 "register_operand" "=a")
	(neg:PDP1_MODE
	  (match_operand:PDP1_MODE 1 "register_operand" "0")))]
  ""
  "cma")

(define_insn "and<mode>3"
  [(set (match_operand:PDP1_MODE 0 "register_operand" "=a")
	(and:PDP1_MODE
	  (match_operand:PDP1_MODE 1 "register_operand" "0")
	  (match_operand:PDP1_MODE 2 "nonimmediate_operand" "g")))]
  ""
  "and\\t%2")

(define_insn "xor<mode>3"
  [(set (match_operand:PDP1_MODE 0 "register_operand" "=a")
	(xor:PDP1_MODE
	  (match_operand:PDP1_MODE 1 "register_operand" "0")
	  (match_operand:PDP1_MODE 2 "nonimmediate_operand" "g")))]
  ""
  "xor\\t%2")

(define_insn "ior<mode>3"
  [(set (match_operand:PDP1_MODE 0 "register_operand" "=a")
	(ior:PDP1_MODE
	  (match_operand:PDP1_MODE 1 "register_operand" "0")
	  (match_operand:PDP1_MODE 2 "nonimmediate_operand" "g")))]
  ""
  "ior\\t%2")

;; -------------------------------------------------------------------------
;; jmp instruction
;; -------------------------------------------------------------------------

(define_insn "indirect_jump"
  [(set (pc) (match_operand:QI 0 "nonimmediate_operand" "r"))]
  ""
  "jmp.i\\t%0")

(define_insn "jump"
  [(set (pc)
        (label_ref (match_operand 0 "" "")))]
  ""
  "jmp\\t%l0")

(define_insn "call"
  [(call (match_operand:QI 0 "memory_operand" "")
	 (match_operand:QI 1 "general_operand" ""))]
  ""
  "jsp\\t%0")

(define_insn "call_value"
  [(set (match_operand 0 "register_operand" "")
	(call (match_operand 1 "memory_operand" "")
	      (match_operand 2 "general_operand" "")))]
  ""
  "jsp\\t%1")

;; -------------------------------------------------------------------------
;; compare and conditionals
;; -------------------------------------------------------------------------

(define_insn "cbranch<mode>4"
  [(set (pc)
        (if_then_else
	  (match_operator 0 "pdp1_comparison_operator"
	    [(match_operand:PDP1_MODE 1 "register_operand" "a")
	     (match_operand:PDP1_MODE 2 "register_operand" "r")])
	  (label_ref (match_operand 3 "" ""))
	  (pc)))]
  ""
{
  switch (GET_CODE (operands[0])) {
  case EQ:
    return "sad\\t%2\\n\\tjmp\\t%l3";
  case NE:
    return "sas\\t%2\\n\\tjmp\\t%l3";
  }
})

;; -------------------------------------------------------------------------
;; prologue and epilogue
;; -------------------------------------------------------------------------

(define_expand "prologue"
  [(clobber (const_int 1))]
  ""
  "
{
  pdp1_expand_prologue();
  DONE;
}")

(define_expand "epilogue"
  [(return)]
  ""
  "
{
  pdp1_expand_epilogue();
  DONE;
}")
