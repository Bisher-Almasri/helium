global _start
_start:
    mov rax, 1
    push rax
    mov rax, 7
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    mov rax, 3
    push rax
    push QWORD [rsp + 8]

    mov rax, 4
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    mov rax, 60
    pop rdi
    syscall