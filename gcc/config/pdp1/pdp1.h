#ifndef GCC_PDP1_H
#define GCC_PDP1_H

#undef PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE NO_DEBUG

#undef ASM_GENERATE_INTERNAL_LABEL
#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM)	\
  sprintf ((LABEL), "*%s%ld", (PREFIX), (long)(NUM))

#undef ASM_OUTPUT_COMMON
#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)  \
( fputs ("\t.common ", (FILE)),		\
  assemble_name ((FILE), (NAME)),		\
  fprintf ((FILE), "," HOST_WIDE_INT_PRINT_UNSIGNED",\"bss\"\n", (SIZE)))

#undef ASM_OUTPUT_SKIP
#define ASM_OUTPUT_SKIP(FILE,SIZE)  \
  fprintf (FILE, "\t.skip " HOST_WIDE_INT_PRINT_UNSIGNED"\n", (SIZE))

#undef ASM_OUTPUT_LOCAL
#define ASM_OUTPUT_LOCAL(FILE, NAME, SIZE, ROUNDED)	\
  (fputs ("\t.lcomm\t", FILE),				\
  assemble_name (FILE, NAME),				\
  fprintf (FILE, ",%d\n", (int)SIZE))

/* Number of storage units in a word; normally the size of a
   general-purpose register, a power of two from 1 or 8.  */
#define BITS_PER_WORD 16
#define UNITS_PER_WORD 1

#define INT_TYPE_SIZE 16
#define SHORT_TYPE_SIZE 16
#define LONG_TYPE_SIZE 16
#define LONG_LONG_TYPE_SIZE 32

#define DEFAULT_SIGNED_CHAR 0

#undef  SIZE_TYPE
#define SIZE_TYPE "unsigned int"

#undef  PTRDIFF_TYPE
#define PTRDIFF_TYPE "int"

#undef  WCHAR_TYPE
#define WCHAR_TYPE "unsigned int"

#undef  WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE BITS_PER_WORD

#define FILE_ASM_OP     "\t.file\n"

/* Switch to the text or data segment.  */
#define TEXT_SECTION_ASM_OP  "\t.text"
#define DATA_SECTION_ASM_OP  "\t.data"

/* 
 * Location counter alignment is not really supported for PDP-1 right now, so
 * just spew out the hardcoded 'align 1'
 */
#define ASM_OUTPUT_ALIGN(STREAM,POWER) \
	fprintf (STREAM, "#\t.align\t%d\n", 1);

/* Output and Generation of Labels */

#define GLOBAL_ASM_OP "\t.global\t"

/* The Overall Framework of an Assembler File */

#define ASM_COMMENT_START "#"
#define ASM_APP_ON ""
#define ASM_APP_OFF ""


/* PDP-1 doesn't have general purpose registers, so we'll just use memory cells
 * 200-299 for that purpose:
 *
 * General purpose:
 *
 *   200 - r0
 *   201 - r1
 *   202 - r2
 *   203 - r3
 *   204 - r4
 *   205 - r5
 *   206 - r6
 *   207 - r7
 *   208 - frame pointer
 *   209 - stack pointer
 *
 * Special registers:
 *
 *   pc  - program counter
 *   acc - accumulator register
 */

#define PDP1_R0      0
#define PDP1_R1      1
#define PDP1_R2      2
#define PDP1_R3      3
#define PDP1_R4      4
#define PDP1_R5      5
#define PDP1_R6      6
#define PDP1_R7      7
#define PDP1_R8      8
#define PDP1_R9      9
#define PDP1_R10     10
#define PDP1_R11     11
#define PDP1_R12     12
#define PDP1_R13     13
#define PDP1_R14     14
#define PDP1_R15     15
#define PDP1_R16     16
#define PDP1_R17     17
#define PDP1_R18     18
#define PDP1_R19     19
#define PDP1_R20     20
#define PDP1_R21     21
#define PDP1_R22     22
#define PDP1_R23     23
#define PDP1_FP      24 
#define PDP1_SP      25
#define PDP1_ACC     26
#define PDP1_IO      27
#define PDP1_QFP     28
#define PDP1_QAP     29
#define PDP1_PC      30
#define PDP1_CC      31

#define FIRST_PSEUDO_REGISTER (PDP1_PC + 1)
#define AVOID_CCMODE_COPIES 1

enum reg_class
{
  NO_REGS,
  GENERAL_REGS,
  ACC_REG,
  IO_REG,
  HW_REGS,
  SPECIAL_REGS,
  CC_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

#define N_REG_CLASSES LIM_REG_CLASSES

#define REG_CLASS_NAMES {	\
  "NO_REGS",			\
  "GENERAL_REGS",		\
  "ACC_REG",			\
  "IO_REG",			\
  "HW_REGS",			\
  "SPECIAL_REGS",		\
  "CC_REGS",			\
  "ALL_REGS"			\
}

