;; -------------------------------------------------------------------------
;; PDP1 specific constraints, predicates and attributes
;; -------------------------------------------------------------------------

(include "constraints.md")
(include "predicates.md")

(define_attr "length" "" (const_int 1))

;; This file implements instruction patterns so that the accumulator register
;; is hidden from the compiler (almost) altogether.
;;
;; Rationale: GCC doesn't handle singleton register classes well, and fails at
;; register spilling in higher optimization levels if we define them.
;;
;; TODO: Some of the patterns defined should eventually be moved to libgcc.


;; TODO: register declaration from `pdp1.h` should be moved here
(define_constants [
  (PDP1_SP  25)
  (PDP1_ACC 26)
  (PDP1_IO  27)
  (PDP1_CC  31)])

;; -------------------------------------------------------------------------
;; nop instruction
;; -------------------------------------------------------------------------

(define_insn "nop"
  [(const_int 0)]
  ""
  "nop"
  [(set_attr "length" "1")])

;; -------------------------------------------------------------------------
;; mov instruction
;; -------------------------------------------------------------------------

;; We need to define this pattern because it is needed for the mulqihi3 pattern
(define_insn "movhi"
  [(set (match_operand:HI 0 "nonimmediate_operand" "=g,g")
	(match_operand:HI 1 "general_operand" "rm,Z"))]
  ""
  "@
  lac\\t%1\\n\\tdac\\t%0\\n\\tlac\\t%1+1\\n\\tdac\\t%0+1
  dzm\\t%0\\n\\tdzm\\t%0+1"
  [(set_attr "length" "4,2")])

;; TODO: we should prepare negative values for the 1's complement arithmetic
;; that PDP-1 uses
(define_insn "movqi"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g,g,a,a,g")
	(match_operand:QI 1 "pdp1_movsrc_operand" "rm,i,Z,rm,i,a"))]
  ""
  "@
  lac\\t%1\\n\\tdac\\t%0
  law\\t%1\\n\\tdac\\t%0
  dzm\\t%0
  lac\\t%1
  law\\t%1
  dac\\t%0"
  [(set_attr "length" "2,2,1,1,1,1")])

(define_expand "movqi_push"
  [(set (mem:QI (pre_dec:QI (reg:QI PDP1_SP)))
  	(match_operand:QI 0 "general_operand" ""))]
  ""
  "
{
  rtx sp = gen_rtx_REG (QImode, PDP1_SP);
  rtx mem = gen_rtx_MEM (QImode, sp);
  rtx incr = gen_rtx_CONST_INT (QImode, -1);

  /* push the actual value */
  emit_move_insn (mem, operands[0]);

  /* decrement the sp */
  emit_insn (gen_addqi3 (sp, sp, incr));

  DONE;
}")

(define_expand "movqi_pop"
  [(set (match_operand:QI 0 "nonimmediate_operand" "")
  	(mem:QI (post_inc:QI (reg:QI PDP1_SP))))]
  ""
  "
{
  rtx sp = gen_rtx_REG (QImode, PDP1_SP);
  rtx mem = gen_rtx_MEM (QImode, sp);
  rtx incr = gen_rtx_CONST_INT (QImode, 1);

  /* increment the sp */
  emit_insn (gen_addqi3 (sp, sp, incr));

  /* pop the actual value */
  emit_move_insn (operands[0], mem);

  DONE;
}")

;; -------------------------------------------------------------------------
;; arith instruction
;; -------------------------------------------------------------------------

(define_insn "addqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g,g")
	(plus:QI
	  (match_operand:QI 1 "nonimmediate_operand" "0,g,g")
	  (match_operand:QI 2 "general_operand" "U,i,rm")))]
  ""
  "@
  idx\\t%0
  law\\t%2\\n\\tadd\\t%1\\n\\tdac\\t%0
  lac\\t%2\\n\\tadd\\t%1\\n\\tdac\\t%0"
  [(set_attr "length" "3,3,1")])

(define_insn "subqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g")
	(minus:QI
	  (match_operand:QI 1 "general_operand" "rm,i")
	  (match_operand:QI 2 "nonimmediate_operand" "g,g")))]
  ""
  "@
  lac\\t%1\\n\\tsub\\t%2\\n\\tdac\\t%0
  law\\t%1\\n\\tsub\\t%2\\n\\tdac\\t%0"
  [(set_attr "length" "3,3")])

(define_insn "mulqihi3"
  [(set (match_operand:HI 0 "nonimmediate_operand" "=g,g")
	(mult:HI
	  (match_operand:QI 1 "nonimmediate_operand" "0,0")
	  (match_operand:QI 2 "general_operand" "rm,i")))]
  ""
  "@
  lac\\t%2\\n\\tmul\\t%1\\n\\tscr\\t1\\n\\tdac\\t%0\\n\\tdio\\t%0+1
  law\\t%2\\n\\tmul\\t%1\\n\\tscr\\t1\\n\\tdac\\t%0\\n\\tdio\\t%0+1"
  [(set_attr "length" "5,5")])

(define_insn "divmodqi4"
  [(set (match_operand:QI 0 "register_operand" "=g")
	(div:QI
	  (match_operand:QI 1 "register_operand" "g")
	  (match_operand:QI 2 "nonimmediate_operand" "g")))
   (set (match_operand:QI 3 "register_operand" "=g")
	(mod:QI
	  (match_dup 1)
	  (match_dup 2)))]
  ""
  ;; cla
  ;; lio %2
  ;; sil 1
  ;; div %1
  ;; nop
  ;; dio %3
  ;; dac %0
  "cla\\n\\tlio\\t%1\\n\\tsil\\t1\\n\\tdiv\\t%2\\n\\tnop\\n\\tdio\\t%3\\n\\tdac\\t%0"
  [(set_attr "length" "7")])

