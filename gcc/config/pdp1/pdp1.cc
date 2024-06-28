#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "stringpool.h"
#include "attribs.h"
#include "df.h"
#include "regs.h"
#include "memmodel.h"
#include "emit-rtl.h"
#include "diagnostic-core.h"
#include "output.h"
#include "stor-layout.h"
#include "varasm.h"
#include "calls.h"
#include "expr.h"
#include "builtins.h"
#include "explow.h"

/* This file should be included last.  */
#include "target-def.h"

#define LOSE_AND_RETURN(msgid, x)		\
  do						\
    {						\
      pdp1_operand_lossage (msgid, x);		\
      return;					\
    } while (0)

typedef struct GTY(()) machine_function
{
  HOST_WIDE_INT callee_saved_reg_size;
  HOST_WIDE_INT local_vars_size;
  HOST_WIDE_INT args_size;
  HOST_WIDE_INT total_size;
} machine_function;

static struct machine_function *
pdp1_init_machine_status (void)
{
  return ggc_cleared_alloc<machine_function> ();
}

static void
pdp1_option_override (void)
{
  init_machine_status = pdp1_init_machine_status;
}

/* Worker function for TARGET_RETURN_IN_MEMORY.  */

static bool
pdp1_return_in_memory (const_tree type, const_tree fntype ATTRIBUTE_UNUSED)
{
  const HOST_WIDE_INT size = int_size_in_bytes (type);
  return (size == -1 || size > 2 * UNITS_PER_WORD);
}

static void
pdp1_compute_frame (void)
{
  int regno;

  cfun->machine->local_vars_size = get_frame_size ();
  cfun->machine->args_size = crtl->outgoing_args_size;
  cfun->machine->callee_saved_reg_size = 0;

  /* save callee-saved regs */
  for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
    if (df_regs_ever_live_p (regno) && !call_used_or_fixed_reg_p (regno))
      cfun->machine->callee_saved_reg_size++;

  cfun->machine->total_size = cfun->machine->local_vars_size +
	  cfun->machine->args_size +
	  cfun->machine->callee_saved_reg_size;
}

#undef TARGET_COMPUTE_FRAME_LAYOUT
#define TARGET_COMPUTE_FRAME_LAYOUT pdp1_compute_frame

int
pdp1_initial_elimination_offset (int from, int to)
{
  int ret;

  switch (from)
    {
    case ARG_POINTER_REGNUM:
      ret = cfun->machine->total_size;
      break;
    case FRAME_POINTER_REGNUM:
      ret = cfun->machine->args_size + cfun->machine->local_vars_size;
      break;
    default:
      gcc_unreachable ();
    }

  if (to == HARD_FRAME_POINTER_REGNUM)
    ret -= cfun->machine->total_size;

  return ret;
}

static bool
pdp1_legitimate_address_p (machine_mode mode,
                           rtx x, bool strict_p,
			   addr_space_t as,
			   code_helper = ERROR_MARK)
{
    if (CONSTANT_ADDRESS_P (x))
      return true;

    switch (GET_CODE (x))
      {
      case REG:
      case MEM:
        return true;
      }

    return false;
}

static void
pdp1_operand_lossage (const char *msgid, rtx op)
{
  debug_rtx (op);
  output_operand_lossage ("%s", msgid);
}

static void
pdp1_print_operand_address (FILE *file, machine_mode, rtx x)
{
  switch (GET_CODE (x))
    {
    case REG:
    case MEM:
      fseek (file, -1, SEEK_CUR);
      fprintf (file, ".i\t%s", reg_names[REGNO (x)]);
      break;

    default:
      output_addr_const (file, x);
      break;
    }
}

static void
pdp1_print_operand (FILE *file, rtx x, int code)
{
  rtx operand = x;

  switch (code)
    {
    case 0:
      /* No code, print as usual.  */
      break;

    default:
      LOSE_AND_RETURN ("invalid operand modifier letter", x);
    }

  /* Print an operand as without a modifier letter.  */
  switch (GET_CODE (operand))
    {
    case REG:
      fprintf (file, "%s", reg_names[REGNO (operand)]);
      return;

    case MEM:
      output_address (GET_MODE (XEXP (operand, 0)), XEXP (operand, 0));
      return;

    default:
      if (CONSTANT_P (operand))
	{
	  output_addr_const (file, operand);
	  return;
	}

      LOSE_AND_RETURN ("unexpected operand", x);
    }
}


/* 
 * GCC expects the port to be easily able to mov return value to this register,
 * so let's use the PDP1_IO for that.
 */
static rtx
pdp1_function_value (const_tree valtype,
                     const_tree fntype_or_decl ATTRIBUTE_UNUSED,
		     bool outgoing ATTRIBUTE_UNUSED)
{
  int regno = PDP1_IO;

  return gen_rtx_REG (TYPE_MODE (valtype), regno);
}