/* nth reg is nth bit in the reg class mask */
#define REG_CLASS_CONTENTS \
{ { 0x00000000 }, /* Empty */ \
  { 0x03FFFFFF }, /* R0-R7, FP, SP = 2^10 - 1 */ \
  { 0x04000000 }, /* $acc = 1 << 10 */ \
  { 0x08000000 }, /* $io = 1 << 11 */ \
  { 0x0C000000 }, /* $io and $acc - for secondary reloads etc */ \
  { 0x70000000 }, /* ?qfp, ?qap, $pc */ \
  { 0x80000000 }, /* ?cc = 1 << 15 */ \
  { 0xFFFFFFFF }  /* All registers */ \
}

#define REGISTER_NAMES {		\
  "0100", "0101", "0102", "0103",	\
  "0104", "0105", "0106", "0107",	\
  "0110", "0111", "0112", "0113",	\
  "0114", "0115", "0116", "0117",	\
  "0120", "0121", "0122", "0123",	\
  "0124", "0125", "0126", "0127",	\
  "0130", "0131", /* $fp and $sp */	\
  "$acc", "$io",			\
  "?fp", "?ap", "$pc",			\
  "?cc"					\
}

#define FIXED_REGISTERS {	\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  1, 1,				\
  0, 0,				\
  1, 1, 1,			\
  1,				\
}

#define CALL_USED_REGISTERS {	\
  1, 1, 1, 1,			\
  1, 1, 1, 1,			\
  1, 1, 1, 1,			\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  0, 0, 0, 0,			\
  1, 1,				\
  1, 1,				\
  1, 1, 1,			\
  1,				\
}

/* A C expression whose value is a register class containing hard
   register REGNO.  */

static inline enum reg_class pdp1_regno_reg_class(int regno)
{
  if (regno <= PDP1_SP)
    return GENERAL_REGS;
  if ((regno == PDP1_ACC) || (regno == PDP1_IO))
    return HW_REGS;
  if (regno == PDP1_CC)
    return CC_REGS;

  return SPECIAL_REGS;
}

#define REGNO_REG_CLASS(R) pdp1_regno_reg_class(R)


/* Passing Arguments in Registers */

/* A C type for declaring a variable that is used as the first
   argument of `FUNCTION_ARG' and other related values.  */
#define CUMULATIVE_ARGS unsigned int

/* If defined, the maximum amount of space required for outgoing arguments
   will be computed and placed into the variable
   `current_function_outgoing_args_size'.  No space will be pushed
   onto the stack for each call; instead, the function prologue should
   increase the stack frame size by this amount.  */
#define ACCUMULATE_OUTGOING_ARGS 1

/* A C statement (sans semicolon) for initializing the variable CUM
   for the state at the beginning of the argument list.  
   For pdp1, the first arg is passed in register 2 (aka $r0).  */
#define INIT_CUMULATIVE_ARGS(CUM,FNTYPE,LIBNAME,FNDECL,N_NAMED_ARGS) \
  (CUM = PDP1_R0)

/* How Scalar Function Values Are Returned */

/* STACK AND CALLING */

/* Define this macro if pushing a word onto the stack moves the stack
   pointer to a smaller address.  */
#define STACK_GROWS_DOWNWARD 1

/* Offset from the argument pointer register to the first argument's
   address.  On some machines it may depend on the data type of the
   function.  */
#define FIRST_PARM_OFFSET(F) 0

/* Define this macro to nonzero value if the addresses of local variable slots
   are at negative offsets from the frame pointer.  */
#define FRAME_GROWS_DOWNWARD 1

/* Define this macro as a C expression that is nonzero for registers that are
   used by the epilogue or the return pattern.  The stack and frame
   pointer registers are already assumed to be used as needed.  */
#define EPILOGUE_USES(R) (R == PDP1_R0)

/* A C expression whose value is RTL representing the location of the
   incoming return address at the beginning of any function, before
   the prologue.  */
#define INCOMING_RETURN_ADDR_RTX					\
  gen_frame_mem (Pmode,							\
		 plus_constant (Pmode, stack_pointer_rtx, UNITS_PER_WORD))

/* Describe how we implement __builtin_eh_return.  */
#define EH_RETURN_DATA_REGNO(N)	((N) < 4 ? (N+2) : INVALID_REGNUM)

/* Store the return handler into the call frame.  */
#define EH_RETURN_HANDLER_RTX						\
  gen_frame_mem (Pmode,							\
		 plus_constant (Pmode, frame_pointer_rtx, UNITS_PER_WORD))

/* Storage Layout */

#define BITS_BIG_ENDIAN 1
#define BYTES_BIG_ENDIAN 1
#define WORDS_BIG_ENDIAN 1

/* Alignment required for a function entry point, in bits.  */
#define FUNCTION_BOUNDARY 16

/* Define this macro as a C expression which is nonzero if accessing
   less than a word of memory (i.e. a `char' or a `short') is no
   faster than accessing a word of memory.  */
#define SLOW_BYTE_ACCESS 1


/* Define this macro to the minimum alignment enforced by hardware
   for the stack pointer on this machine.  The definition is a C
   expression for the desired alignment (measured in bits).  */
