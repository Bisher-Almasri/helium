; ASM IS NOT FUN
; ASM IS NOT FUN


global _start

section .bss
    buffer resb 32

section .data
    newline db 10

section .text
_start:
    ; reserve buffer
    mov rax, 5
    mov rcx, buffer + 31; move tp end of bufger
    ; divider
    mov rbx, 10

    mov rdx, 0

    call .print_loop
    mov rax, 60
    mov rdi, 0
    syscall

.print_loop:
    xor rdx, rdx ; ensure 0
    div rbx ; if its 0 mean we done
    add rdx, '0' ; convert to ascii
    mov [rcx], dl ; write to buffer
    dec rcx ; move backwards to go to next character as we started at end of buffer
    test rax, rax; sets zf if rax == 0
    jnz .print_loop ; if not zero repeat since we aint done

    inc rcx ; move forward by 1 to go to first character

    ; GET LENGTH
    mov rdx, buffer + 32
    sub rdx, rcx

    mov rax, 1
    mov rdi, 1
    mov rsi, rcx ; buffer in rcx
    syscall

    mov rax, 1
    mov rdi, 1
    lea rsi, [rel newline]
    mov rdx, 1
    syscall