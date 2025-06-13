global _start
_start:
    mov rax, 5
    push rax
    mov rax, 6
    push rax
    mov rax, 2
    push rax
    pop rax
    pop rbx
    mul rbx
    push rax
    mov rax, 3
    push rax
    mov rax, 6
    push rax
    mov rax, 3
    push rax
    mov rax, 4
    push rax
    pop rax
    pop rbx
    mul rbx
    push rax
    pop rax
    pop rbx
    div rbx
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    pop rax
    pop rbx
    add rax, rbx
    push rax
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    mov rax, 3
    push rax
    mov rax, 2
    push rax
    mov rax, 1
    push rax
    pop rax
    pop rbx
    mul rbx
    push rax
    pop rax
    pop rbx
    mul rbx
    push rax
    mov rax, 1
    push rax
    mov rax, 3
    push rax
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    mov rax, 2
    push rax
    pop rax
    pop rbx
    mul rbx
    push rax
    push QWORD [rsp + 0]

    push QWORD [rsp + 16]

    push QWORD [rsp + 16]

    pop rax
    pop rbx
    add rax, rbx
    push rax
    pop rax
    pop rbx
    sub rax, rbx
    push rax
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
