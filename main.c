#include "zvm.h"

int main(void){
    zvm_vm_t vm;
    uint8_t program_size = 24;

    uint8_t program [] = {
        2, R0, 'Z', 0,
        7, R0, 0, 0,
        2, R0, ZVM_IO_PORT_SCREEN, 0,
        2, R1, ZVM_IO_SERVICE_SCREEN_WRITE_CHARACTER, 0,
        2, R2, 1, 0,
        12, R0, R1, R2,
    };
    
    return zvm_main(&vm, program, program_size);
}
