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
    blb_blob_jump(vm->program.data, right);
    blb_blob_put(vm->program.data, vm->cpu.R[left]);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(ldm)
    uint8_t val = 0;
    blb_blob_jump(vm->program.data, right);
    blb_blob_get(vm->program.data, &val);
    vm->cpu.R[left] = val;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(lda)
    uint8_t address = vm->cpu.R[left];
    uint8_t val = 0;
    if(!blb_blob_jump(vm->program.data, address)){
        zvm_raise(vm, EXECUTE, BAD_MEMORY_ADDRESS) return false;
    }
    blb_blob_get(vm->program.data, &val);
    vm->cpu.R[right] = val;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(sta)
    uint8_t address = vm->cpu.R[left];
    if(!blb_blob_jump(vm->program.data, address)){
        zvm_raise(vm, EXECUTE, BAD_MEMORY_ADDRESS) return false;
    }
    blb_blob_put(vm->program.data, vm->cpu.R[right]);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(push)
    int32_t new_sp = vm->cpu.SP + 1;
    int32_t stack_size = (int32_t)vm->program.stack->block->size;
    if(new_sp >= stack_size){
        zvm_raise(vm, EXECUTE, STACK_OVERFLOW) return false;
    }
    vm->cpu.SP = new_sp;
    int32_t addr = stack_size - 1 - vm->cpu.SP;
    blb_blob_jump(vm->program.stack, addr);
    blb_blob_put(vm->program.stack, vm->cpu.R[left]);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(pop)
    if(vm->cpu.SP < 0){
        zvm_raise(vm, EXECUTE, STACK_UNDERFLOW) return false;
    }
    int32_t stack_size = (int32_t)vm->program.stack->block->size;
    int32_t addr = stack_size - 1 - vm->cpu.SP;
    uint8_t val = 0;
    blb_blob_jump(vm->program.stack, addr);
    blb_blob_get(vm->program.stack, &val);
    vm->cpu.SP--;
    vm->cpu.R[left] = val;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(inc)
    vm->cpu.R[left]++;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(dec)
    vm->cpu.R[left]--;
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(in)
    uint8_t port     = vm->cpu.R[left];
    uint8_t function = vm->cpu.R[right];
    uint8_t argc     = vm->cpu.R[output];
    assert(argc <= 4);
    if(port >= ZVM_IO_MAX_DEVICES){ zvm_raise(vm, EXECUTE, BAD_INSTRUCTION) return false; }
    if(!vm->io_devices[port])     { zvm_raise(vm, EXECUTE, IO_DEVICE_NOT_FOUND) return false; }
    assert(vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_IN ||
           vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_INOUT);
    return vm->io_devices[port]->handler(vm, port, function, argc);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END

ZVM_INSTRUCTION_HANDLER_FUNCTION_BEGIN(out)
    uint8_t port     = vm->cpu.R[left];
    uint8_t function = vm->cpu.R[right];
    uint8_t argc     = vm->cpu.R[output];
    assert(argc <= 4);
    if(port >= ZVM_IO_MAX_DEVICES){ zvm_raise(vm, EXECUTE, BAD_INSTRUCTION) return false; }
    if(!vm->io_devices[port])     { zvm_raise(vm, EXECUTE, IO_DEVICE_NOT_FOUND) return false; }
    assert(vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_OUT ||
           vm->io_devices[port]->type == ZVM_IO_DEVICE_TYPE_INOUT);
    if(argc > 0){
        /* pop argc values from stack into I[] */
        int32_t stack_size = (int32_t)vm->program.stack->block->size;
        for(uint8_t i = 0; i < argc; i++){
            if(vm->cpu.SP < 0){ zvm_raise(vm, EXECUTE, STACK_UNDERFLOW) return false; }
            int32_t addr = stack_size - 1 - vm->cpu.SP;
            blb_blob_jump(vm->program.stack, addr);
            blb_blob_get(vm->program.stack, &vm->io_devices[port]->I[i]);
            vm->cpu.SP--;
        }
    }
    return vm->io_devices[port]->handler(vm, port, function, argc);
ZVM_INSTRUCTION_HANDLER_FUNCTION_END