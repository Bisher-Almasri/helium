global _start
_start:
    mov rax, 7
    push rax
    push QWORD [rsp + 18446744073709551608]

    push QWORD [rsp + 8]

    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