#define STACK_BOUNDARY 16

/* Normal alignment required for function parameters on the stack, in
   bits.  All stack parameters receive at least this much alignment
   regardless of data type.  */
#define PARM_BOUNDARY 16

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY  16

/* No data type wants to be aligned rounder than this.  */
#define BIGGEST_ALIGNMENT 16

/* The best alignment to use in cases where we have a choice.  */
#define FASTEST_ALIGNMENT 16

/* Every structures size must be a multiple of 8 bits.  */
#define STRUCTURE_SIZE_BOUNDARY 16

/* Look at the fundamental type that is used for a bit-field and use 
   that to impose alignment on the enclosing structure.
   struct s {int a:8}; should have same alignment as "int", not "char".  */
#define	PCC_BITFIELD_TYPE_MATTERS	1

/* Largest integer machine mode for structures.  If undefined, the default
   is GET_MODE_SIZE(DImode).  */
#define MAX_FIXED_MODE_SIZE 16

/* Make arrays of chars word-aligned for the same reasons.  */
#define DATA_ALIGNMENT(TYPE, ALIGN)		\
  (TREE_CODE (TYPE) == ARRAY_TYPE		\
   && TYPE_MODE (TREE_TYPE (TYPE)) == QImode	\
   && (ALIGN) < FASTEST_ALIGNMENT ? FASTEST_ALIGNMENT : (ALIGN))
     
/* Set this nonzero if move instructions will actually fail to work
   when given unaligned data.  */
#define STRICT_ALIGNMENT 1

/* Generating Code for Profiling */
#define FUNCTION_PROFILER(FILE,LABELNO) (abort (), 0)

/* Trampolines for Nested Functions.  */
#define TRAMPOLINE_SIZE (2 + 6 + 4 + 2 + 6)

/* Alignment required for trampolines, in bits.  */
#define TRAMPOLINE_ALIGNMENT 16

/* An alias for the machine mode for pointers.  */
#define Pmode         QImode

/* An alias for the machine mode used for memory references to
   functions being called, in `call' RTL expressions.  */
#define FUNCTION_MODE QImode

/* The register number of the stack pointer register, which must also
   be a fixed register according to `FIXED_REGISTERS'.  */
#define STACK_POINTER_REGNUM PDP1_SP

/* The register number of the frame pointer register, which is used to
   access automatic variables in the stack frame.  */
#define FRAME_POINTER_REGNUM PDP1_QFP

/* The register number of the arg pointer register, which is used to
   access the function's argument list.  */
#define ARG_POINTER_REGNUM PDP1_QAP

#define HARD_FRAME_POINTER_REGNUM PDP1_FP

#define ELIMINABLE_REGS							\
{{ FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM },			\
 { FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM },			\
 { ARG_POINTER_REGNUM,   STACK_POINTER_REGNUM },	        	\
 { ARG_POINTER_REGNUM,   HARD_FRAME_POINTER_REGNUM }}			

/* This macro returns the initial difference between the specified pair
   of registers.  */
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET)			\
  do {									\
    (OFFSET) = pdp1_initial_elimination_offset ((FROM), (TO));		\
  } while (0)

/* A C expression that is nonzero if REGNO is the number of a hard
   register in which function arguments are sometimes passed.  */
#define FUNCTION_ARG_REGNO_P(r) (r >= PDP1_R0 && r <= PDP1_R5)

/* A macro whose definition is the name of the class to which a valid
   base register must belong.  A base register is one used in an
   address which is the register value plus a displacement.  */
#define BASE_REG_CLASS GENERAL_REGS
#define INDEX_REG_CLASS NO_REGS
#define REGNO_OK_FOR_INDEX_P(REGNO) 0
#define REGNO_OK_FOR_BASE_P(REGNO) ((REGNO) <= PDP1_SP)

/* The maximum number of bytes that a single instruction can move
   quickly between memory and registers or between two memory
   locations.  */
#define MOVE_MAX 1

/* All load operations zero extend.  */
#define LOAD_EXTEND_OP(MEM) ZERO_EXTEND

/* A number, the maximum number of registers that can appear in a
   valid memory address.  */
#define MAX_REGS_PER_ADDRESS 1

/* An alias for a machine mode name.  This is the machine mode that
   elements of a jump-table should have.  */
#define CASE_VECTOR_MODE QImode

/* Run-time Target Specification */

#define TARGET_CPU_CPP_BUILTINS() \
  { \
    builtin_define_std ("pdp1");			\
    builtin_define_std ("PDP1");			\
  }

#define HAS_LONG_UNCOND_BRANCH true

#define LIBCALL_VALUE(MODE) \
  gen_rtx_REG (MODE, PDP1_R2)

#define TARGET_ASM_NAMED_SECTION pdp1_asm_named_section
#define TARGET_SMALL_REGISTER_CLASSES_FOR_MODE_P hook_bool_mode_true
#define TARGET_REGISTER_USAGE_LEVELING hook_bool_void_true

#endif /* GCC_PDP1_H */
