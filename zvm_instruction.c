#include "zvm.h"

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(add)
    vm->cpu.R[output] = vm->cpu.R[left] + vm->cpu.R[right];
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(sub)
    vm->cpu.R[output] = vm->cpu.R[left] - vm->cpu.R[right];
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(ldi)
    vm->cpu.R[left] = right;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(str)
    if (blb_blob_jump(vm->program.data_blob, right)) {
        blb_blob_put(vm->program.data_blob, vm->cpu.R[left]);
    } else {
        zvm_raise(vm, EXECUTE, BAD_MEMORY_ADDRESS);
        return false;
    }
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(ldm)
    uint8_t val = 0;
    if (blb_blob_jump(vm->program.data_blob, right) && blb_blob_get(vm->program.data_blob, &val)) {
        vm->cpu.R[left] = val;
    } else {
        zvm_raise(vm, EXECUTE, BAD_MEMORY_ADDRESS);
        return false;
    }
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(lda)
    uint8_t address = vm->cpu.R[left];
    uint8_t val = 0;
    if (blb_blob_jump(vm->program.data_blob, address) && blb_blob_get(vm->program.data_blob, &val)) {
        vm->cpu.R[right] = val;
    } else {
        zvm_raise(vm, EXECUTE, BAD_MEMORY_ADDRESS);
        return false;
    }
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(sta)
    uint8_t address = vm->cpu.R[left];
    if (blb_blob_jump(vm->program.data_blob, address)) {
        blb_blob_put(vm->program.data_blob, vm->cpu.R[right]);
    } else {
        zvm_raise(vm, EXECUTE, BAD_MEMORY_ADDRESS);
        return false;
    }
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(push)
    uint32_t stack_size = vm->program.stack_blob->block->size;
    if((uint32_t)(vm->cpu.SP + 1) >= stack_size){
        zvm_raise(vm, EXECUTE, STACK_OVERFLOW)
        return false;
    }

    vm->cpu.SP++;
    uint8_t value = vm->cpu.R[left];
    uint32_t addr = (stack_size - 1) - vm->cpu.SP;
    
    if (blb_blob_jump(vm->program.stack_blob, addr)) {
        blb_blob_put(vm->program.stack_blob, value);
    }
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(pop)
    if(vm->cpu.SP < 0){
        zvm_raise(vm, EXECUTE, STACK_UNDERFLOW)
        return false;
    }

    uint8_t val = 0;
    uint32_t stack_size = vm->program.stack_blob->block->size;
    uint32_t addr = (stack_size - 1) - vm->cpu.SP;
    
    if (blb_blob_jump(vm->program.stack_blob, addr) && blb_blob_get(vm->program.stack_blob, &val)) {
        vm->cpu.SP--;
        vm->cpu.R[left] = val;
        printf("POP = %u\n", val);
    } else {
        return false;
    }
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(inc)
    vm->cpu.R[left]++;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(dec)
    vm->cpu.R[left]--;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(in)
    uint8_t port = vm->cpu.R[left];
    uint8_t function = vm->cpu.R[right];
    uint8_t argc = vm->cpu.R[output];

    assert(argc <= 4);

    if(port >= ZVM_IO_MAX_DEVICES){
        zvm_raise(vm, EXECUTE, BAD_INSTRUCTION);
        return false;
    }

    if(vm->io_devices[port] == NULL){
        zvm_raise(vm, EXECUTE, IO_DEVICE_NOT_FOUND);
        return false;
    }

    assert(vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_IN
     || vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_INOUT);
     
    if(argc > 0){
        uint32_t stack_size = vm->program.stack_blob->block->size;
        for(uint8_t i = 0; i < argc; i++){
            if(vm->cpu.SP < 0) return false;
            uint8_t pop_val = 0;
            uint32_t addr = (stack_size - 1) - vm->cpu.SP;
            
            if (blb_blob_jump(vm->program.stack_blob, addr) && blb_blob_get(vm->program.stack_blob, &pop_val)) {
                vm->cpu.SP--;
                vm->cpu.R[0] = pop_val;
                vm->io_devices[port]->I[i] = vm->cpu.R[0];
            }
        }
    }
    return vm->io_devices[port]->handler(vm, port, function, argc);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(out)
    uint8_t port = vm->cpu.R[left];
    uint8_t function = vm->cpu.R[right];
    uint8_t argc = vm->cpu.R[output];

    assert(argc <= 4);

    if(port >= ZVM_IO_MAX_DEVICES){
        zvm_raise(vm, EXECUTE, BAD_INSTRUCTION);
        return false;
    }

    if(vm->io_devices[port] == NULL){
        zvm_raise(vm, EXECUTE, IO_DEVICE_NOT_FOUND);
        return false;
    }

    assert(vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_OUT
     || vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_INOUT);

    if(argc > 0){
        uint32_t stack_size = vm->program.stack_blob->block->size;
        for(uint8_t i = 0; i < argc; i++){
            if(vm->cpu.SP < 0) return false;
            uint8_t pop_val = 0;
            uint32_t addr = (stack_size - 1) - vm->cpu.SP;
            
            if (blb_blob_jump(vm->program.stack_blob, addr) && blb_blob_get(vm->program.stack_blob, &pop_val)) {
                vm->cpu.SP--;
                vm->cpu.R[0] = pop_val;
                printf("R0 = %u\n", vm->cpu.R[0]);
                vm->io_devices[port]->I[i] = vm->cpu.R[0];
            }
        }
    }
    return vm->io_devices[port]->handler(vm, port, function, argc);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END