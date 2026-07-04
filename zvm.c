#include "zvm.h"
#include <stdlib.h>

bool zvm_init(zvm_vm_t *vm){
    vm->cpu.IP = 0;
    vm->cpu.IR = NULL;

    vm->cpu.R[0] = 0;
    vm->cpu.R[1] = 0;
    vm->cpu.R[2] = 0;
    vm->cpu.R[3] = 0;

    vm->cpu.FLAGS  = 0;
    vm->cpu.SP = -1;

    vm->program.data_blob = blb_blob_create(ZVM_PROGRAM_DEFAULT_DATA_SEGMENT_SIZE, 1);
    vm->program.stack_blob = blb_blob_create(ZVM_PROGRAM_DEFAULT_STACK_SEGMENT_SIZE, 1);
    vm->program.code_blob = NULL; // سيتم إنشاؤه ديناميكياً عند تحميل البرنامج

    vm->cpu.IR = malloc(sizeof(zvm_instruction_t));

    if(!vm->program.data_blob || !vm->program.stack_blob || !vm->cpu.IR){
        return false;
    }

    return zvm_init_io(vm);
}

void zvm_release(zvm_vm_t *vm){
    if(vm->program.data_blob)    { blb_blob_delete(vm->program.data_blob);    vm->program.data_blob = NULL; }
    if(vm->program.stack_blob)   { blb_blob_delete(vm->program.stack_blob);   vm->program.stack_blob = NULL; }
    if(vm->program.code_blob)    { blb_blob_delete(vm->program.code_blob);    vm->program.code_blob = NULL; }
    if(vm->cpu.IR)               { free(vm->cpu.IR);                          vm->cpu.IR = NULL; }
}

static bool zvm_init_io(zvm_vm_t *vm){
    vm->io_devices[0] = &keyboard;
    vm->io_devices[1] = &screen;
    vm->io_devices[2] = NULL;
    vm->io_devices[3] = NULL;
    return true;
}

static bool zvm_fetch(zvm_vm_t *vm){
    uint32_t offset = vm->cpu.IP * ZVM_INSTRUCTION_SIZE;
    uint8_t opcode = 0, left = 0, right = 0, output = 0;

    if (!blb_block_get(vm->program.code_blob->block, offset, &opcode) ||
        !blb_block_get(vm->program.code_blob->block, offset + 1, &left) ||
        !blb_block_get(vm->program.code_blob->block, offset + 2, &right) ||
        !blb_block_get(vm->program.code_blob->block, offset + 3, &output)) {
        return false;
    }

    vm->cpu.IR->metadata = (zvm_instruction_metadata_t*)&instruction_handlers[opcode];
    vm->cpu.IR->operands[0].value = left;
    vm->cpu.IR->operands[1].value = right;
    vm->cpu.IR->operands[2].value = output;

    vm->cpu.IP++;
    return true;
}

static bool zvm_decode(zvm_vm_t* vm){
    uint8_t val0 = vm->cpu.IR->operands[0].value;
    uint8_t val1 = vm->cpu.IR->operands[1].value;
    uint8_t val2 = vm->cpu.IR->operands[2].value;

    switch(vm->cpu.IR->metadata->handler.type){
        case ZVM_INST_HANDLER_TYPE_RRR:
            if(val0 >= ZVM_RX_REGISTERS_COUNT || val1 >= ZVM_RX_REGISTERS_COUNT || val2 >= ZVM_RX_REGISTERS_COUNT){
                zvm_raise(vm, DECODE, INVALID_REGISTER)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_RI:
            if(val0 >= ZVM_RX_REGISTERS_COUNT || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_RM:
            if(val0 >= ZVM_RX_REGISTERS_COUNT || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_RR:
            if(val0 >= ZVM_RX_REGISTERS_COUNT || val1 >= ZVM_RX_REGISTERS_COUNT || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_R:
            if(val0 >= ZVM_RX_REGISTERS_COUNT || val1 != 0 || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
    }
    return true;
}

static bool zvm_execute(zvm_vm_t* vm){
    zfn_instruction_handler_t handler = (zfn_instruction_handler_t)vm->cpu.IR->metadata->handler.handler;
    return handler(vm, vm->cpu.IR->operands[0].value,
                       vm->cpu.IR->operands[1].value,
                       vm->cpu.IR->operands[2].value);
}

bool zvm_except(zvm_vm_t* vm){
    if(vm->has_exception){
        int8_t code = zvm_exception_get_code(vm);
        if(zvm_exception_is_handler(code)){
            return exception_handlers[code].handler(vm, code);
        }
        return false;
    }
    return true;
}

int zvm_run(zvm_vm_t *vm){    
    while(zvm_has_next_instruction(vm)){
        if(!zvm_fetch(vm)){
            zvm_raise(vm, FETCH, FETCH)
            goto zvm_catch;
        }
        if(!zvm_decode(vm)){
            zvm_raise(vm, DECODE, DECODE)
            goto zvm_catch;
        }
        if(!zvm_execute(vm)){
            goto zvm_catch;
        }
        continue;
    zvm_catch:
        if(!zvm_except(vm)){
            break;
        }else{
            zvm_exception_reset(vm)
        }
    }

    if(vm->has_exception){
        fprintf(stderr, "Exception(%d): %s\n", vm->exception_metadata.code,
                    exception_handlers[vm->exception_metadata.code].message);
    }
    return 0;
}

bool zvm_load_program(zvm_vm_t* vm, const uint8_t *program, uint32_t program_size){
    if(vm == NULL || program == NULL || program_size == 0){
        return false;
    }

    if((program_size % ZVM_INSTRUCTION_SIZE) != 0){
        zvm_raise(vm, LOAD, LOAD_PROGRAM)
        return false;
    }

    if(vm->program.code_blob) {
        blb_blob_delete(vm->program.code_blob);
    }
    vm->program.code_blob = blb_blob_create(program_size, 1);
    if(!vm->program.code_blob){
        return false;
    }

    for(uint32_t i = 0; i < program_size; i++){
        blb_block_put(vm->program.code_blob->block, i, program[i]);
    }

    return true;
}

int zvm_main(zvm_vm_t *vm, uint8_t *program, uint32_t program_size){
    int result = 0;
    if(!zvm_init(vm)){
        zvm_raise(vm, VM, VM_INIT)
        return false;
    }

    if(!zvm_load_program(vm, program, program_size)){
        zvm_except(vm);
        zvm_release(vm);
        return -1;
    }

    result = zvm_run(vm);
    
    return result;
}