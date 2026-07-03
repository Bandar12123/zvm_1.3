#ifndef __ZVM_API_H__
#define __ZVM_API_H__

typedef struct zvm_vm_t zvm_vm_t;
typedef struct zvm_cpu_t zvm_cpu_t;
typedef struct zvm_io_device_t zvm_io_device_t;

typedef struct zvm_program_t zvm_program_t;
typedef struct zvm_instruction_t zvm_instruction_t;
typedef struct zvm_instruction_handler_t zvm_instruction_handler_t;
typedef struct zvm_instruction_metadata_t zvm_instruction_metadata_t;
typedef struct zvm_operand_t zvm_operand_t;
typedef struct zvm_operand_metadata_t zvm_operand_metadata_t;

typedef struct zvm_exception_metadata_t zvm_exception_metadata_t;
typedef struct zvm_exception_entry_t zvm_exception_entry_t;

typedef bool (*zfn_instruction_handler_t)(zvm_vm_t* vm, uint8_t, uint8_t, uint8_t);
typedef bool (*zfn_io_handler_t)(zvm_vm_t *vm,  uint8_t port, uint8_t operation,  uint8_t argc);
typedef bool (*zfn_exception_handler_t)(zvm_vm_t* vm, uint8_t code);

/* Program */

#define zvm_is_address(v)  ((v) < ZVM_PROGRAM_DEFAULT_DATA_SEGMENT_SIZE)
#define zvm_is_stack_address zvm_is_address

/* Instruction */
#define ZVM_INSTRUCTION_MAX_OPCODE 255
#define ZVM_INSTRUCTION_SIZE 4

#define ZVM_MAX_INSTRUCTIONS_COUNT (ZVM_PROGRAM_DEFAULT_CODE_SEGMENT_SIZE / ZVM_INSTRUCTION_SIZE)

#define zvm_is_valid_instruction(opcode) ((opcode) < ZVM_INSTRUCTION_HANDLERS_COUNT)

#define ZVM_INSTRUCTION_HANDLER_FUNCTION_NAME(name)   zfnx_ ## name
#define ZVM_INSTRUCTION_HANDLER_FUNCTION_PARAM \
    zvm_vm_t* vm, uint8_t left, uint8_t right, uint8_t output

#define ZVM_INSTRUCTION_HANDLER_FUNCTION_ARGS \
    vm, left, right, output

#define ZVM_INSTRUCTION_HANDLER_FUNCTION(name)  \
    bool ZVM_INSTRUCTION_HANDLER_FUNCTION_NAME(name) (ZVM_INSTRUCTION_HANDLER_FUNCTION_PARAM)


#define ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(name) \
    ZVM_INSTRUCTION_HANDLER_FUNCTION(name) {

#define ZVM_INSTRUCTION_HANDLER_FUNCTION_END    \
        return true;                            \
    }

#define ZVM_INSTRUCTION_HANDLER_CALL(name) zfnx_ ## name(ZVM_INSTRUCTION_HANDLER_FUNCTION_ARGS)

#define ZVM_OPERAND_TYPE_REG 0
#define ZVM_OPERAND_TYPE_MEM 1
#define ZVM_OPERAND_TYPE_IMM 2

#define ZVM_INST_0_OP  0
#define ZVM_INST_1_OP  1
#define ZVM_INST_2_OP  2
#define ZVM_INST_3_OP  3


#define ZVM_OPCODE_ADD        0  /*  ADD reg, reg, reg  */
#define ZVM_OPCODE_SUB        1  /*  SUB reg, reg, reg  */
#define ZVM_OPCODE_LDI        2  /*  LDI reg, imm       */
#define ZVM_OPCODE_STR        3  /*  STR reg, mem       */
#define ZVM_OPCODE_LDM        4  /*  LDM reg, mem       */
#define ZVM_OPCODE_LDA        5  /*  LDA reg, reg       */
#define ZVM_OPCODE_STA        6  /*  STA reg, reg       */
#define ZVM_OPCODE_PUSH       7  /*  PUSH reg           */
#define ZVM_OPCODE_POP        8  /*  POP reg            */
#define ZVM_OPCODE_INC        9  /*  INC reg            */
#define ZVM_OPCODE_DEC        10 /*  DEC reg            */
#define ZVM_OPCODE_IN         11 /*  IN reg(port), reg(function), reg(argc)   */
#define ZVM_OPCODE_OUT        12 /*  OUT reg(port), reg(function), reg(argc)   */


