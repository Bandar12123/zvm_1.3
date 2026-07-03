#include "zvm.h"

bool zfnio_keyboard(zvm_vm_t *vm,  uint8_t port, uint8_t operation,  uint8_t argc){
    char character = 0;
    scanf("%c", &character);
    vm->io_devices[port]->O[0] = (uint8_t)character;
    vm->cpu.R[0] = vm->io_devices[port]->O[0];
    return true;
}

bool zfnio_screen(zvm_vm_t *vm, uint8_t port, uint8_t operation,  uint8_t argc){
    uint8_t character = vm->io_devices[port]->I[0];
    printf("%c", character);
    return true;
}