static bool
pdp1_function_value_regno_p (const unsigned int regno)
{
  return (regno == PDP1_IO);
}

#define PDP1_FUNCTION_ARG_SIZE(MODE, TYPE)	\
  ((MODE) != BLKmode ? GET_MODE_SIZE (MODE)	\
   : (unsigned) int_size_in_bytes (TYPE))

static void
pdp1_function_arg_advance (cumulative_args_t cum_v,
                           const function_arg_info &arg)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);
  *cum = *cum + 1;
}

/* Return the next register to be used to hold a function argument or
   NULL_RTX if there's no more space.  */
static rtx
pdp1_function_arg (cumulative_args_t cum_v, const function_arg_info &arg)
{
  CUMULATIVE_ARGS *cum = get_cumulative_args (cum_v);

  if (*cum < 8)
    return gen_rtx_REG (arg.mode, *cum);
  else 
    return NULL_RTX;
}

bool
pdp1_check_if_acc (rtx reg)
{
  return REG_P (reg) && (REGNO (reg) == PDP1_ACC);
}

void
pdp1_expand_prologue ()
{
  int regno;
  rtx insn;
  rtx acc = gen_rtx_REG (Pmode, PDP1_ACC);

  /* let's save acc on the stack, because it holds the reset addr */
  insn = emit_insn (gen_movqi_push (acc));
  RTX_FRAME_RELATED_P (insn) = 1;

  /* push fp */
  insn = emit_move_insn (acc, hard_frame_pointer_rtx);
  RTX_FRAME_RELATED_P (insn) = 1;
  insn = emit_insn (gen_movqi_push (acc));
  RTX_FRAME_RELATED_P (insn) = 1;

  /* set fp = sp */
  insn = emit_move_insn (acc, stack_pointer_rtx);
  RTX_FRAME_RELATED_P (insn) = 1;
  insn = emit_move_insn (hard_frame_pointer_rtx, acc);
  RTX_FRAME_RELATED_P (insn) = 1;

  /* 
   * increment fp - this is needed because we'll reference local vars starting
   * with an offset of -1
   *
   * TODO: can this be handled better? 
   */
  insn = emit_move_insn (acc, GEN_INT (1));
  RTX_FRAME_RELATED_P (insn) = 1;
  insn = emit_insn (gen_addqi3 (acc, acc, hard_frame_pointer_rtx));
  RTX_FRAME_RELATED_P (insn) = 1;
  insn = emit_move_insn (hard_frame_pointer_rtx, acc);
  RTX_FRAME_RELATED_P (insn) = 1;
  
  /* save callee-saved regs */
  for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
    {
      if (df_regs_ever_live_p (regno) && !call_used_or_fixed_reg_p (regno))
        {
          insn = emit_move_insn (acc, gen_rtx_REG (Pmode, regno));
          RTX_FRAME_RELATED_P (insn) = 1;
          insn = emit_insn (gen_movqi_push (acc));
          RTX_FRAME_RELATED_P (insn) = 1;
        }
    }

  /* make space for local vars on the stack */
  if (cfun->machine->local_vars_size > 0)
    {
      insn = emit_move_insn (acc, GEN_INT (-cfun->machine->local_vars_size));
      RTX_FRAME_RELATED_P (insn) = 1;
      insn = emit_insn (gen_addqi3 (acc, acc, stack_pointer_rtx));
      RTX_FRAME_RELATED_P (insn) = 1;
      insn = emit_move_insn (stack_pointer_rtx, acc);
      RTX_FRAME_RELATED_P (insn) = 1;
    }

  /* make space for args */
  if (cfun->machine->args_size > 0)
    {
      insn = emit_move_insn (acc, GEN_INT (-cfun->machine->args_size));
      RTX_FRAME_RELATED_P (insn) = 1;
      insn = emit_insn (gen_addqi3 (acc, acc, stack_pointer_rtx));
      RTX_FRAME_RELATED_P (insn) = 1;
      insn = emit_move_insn (stack_pointer_rtx, acc);
      RTX_FRAME_RELATED_P (insn) = 1;
    }
}

void
pdp1_expand_epilogue ()
{
  int regno;
  rtx insn;
  rtx reg;
  rtx acc = gen_rtx_REG (Pmode, PDP1_ACC);

  if (cfun->machine->args_size > 0)
    {
      emit_move_insn (acc, GEN_INT (cfun->machine->args_size));
      emit_insn ( gen_addqi3 (acc, acc, stack_pointer_rtx));
      emit_move_insn (stack_pointer_rtx, acc);
    }

  if (cfun->machine->local_vars_size > 0)
    {
      emit_move_insn (acc, GEN_INT (cfun->machine->local_vars_size));
      emit_insn ( gen_addqi3 (acc, acc, stack_pointer_rtx));
      emit_move_insn (stack_pointer_rtx, acc);
    }

  /* restore callee-saved regs */
  for (regno = FIRST_PSEUDO_REGISTER - 1; regno >= 0; regno--)
    {
      if (df_regs_ever_live_p (regno) && !call_used_or_fixed_reg_p (regno))
        {
          reg = gen_rtx_REG (Pmode, regno);
          emit_insn (gen_movqi_pop (reg));
        }
    }

  /* restore fp */
  emit_insn (gen_movqi_pop (hard_frame_pointer_rtx));

  /* restore return addr and move it to r0 */
  reg = gen_rtx_REG (Pmode, PDP1_R0);
  emit_insn (gen_movqi_pop (reg));

  /* returner */
  emit_insn (gen_indirect_jump (reg));
}