/* instructions handlers */
ZVM_INSTRUCTION_HANDLER_FUNCTION(add);
ZVM_INSTRUCTION_HANDLER_FUNCTION(sub);
ZVM_INSTRUCTION_HANDLER_FUNCTION(ldi);
ZVM_INSTRUCTION_HANDLER_FUNCTION(str);
ZVM_INSTRUCTION_HANDLER_FUNCTION(ldm);
ZVM_INSTRUCTION_HANDLER_FUNCTION(lda);
ZVM_INSTRUCTION_HANDLER_FUNCTION(sta);

ZVM_INSTRUCTION_HANDLER_FUNCTION(push);
ZVM_INSTRUCTION_HANDLER_FUNCTION(pop);

ZVM_INSTRUCTION_HANDLER_FUNCTION(inc);
ZVM_INSTRUCTION_HANDLER_FUNCTION(dec);

ZVM_INSTRUCTION_HANDLER_FUNCTION(in);
ZVM_INSTRUCTION_HANDLER_FUNCTION(out);

#define ZVM_INST_HANDLER_TYPE_RRR  0
#define ZVM_INST_HANDLER_TYPE_RI   1
#define ZVM_INST_HANDLER_TYPE_RM   2
#define ZVM_INST_HANDLER_TYPE_RR   3
#define ZVM_INST_HANDLER_TYPE_R    4

#define ZVM_INSTRUCTION_HANDLERS_COUNT (sizeof(instruction_handlers) / sizeof(zvm_instruction_metadata_t))

