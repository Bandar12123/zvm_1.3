#ifndef __ZVM_TYPES_H__
#define __ZVM_TYPES_H__

#define ZVM_RX_REGISTERS_COUNT 4
#define ZVM_IO_MAX_DEVICES 4

#define ZVM_IO_INPUT_REGISTERS_COUNT 4
#define ZVM_IO_OUTPUT_REGISTERS_COUNT 4

#define ZVM_PROGRAM_DEFAULT_DATA_SEGMENT_SIZE   64
#define ZVM_PROGRAM_DEFAULT_CODE_SEGMENT_SIZE   128
#define ZVM_PROGRAM_DEFAULT_STACK_SEGMENT_SIZE  64

#define ZVM_INST_MAX_OPERANDS  3

#include <stdint.h>
#include <stdbool.h>

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

struct zvm_cpu_t{
    uint8_t IP;
    zvm_instruction_t* IR;
    uint8_t R[ZVM_RX_REGISTERS_COUNT];
    uint8_t FLAGS;
    int8_t SP;
};

struct zvm_exception_metadata_t{
    int8_t type;
    int8_t code;
};

struct zvm_io_device_t{
    // TODO:
    uint8_t id;
    char *name;
    uint8_t status;
    uint8_t type;
    uint8_t I[ZVM_IO_INPUT_REGISTERS_COUNT];
    uint8_t O[ZVM_IO_OUTPUT_REGISTERS_COUNT];
    zfn_io_handler_t handler;
};

typedef struct zvm_operand_metadata_t {
    uint8_t type;
}zvm_operand_metadata_t;

typedef struct zvm_operand_t{
    zvm_operand_metadata_t* metadata;
    uint8_t value;
}zvm_operand_t;

struct zvm_instruction_handler_t{
    int8_t type;
    zfn_instruction_handler_t handler;
};

typedef struct zvm_instruction_metadata_t{
    char *mnemonic;
    uint8_t opcode;
    uint8_t operands_count;
    zvm_operand_metadata_t operands[ZVM_INST_MAX_OPERANDS];
    zvm_instruction_handler_t handler;
}zvm_instruction_metadata_t;

struct zvm_instruction_t {
    zvm_instruction_metadata_t *metadata;
    zvm_operand_t operands[ZVM_INST_MAX_OPERANDS];
};

struct zvm_exception_entry_t{
    zvm_exception_metadata_t metadata;
    zfn_exception_handler_t handler;
    char *message;
};

struct zvm_program_t{
    uint8_t data[ZVM_PROGRAM_DEFAULT_DATA_SEGMENT_SIZE];
    uint8_t stack[ZVM_PROGRAM_DEFAULT_STACK_SEGMENT_SIZE];
    zvm_instruction_t instructions[ZVM_PROGRAM_DEFAULT_CODE_SEGMENT_SIZE];

    int8_t data_count;
    int8_t instructions_count;
    int8_t stack_counts;
};

struct zvm_vm_t{
    bool has_exception;
    zvm_exception_metadata_t exception_metadata;
    zvm_cpu_t cpu;
    zvm_program_t program;
    zvm_io_device_t* io_devices[ZVM_IO_MAX_DEVICES];
};
#endif
