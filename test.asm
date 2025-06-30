global _start
section .bss
    buffer resb 32
section .data
    newline db 10
section .text
_start:
    mov rax, 7
    push rax
    mov rax, 42
    push rax
    pop rbx
    pop rax
    add rax, rbx
    push rax
    mov rax, QWORD [rsp + 0]
    push rax
    pop rax
    mov rcx, buffer + 31
    mov rbx, 10
    mov rdx, 0
.print_loop#0:
    xor rdx, rdx
    div rbx
    add rdx, '0'
    mov [rcx], dl
    dec rcx
    test rax, rax
    jnz .print_loop#0
    inc rcx
    mov rdx, buffer + 32
    sub rdx, rcx
    mov rax, 1
    mov rdi, 1
    mov rsi, rcx
    syscall
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel newline]
    mov rdx, 1
    syscall
.print_done#1:
    mov rax, 0
    push rax
    pop rdi
    mov rax, 60
    syscall