#define ZVM_INSTRUCTION_HANDLER_ENTRY_BEGIN(name, argc) \
    [ZVM_OPCODE_ ## name] = {#name, ZVM_OPCODE_ ## name, ZVM_INST_ ## argc ## _OP, {

#define ZVM_INSTRUCTION_HANDLER_ENTRY_END(fn, type) \
    },{ZVM_INST_HANDLER_TYPE_ ## type, ZVM_INSTRUCTION_HANDLER_FUNCTION_NAME(fn)}},

#define ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(type) \
    {ZVM_OPERAND_TYPE_ ## type},

#define ZVM_INSTRUCTION_HANDLER_ENTRY_RRR(name, fn) \
     ZVM_INSTRUCTION_HANDLER_ENTRY_BEGIN(name, 3) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
    ZVM_INSTRUCTION_HANDLER_ENTRY_END(fn, RRR)

#define ZVM_INSTRUCTION_HANDLER_ENTRY_RI(name, fn) \
     ZVM_INSTRUCTION_HANDLER_ENTRY_BEGIN(name, 2) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(IMM) \
    ZVM_INSTRUCTION_HANDLER_ENTRY_END(fn, RI)

#define ZVM_INSTRUCTION_HANDLER_ENTRY_RM(name, fn) \
     ZVM_INSTRUCTION_HANDLER_ENTRY_BEGIN(name, 2) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(MEM) \
    ZVM_INSTRUCTION_HANDLER_ENTRY_END(fn, RM)

#define ZVM_INSTRUCTION_HANDLER_ENTRY_RR(name, fn) \
     ZVM_INSTRUCTION_HANDLER_ENTRY_BEGIN(name, 2) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
    ZVM_INSTRUCTION_HANDLER_ENTRY_END(fn, RR)

#define ZVM_INSTRUCTION_HANDLER_ENTRY_R(name, fn) \
     ZVM_INSTRUCTION_HANDLER_ENTRY_BEGIN(name, 1) \
        ZVM_INSTRUCTION_HANDLER_OPERAND_ENTRY(REG) \
    ZVM_INSTRUCTION_HANDLER_ENTRY_END(fn, R)

static const zvm_instruction_metadata_t instruction_handlers [ZVM_INSTRUCTION_MAX_OPCODE + 1] = {
    ZVM_INSTRUCTION_HANDLER_ENTRY_RRR(ADD, add)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RRR(SUB, sub)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RI(LDI, ldi)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RM(STR, str)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RM(LDM, ldm)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RR(LDA, lda)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RR(STA, sta)
    ZVM_INSTRUCTION_HANDLER_ENTRY_R(PUSH, push)
    ZVM_INSTRUCTION_HANDLER_ENTRY_R(POP, pop)
    ZVM_INSTRUCTION_HANDLER_ENTRY_R(INC, inc)
    ZVM_INSTRUCTION_HANDLER_ENTRY_R(DEC, dec)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RRR(IN, in)
    ZVM_INSTRUCTION_HANDLER_ENTRY_RRR(OUT, out)
};

#define zvm_has_next_instruction(vm) \
  ((vm)->cpu.IP < (vm)->program.instructions_count)

#define zvm_set_instruction_ex(p, iht, i, opcode, v1, v2, v3)\
    (p).instructions[(i)].metadata = (zvm_instruction_metadata_t*)&((iht)[ZVM_OPCODE_ ## opcode]);\
    (p).instructions[(i)].operands[0].metadata = (zvm_operand_metadata_t*)&((iht)[ZVM_OPCODE_ ## opcode]).operands[0];\
    (p).instructions[(i)].operands[1].metadata = (zvm_operand_metadata_t*)&((iht)[ZVM_OPCODE_ ## opcode]).operands[1];\
    (p).instructions[(i)].operands[2].metadata = (zvm_operand_metadata_t*)&((iht)[ZVM_OPCODE_ ## opcode]).operands[2];\
    (p).instructions[(i)].operands[0].value = (v1); \
    (p).instructions[(i)].operands[1].value = (v2); \
    (p).instructions[(i)].operands[2].value = (v3);

#define zvm_set_instruction(p, i, opcode, v1, v2, v3)  zvm_set_instruction_ex(p, instruction_handlers, i, opcode, v1, v2, v3)

#define R0  0
#define R1  1
#define R2  2
#define R3  3

#define INST(p, opcode, v1, v2, v3) \
    zvm_set_instruction(p, ((p).instructions_count), opcode, v1, v2, v3) \
    (p).instructions_count++;


#define ADD(p, v1, v2, v3) INST(p, ADD, v1, v2, v3)
#define SUB(p, v1, v2, v3) INST(p, SUB, v1, v2, v3)
#define LDI(p, v1, v2) INST(p, LDI, v1, v2, 0)

#define LDM(p, v1, v2) INST(p, LDM, v1, v2, 0)
#define STR(p, v1, v2) INST(p, STR, v1, v2, 0)

#define LDA(p, v1, v2) INST(p, LDA, v1, v2, 0)
#define STA(p, v1, v2) INST(p, STA, v1, v2, 0)

#define PUSH(p, v1) INST(p, PUSH, v1, 0, 0)
#define POP(p, v1) INST(p, POP, v1, 0, 0)

#define INC(p, v1) INST(p, INC, v1, 0, 0)
#define DEC(p, v1) INST(p, DEC, v1, 0, 0)

#define IN(p, v1, v2, v3) INST(p, IN, v1, v2, v3)
#define OUT(p, v1, v2, v3) INST(p, OUT, v1, v2, v3)


/* Exception */

#define ZVM_EXCEPTION_HANDLER_FUNCTION_NAME(name)   zxpt_ ## name
#define ZVM_EXCEPTION_HANDLER_FUNCTION_PARAM \
    zvm_vm_t* vm, uint8_t code

#define ZVM_EXCEPTION_HANDLER_FUNCTION_ARGS \
    vm, code

#define ZVM_EXCEPTION_HANDLER_FUNCTION(name)  \
    bool ZVM_EXCEPTION_HANDLER_FUNCTION_NAME(name) (ZVM_EXCEPTION_HANDLER_FUNCTION_PARAM)

#define ZVM_EXCEPTION_HANDLER_CALL(name) zxpt_ ## name(ZVM_EXCEPTION_HANDLER_FUNCTION_ARGS)


#define ZVM_EXCEPT_TYPE_FETCH   0
#define ZVM_EXCEPT_TYPE_DECODE  1
#define ZVM_EXCEPT_TYPE_EXECUTE 2
#define ZVM_EXCEPT_TYPE_LOAD    3
#define ZVM_EXCEPT_TYPE_VM      4

#define ZVM_EXCEPT_CODE_FETCH                0
#define ZVM_EXCEPT_CODE_DECODE               1
#define ZVM_EXCEPT_CODE_EXECUTE              2
#define ZVM_EXCEPT_CODE_INVALID_REGISTER     3
#define ZVM_EXCEPT_CODE_BAD_INSTRUCTION      4
#define ZVM_EXCEPT_CODE_BAD_MEMORY_ADDRESS   5
#define ZVM_EXCEPT_CODE_STACK_OVERFLOW       6
#define ZVM_EXCEPT_CODE_STACK_UNDERFLOW      7
#define ZVM_EXCEPT_CODE_IO_DEVICE_NOT_FOUND  8
#define ZVM_EXCEPT_CODE_LOAD_PROGRAM         9
#define ZVM_EXCEPT_CODE_VM_INIT              10


ZVM_EXCEPTION_HANDLER_FUNCTION(default_exception_handler);

#define zvm_raise(vm, t, c) \
    (vm)->has_exception  = true;    \
    (vm)->exception_metadata.type = ZVM_EXCEPT_TYPE_ ## t;     \
    (vm)->exception_metadata.code = ZVM_EXCEPT_CODE_ ## c;

#define zvm_exception_get_type(vm)  (vm)->exception_metadata.type
#define zvm_exception_get_code(vm)  (vm)->exception_metadata.code

#define zvm_exception_reset(vm)      \
    (vm)->has_exception  = false;    \
    (vm)->exception_metadata.type = -1;       \
    (vm)->exception_metadata.code = -1;

#define ZVM_EXCEPTION_HANDLERS_COUNT (sizeof(exception_handlers)) / sizeof(zvm_exception_entry_t)
#define zvm_exception_is_handler(code)  ((code) < ZVM_EXCEPTION_HANDLERS_COUNT)

#define ZVM_EXCEPTION_HANDLER_ENTRY(type, code, name, message) \
    {ZVM_EXCEPT_TYPE_ ## type, ZVM_EXCEPT_CODE_ ## code,  ZVM_EXCEPTION_HANDLER_FUNCTION_NAME(name)   , message},

static const zvm_exception_entry_t exception_handlers [] = {
    ZVM_EXCEPTION_HANDLER_ENTRY(FETCH,   FETCH,   default_exception_handler,   "fetch instruction error")
    ZVM_EXCEPTION_HANDLER_ENTRY(DECODE,  DECODE,  default_exception_handler,  "decode instruction error")
    ZVM_EXCEPTION_HANDLER_ENTRY(EXECUTE, EXECUTE, default_exception_handler, "execution: bad instruction error")
    ZVM_EXCEPTION_HANDLER_ENTRY(EXECUTE, INVALID_REGISTER, default_exception_handler, "invalid register")
    ZVM_EXCEPTION_HANDLER_ENTRY(DECODE,  BAD_INSTRUCTION,  default_exception_handler,  "decoding: bad instruction error")
    ZVM_EXCEPTION_HANDLER_ENTRY(EXECUTE,  BAD_MEMORY_ADDRESS,  default_exception_handler,  "bad memory address error")
    ZVM_EXCEPTION_HANDLER_ENTRY(EXECUTE,  STACK_OVERFLOW,  default_exception_handler,  "stack overflow error")
    ZVM_EXCEPTION_HANDLER_ENTRY(EXECUTE,  STACK_UNDERFLOW,  default_exception_handler,  "stack underflow error")
    ZVM_EXCEPTION_HANDLER_ENTRY(EXECUTE,  IO_DEVICE_NOT_FOUND,  default_exception_handler,  "io device not found")
    ZVM_EXCEPTION_HANDLER_ENTRY(LOAD,  LOAD_PROGRAM,  default_exception_handler,  "unable to load the program")
    ZVM_EXCEPTION_HANDLER_ENTRY(VM,  VM_INIT,  default_exception_handler,  "VM initialization error")
};

/* IO */

#define ZVM_IO_DEVICE_STATUS_READY 0
#define ZVM_IO_DEVICE_STATUS_BUSY  1
#define ZVM_IO_DEVICE_STATUS_OFF   2

#define ZVM_IO_DEVICE_TYPE_IN    0
#define ZVM_IO_DEVICE_TYPE_OUT   1
#define ZVM_IO_DEVICE_TYPE_INOUT 2

#define ZVM_IO_PORT_KEYBOARD 0
#define ZVM_IO_PORT_SCREEN   1

#define ZVM_IO_SERVICE_SCREEN_WRITE_CHARACTER  0
#define ZVM_IO_SERVICE_KEYBOARD_READ_CHARACTER 0

bool zfnio_keyboard(zvm_vm_t *vm,  uint8_t port, uint8_t operation,  uint8_t argc);
bool zfnio_screen(zvm_vm_t *vm, uint8_t port, uint8_t operation,  uint8_t argc);


static zvm_io_device_t keyboard = 
 {0, "keyboard", ZVM_IO_DEVICE_STATUS_READY, ZVM_IO_DEVICE_TYPE_IN, {0, 0, 0, 0}, {0, 0, 0, 0}, zfnio_keyboard};

static zvm_io_device_t screen = 
 {1, "screen", ZVM_IO_DEVICE_STATUS_READY, ZVM_IO_DEVICE_TYPE_OUT, {0, 0, 0, 0}, {0, 0, 0, 0}, zfnio_screen};

#endif