;; -------------------------------------------------------------------------
;; logic instruction
;; -------------------------------------------------------------------------

(define_insn "one_cmplqi2"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g")
	(neg:QI
	  (match_operand:QI 1 "general_operand" "rm,i")))]
  ""
  "@
  lac\\t%1\\n\\tcma\\n\\tdac\\t%0
  law\\t%1\\n\\tcma\\n\\tdac\\t%0"
  [(set_attr "length" "3,3")])

(define_insn "andqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g")
	(and:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g,g")
	  (match_operand:QI 2 "general_operand" "rm,i")))]
  ""
  "@
  lac\\t%2\\n\\tand\\t%1\\n\\tdac\\t%0
  law\\t%2\\n\\tand\\t%1\\n\\tdac\\t%0"
  [(set_attr "length" "3,3")])

(define_insn "iorqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g")
	(ior:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g,g")
	  (match_operand:QI 2 "general_operand" "rm,i")))]
  ""
  "@
  lac\\t%2\\n\\tior\\t%1\\n\\tdac\\t%0
  law\\t%2\\n\\tior\\t%1\\n\\tdac\\t%0"
  [(set_attr "length" "3,3")])

(define_insn "xorqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g,g")
	(xor:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g,g")
	  (match_operand:QI 2 "general_operand" "rm,i")))]
  ""
  "@
  lac\\t%2\\n\\txor\\t%1\\n\\tdac\\t%0
  law\\t%2\\n\\txor\\t%1\\n\\tdac\\t%0"
  [(set_attr "length" "3,3")])

;; -------------------------------------------------------------------------
;; shift instruction
;; -------------------------------------------------------------------------

(define_insn "ashlqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g")
	(ashift:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g")
	  (match_operand:QI 2 "pdp1_shift_rotate_amount" "i")))]
  ""
  "lac\\t%1\\n\\tsal\\t%2\\n\\tdac\\t%0"
  [(set_attr "length" "3")])

(define_insn "ashrqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g")
	(ashiftrt:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g")
	  (match_operand:QI 2 "pdp1_shift_rotate_amount" "i")))]
  ""
  "lac\\t%1\\n\\tsar\\t%2\\n\\tdac\\t%0"
  [(set_attr "length" "3")])

(define_insn "rotlqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g")
	(rotate:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g")
	  (match_operand:QI 2 "pdp1_shift_rotate_amount" "i")))]
  ""
  "lac\\t%1\\n\\tral\\t%2\\n\\tdac\\t%0"
  [(set_attr "length" "3")])

(define_insn "rotrqi3"
  [(set (match_operand:QI 0 "nonimmediate_operand" "=g")
	(rotatert:QI
	  (match_operand:QI 1 "nonimmediate_operand" "g")
	  (match_operand:QI 2 "pdp1_shift_rotate_amount" "i")))]
  ""
  "lac\\t%1\\n\\trar\\t%2\\n\\tdac\\t%0"
  [(set_attr "length" "3")])

;; -------------------------------------------------------------------------
;; jmp instruction
;; -------------------------------------------------------------------------

(define_insn "indirect_jump"
  [(set (pc) (match_operand:QI 0 "nonimmediate_operand" "r"))]
  ""
  "jmp.i\\t%0"
  [(set_attr "length" "1")])

(define_insn "jump"
  [(set (pc)
        (label_ref (match_operand 0 "" "")))]
  ""
  "jmp\\t%l0"
  [(set_attr "length" "1")])

(define_insn "call"
  [(call (match_operand:QI 0 "memory_operand" "")
         (match_operand:QI 1 "general_operand" ""))]
  ""
  "jsp\\t%0"
  [(set_attr "length" "1")])

(define_insn "call_value"
  [(set (match_operand 0 "register_operand" "")
        (call (match_operand:QI 1 "memory_operand" "")
              (match_operand:QI 2 "general_operand" "")))]
  ""
  "jsp\\t%1"
  [(set_attr "length" "1")])

;; -------------------------------------------------------------------------
;; compare and conditionals
;; -------------------------------------------------------------------------

(define_insn "cbranchqi4"
  [(set (pc)
        (if_then_else
	  (match_operator 0 "pdp1_comparison_operator"
	    [(match_operand:QI 1 "nonimmediate_operand" "g")
	     (match_operand:QI 2 "nonimmediate_operand" "g")])
	  (label_ref (match_operand 3 "" ""))
	  (pc)))]
  ""
{
  switch (GET_CODE (operands[0])) {
  case EQ:
    return "lac\\t%1\\n\\t" 
           "sad\\t%2\\n\\t"
           "jmp\\t%l3";
  case NE:
    return "lac\\t%1\\n\\t" 
           "sas\\t%2\\n\\t"
           "jmp\\t%l3";

  case GT:
    /* 0500 = sza | sma */
    return "lac\\t%1\\n\\t" 
           "sub\\t%2\\n\\t"
           "skp\\t0500\\n\\t"
           "jmp\\t%l3\\n\\t";

  case LT:
    /* 0300 = sza | spa */
    return "lac\\t%1\\n\\t" 
           "sub\\t%2\\n\\t"
           "skp\\t0300\\n\\t"
           "jmp\\t%l3\\n\\t";
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
