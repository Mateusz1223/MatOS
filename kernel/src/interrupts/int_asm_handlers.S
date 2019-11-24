.intel_syntax noprefix
.section .text
.globl _int_0_handler
.globl _int_1_handler
.globl _int_2_handler
.globl _int_3_handler
.globl _int_4_handler
.globl _int_5_handler
.globl _int_6_handler
.globl _int_7_handler
.globl _int_8_handler
.globl _int_9_handler
.globl _int_10_handler
.globl _int_11_handler
.globl _int_12_handler
.globl _int_13_handler
.globl _int_14_handler
.globl _int_15_handler
.globl _int_16_handler
.globl _int_17_handler
.globl _int_18_handler
.globl _int_19_handler
.globl _int_20_handler
.globl _int_21_handler
.globl _int_22_handler
.globl _int_23_handler
.globl _int_24_handler
.globl _int_25_handler
.globl _int_26_handler
.globl _int_27_handler
.globl _int_28_handler
.globl _int_29_handler
.globl _int_30_handler
.globl _int_31_handler

/*.globl _irq_0_handler
.globl _irq_1_handler
.globl _irq_2_handler
.globl _irq_3_handler
.globl _irq_4_handler
.globl _irq_5_handler
.globl _irq_6_handler
.globl _irq_7_handler
.globl _irq_8_handler
.globl _irq_9_handler
.globl _irq_10_handler
.globl _irq_11_handler
.globl _irq_12_handler
.globl _irq_13_handler
.globl _irq_14_handler
.globl _irq_15_handler*/

.globl _irq_handler

.section .text
.def _int_0_handler ; .scl 2 ; .type 32 ; .endef
.def _int_1_handler ; .scl 2 ; .type 32 ; .endef
.def _int_2_handler ; .scl 2 ; .type 32 ; .endef
.def _int_3_handler ; .scl 2 ; .type 32 ; .endef
.def _int_4_handler ; .scl 2 ; .type 32 ; .endef
.def _int_5_handler ; .scl 2 ; .type 32 ; .endef
.def _int_6_handler ; .scl 2 ; .type 32 ; .endef
.def _int_7_handler ; .scl 2 ; .type 32 ; .endef
.def _int_8_handler ; .scl 2 ; .type 32 ; .endef
.def _int_9_handler ; .scl 2 ; .type 32 ; .endef
.def _int_10_handler ; .scl 2 ; .type 32 ; .endef
.def _int_11_handler ; .scl 2 ; .type 32 ; .endef
.def _int_12_handler ; .scl 2 ; .type 32 ; .endef
.def _int_13_handler ; .scl 2 ; .type 32 ; .endef
.def _int_14_handler ; .scl 2 ; .type 32 ; .endef
.def _int_15_handler ; .scl 2 ; .type 32 ; .endef
.def _int_16_handler ; .scl 2 ; .type 32 ; .endef
.def _int_17_handler ; .scl 2 ; .type 32 ; .endef
.def _int_18_handler ; .scl 2 ; .type 32 ; .endef
.def _int_19_handler ; .scl 2 ; .type 32 ; .endef
.def _int_20_handler ; .scl 2 ; .type 32 ; .endef
.def _int_21_handler ; .scl 2 ; .type 32 ; .endef
.def _int_22_handler ; .scl 2 ; .type 32 ; .endef
.def _int_23_handler ; .scl 2 ; .type 32 ; .endef
.def _int_24_handler ; .scl 2 ; .type 32 ; .endef
.def _int_25_handler ; .scl 2 ; .type 32 ; .endef
.def _int_26_handler ; .scl 2 ; .type 32 ; .endef
.def _int_27_handler ; .scl 2 ; .type 32 ; .endef
.def _int_28_handler ; .scl 2 ; .type 32 ; .endef
.def _int_29_handler ; .scl 2 ; .type 32 ; .endef
.def _int_30_handler ; .scl 2 ; .type 32 ; .endef
.def _int_31_handler ; .scl 2 ; .type 32 ; .endef

/*.def _irq_0_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_1_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_2_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_3_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_4_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_5_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_6_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_7_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_8_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_9_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_10_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_11_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_12_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_13_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_14_handler ; .scl 2 ; .type 32 ; .endef
.def _irq_15_handler ; .scl 2 ; .type 32 ; .endef*/

.def _irq_handler ; .scl 2 ; .type 32 ; .endef

_int_0_handler:
  cli
  pushad
  push esp
  push 0
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_1_handler:
  cli
  pushad
  push esp
  push 1
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_2_handler:
  cli
  pushad
  push esp
  push 2
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_3_handler:
  cli
  pushad
  push esp
  push 3
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_4_handler:
  cli
  pushad
  push esp
  push 4
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_5_handler:
  cli
  pushad
  push esp
  push 5
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_6_handler:
  cli
  pushad
  push esp
  push 6
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_7_handler:
  cli
  pushad
  push esp
  push 7
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_8_handler:
  cli
  pushad
  push esp
  push 8
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_9_handler:
  cli
  pushad
  push esp
  push 9
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_10_handler:
  cli
  pushad
  push esp
  push 10
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_11_handler:
  cli
  pushad
  push esp
  push 11
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_12_handler:
  cli
  pushad
  push esp
  push 12
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_13_handler:
  cli
  pushad
  push esp
  push 13
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_14_handler:
  cli
  pushad
  push esp
  push 14
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_15_handler:
  cli
  pushad
  push esp
  push 15
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_16_handler:
  cli
  pushad
  push esp
  push 16
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_17_handler:
  cli
  pushad
  push esp
  push 17
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_18_handler:
  cli
  pushad
  push esp
  push 18
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_19_handler:
  cli
  pushad
  push esp
  push 19
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_20_handler:
  cli
  pushad
  push esp
  push 20
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_21_handler:
  cli
  pushad
  push esp
  push 21
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_22_handler:
  cli
  pushad
  push esp
  push 22
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_23_handler:
  cli
  pushad
  push esp
  push 23
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_24_handler:
  cli
  pushad
  push esp
  push 24
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_25_handler:
  cli
  pushad
  push esp
  push 25
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_26_handler:
  cli
  pushad
  push esp
  push 26
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_27_handler:
  cli
  pushad
  push esp
  push 27
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_28_handler:
  cli
  pushad
  push esp
  push 28
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_29_handler:
  cli
  pushad
  push esp
  push 29
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_30_handler:
  cli
  pushad
  push esp
  push 30
  call _interrupt_handler
  popad
  sti
  iret
  
  _int_31_handler:
  cli
  pushad
  push esp
  push 31
  call _interrupt_handler
  popad
  sti
  iret
  
  _irq_handler:
  cli
  pushad
  call _pic_handler
  popad
  sti
  iret
  
  