static reg_class_t
pdp1_secondary_reload (bool in_p ATTRIBUTE_UNUSED,
			rtx x,
			reg_class_t reload_class,
			machine_mode reload_mode ATTRIBUTE_UNUSED,
			secondary_reload_info *sri ATTRIBUTE_UNUSED)
{
  if (reload_class == HW_REGS)
    return NO_REGS;

  if (REGNO_REG_CLASS (REGNO (x)) == HW_REGS)
    return NO_REGS;

  return HW_REGS;
}

static reg_class_t
pdp1_preferred_reload_class (rtx x, reg_class_t rclass)
{
  return HW_REGS;
}

static reg_class_t
pdp1_preferred_output_reload_class (rtx x, reg_class_t rclass)
{
  return HW_REGS;
}

#undef TARGET_PROMOTE_PROTOTYPES
#define TARGET_PROMOTE_PROTOTYPES hook_bool_const_tree_true

#undef TARGET_RETURN_IN_MEMORY
#define TARGET_RETURN_IN_MEMORY pdp1_return_in_memory
#undef TARGET_MUST_PASS_IN_STACK
#define TARGET_MUST_PASS_IN_STACK must_pass_in_stack_var_size
#undef TARGET_PASS_BY_REFERENCE
#define TARGET_PASS_BY_REFERENCE hook_pass_by_reference_must_pass_in_stack

#undef  TARGET_ADDR_SPACE_LEGITIMATE_ADDRESS_P
#define TARGET_ADDR_SPACE_LEGITIMATE_ADDRESS_P	pdp1_legitimate_address_p

#undef  TARGET_PRINT_OPERAND
#define TARGET_PRINT_OPERAND pdp1_print_operand
#undef  TARGET_PRINT_OPERAND_ADDRESS
#define TARGET_PRINT_OPERAND_ADDRESS pdp1_print_operand_address

#undef TARGET_FUNCTION_VALUE
#define TARGET_FUNCTION_VALUE pdp1_function_value
#undef TARGET_FUNCTION_VALUE_REGNO_P
#define TARGET_FUNCTION_VALUE_REGNO_P pdp1_function_value_regno_p
#undef  TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE pdp1_function_arg_advance
#undef TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG pdp1_function_arg

#undef TARGET_OPTION_OVERRIDE
#define TARGET_OPTION_OVERRIDE pdp1_option_override

#undef  TARGET_SECONDARY_RELOAD
#define TARGET_SECONDARY_RELOAD pdp1_secondary_reload

#undef  TARGET_PREFERRED_RELOAD_CLASS
#define TARGET_PREFERRED_RELOAD_CLASS pdp1_preferred_reload_class

#undef  TARGET_PREFERRED_OUTPUT_RELOAD_CLASS
#define TARGET_PREFERRED_OUTPUT_RELOAD_CLASS pdp1_preferred_output_reload_class

static void
pdp1_asm_named_section (const char *name, unsigned int flags ATTRIBUTE_UNUSED,
                        tree decl ATTRIBUTE_UNUSED)
{
  fprintf (asm_out_file, "\t.section %s\n", name);
}

static bool
pdp1_class_likely_spilled_p (reg_class_t c)
{
  switch (c)
    {
      case ACC_REG:
      case IO_REG:
      case HW_REGS:
        return true;
      default:
	return false;
    }
}

#undef TARGET_CLASS_LIKELY_SPILLED_P
#define TARGET_CLASS_LIKELY_SPILLED_P pdp1_class_likely_spilled_p

static int
pdp1_register_move_cost (machine_mode mode ATTRIBUTE_UNUSED,
                         reg_class_t from, reg_class_t to)
{
  if (from == HW_REGS && to == HW_REGS)
    return 4;
  if (from == HW_REGS && to == GENERAL_REGS)
    return 2;
  if (from == GENERAL_REGS && to == HW_REGS)
    return 2;
  else
    return 4;
}

#undef TARGET_REGISTER_MOVE_COST
#define TARGET_REGISTER_MOVE_COST pdp1_register_move_cost

struct gcc_target targetm = TARGET_INITIALIZER;

#include "gt-pdp1.h"
