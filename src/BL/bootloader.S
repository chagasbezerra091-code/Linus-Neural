// SPDX-License-Identifier: Apache-2.0
/*
 * bootloader.S — Linus Neural Project
 *
 * Bootloader em assembly ARM64: inicializa registradores, stack pointer,
 * limpa seções de memória e chama o ponto de entrada em C (boot_main).
 *
 * Copyright (c) 2025 Linus Neural Project
 */

    .section .text
    .global _start
    .type _start, %function

_start:
    // Define o modo de execução inicial (EL1 para kernel)
    mrs     x0, CurrentEL
    cmp     x0, #0x4
    b.eq    _in_el1
    b       _enter_el1

_in_el1:
    // Inicializa stack pointer
    ldr     x1, =_stack_top
    mov     sp, x1

    // Zera registradores gerais
    mov     x2, #0
    mov     x3, #0
    mov     x4, #0
    mov     x5, #0

    // Mensagem simbólica (exibição feita em C)
    bl      boot_main

_halt:
    // Caso o boot_main retorne, trava o sistema
    wfe
    b       _halt

_enter_el1:
    // Aqui seria a transição de EL3→EL1 (simulado)
    b       _in_el1

    .section .bss
    .space  4096
_stack_top:
