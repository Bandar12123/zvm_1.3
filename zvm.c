#include "zvm.h"
#include <stdlib.h>

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

    // الأسماء الصحيحة المتوافقة مع zvm_types.h
    vm->program.data_blob = blb_blob_create(ZVM_PROGRAM_DEFAULT_DATA_SEGMENT_SIZE, 1);
    vm->program.stack_blob = blb_blob_create(ZVM_PROGRAM_DEFAULT_STACK_SEGMENT_SIZE, 1);
    
    // تخصيص مصفوفة الـ instructions ديناميكياً كمصفوفة Structs (باستخدام malloc وليس blb_blob_create)
    vm->program.instructions = malloc(sizeof(zvm_instruction_t) * ZVM_PROGRAM_DEFAULT_CODE_SEGMENT_SIZE);

    if(!vm->program.data_blob || !vm->program.stack_blob || !vm->program.instructions){
        return false;
    }

    return zvm_init_io(vm);
}

void zvm_release(zvm_vm_t *vm){
    vm->cpu.IP = 0;
    vm->cpu.IR = NULL;

    vm->cpu.R[0] = 0;
    vm->cpu.R[1] = 0;
    vm->cpu.R[2] = 0;
    vm->cpu.R[3] = 0;

    vm->cpu.FLAGS  = 0;
    vm->cpu.SP = -1;

    // تنظيف الذاكرة بالأسماء الجديدة وطرق التحرير الصحيحة لكل نوع
    if(vm->program.data_blob)    { blb_blob_delete(vm->program.data_blob);    vm->program.data_blob = NULL; }
    if(vm->program.stack_blob)   { blb_blob_delete(vm->program.stack_blob);   vm->program.stack_blob = NULL; }
    if(vm->program.instructions) { free(vm->program.instructions);            vm->program.instructions = NULL; }
}

static bool zvm_init_io(zvm_vm_t *vm){
    vm->io_devices[0] = &keyboard;
    vm->io_devices[1] = &screen;
    vm->io_devices[2] = NULL;
    vm->io_devices[3] = NULL;
    return true;
}

void zvm_init_program(zvm_vm_t* vm){
    // متبقية فارغة للمحافظة على التوافق إذا تم استدعاؤها
}

static bool zvm_fetch(zvm_vm_t *vm){
    vm->cpu.IR = zvm_program_instruction(vm, vm->cpu.IP);
    vm->cpu.IP++;
    return true;
}

static bool zvm_decode(zvm_vm_t* vm){
    uint8_t val0 = vm->cpu.IR->operands[0].value;
    uint8_t val1 = vm->cpu.IR->operands[1].value;
    uint8_t val2 = vm->cpu.IR->operands[2].value;

    switch(vm->cpu.IR->metadata->handler.type){
        case ZVM_INST_HANDLER_TYPE_RRR:
            if(val0 >= ZVM_RX_REGISTERS_COUNT
               || val1 >= ZVM_RX_REGISTERS_COUNT
               || val2 >= ZVM_RX_REGISTERS_COUNT){
                zvm_raise(vm, DECODE, INVALID_REGISTER)
                return false;
            }
        break;
         
        case ZVM_INST_HANDLER_TYPE_RI:
            if(val0 >= ZVM_RX_REGISTERS_COUNT
               || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_RM:
            if(val0 >= ZVM_RX_REGISTERS_COUNT
               || val1 >= ZVM_PROGRAM_DEFAULT_DATA_SEGMENT_SIZE
               || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_RR:
            if(val0 >= ZVM_RX_REGISTERS_COUNT
               || val1 >= ZVM_RX_REGISTERS_COUNT
               || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
        case ZVM_INST_HANDLER_TYPE_R:
            if(val0 >= ZVM_RX_REGISTERS_COUNT
               || val1 != 0
               || val2 != 0){
                zvm_raise(vm, DECODE, BAD_INSTRUCTION)
                return false;
            }
        break;
    }
    return true;
}

static bool zvm_execute(zvm_vm_t* vm){
    int8_t instruction_index = vm->cpu.IR->metadata->handler.type;
    if(instruction_index >= ZVM_INSTRUCTION_HANDLERS_COUNT){
        zvm_raise(vm, EXECUTE, EXECUTE);
        return false;
    }
    
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
        }else{
            fprintf(stderr, "Invalid exception handler code\n");
            return false;
        }
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
    if(vm == NULL || program == NULL){
        return false;
    }

    if(program_size == 0){
        return true;
    }

    if(((program_size % ZVM_INSTRUCTION_SIZE) != 0)){
        zvm_raise(vm, LOAD, LOAD_PROGRAM)
        return false;
    }

    uint8_t instructions_count = program_size / ZVM_INSTRUCTION_SIZE;
    uint8_t opcode = 0, left_operand = 0, right_operand = 0, output_operand = 0;

    vm->program.instructions_count = instructions_count;
    vm->program.data_count = 0;
    vm->program.stack_counts = 0;

    // إعادة تخصيص حجم مصفوفة التعليمات بناءً على حجم البرنامج الممرر فعلياً
    free(vm->program.instructions);
    vm->program.instructions = malloc(sizeof(zvm_instruction_t) * instructions_count);
    if (!vm->program.instructions) {
        zvm_raise(vm, VM, VM_INIT);
        return false;
    }

    for(int i = 0; i < program_size; i+= ZVM_INSTRUCTION_SIZE){
        opcode         = program[0 + i];
        left_operand   = program[1 + i];
        right_operand  = program[2 + i];
        output_operand = program[3 + i];

        uint8_t idx = (i / ZVM_INSTRUCTION_SIZE);
        zvm_instruction_t *inst = zvm_program_instruction(vm, idx);

        inst->metadata = (zvm_instruction_metadata_t*)&instruction_handlers[opcode];
        inst->operands[0].metadata = (zvm_operand_metadata_t*)&instruction_handlers[opcode].operands[0];
        inst->operands[1].metadata = (zvm_operand_metadata_t*)&instruction_handlers[opcode].operands[1];
        inst->operands[2].metadata = (zvm_operand_metadata_t*)&instruction_handlers[opcode].operands[2];

        inst->operands[0].value = left_operand;
        inst->operands[1].value = right_operand;
        inst->operands[2].value = output_operand;
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
    }

    result = zvm_run(vm);
    zvm_release(vm);
    return result;
}