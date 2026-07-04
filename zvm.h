#ifndef __ZVM_H__
#define __ZVM_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "zvm_types.h"
#include "zvm_api.h"
#include "blobify.h"


#ifdef zvm_has_next_instruction
  #undef zvm_has_next_instruction
#endif
#define zvm_has_next_instruction(vm) \
  (((uint32_t)(vm)->cpu.IP * ZVM_INSTRUCTION_SIZE) < (vm)->program.code_blob->block->size)

/* ZVM API */
bool zvm_init(zvm_vm_t *vm); 
void zvm_release(zvm_vm_t *vm);
static bool zvm_init_io(zvm_vm_t *vm);

bool zvm_load_program(zvm_vm_t* vm, const uint8_t *program, uint32_t program_size);

static bool zvm_fetch(zvm_vm_t *vm);
static bool zvm_decode(zvm_vm_t *vm);
static bool zvm_execute(zvm_vm_t *vm);

bool zvm_except(zvm_vm_t *vm);
int zvm_run(zvm_vm_t *vm);
int zvm_main(zvm_vm_t *vm, uint8_t *program, uint32_t program_size);


#include "zvm.c"
#include "zvm_instruction.c"
#include "zvm_exception.c"
#include "zvm_io.c"     
#include "blobify.c"


#endif