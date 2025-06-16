; MY ASM PLAYGROUND
; I TEST HOW TO DO BEFROE DOING


; PRINT INT

;global _start

;section .bss
;    buffer resb 32

;section .data
;    newline db 10

;section .text
;_start:
;    ; reserve buffer
;    mov rax, 5
;    mov rcx, buffer + 31; move tp end of bufger
;    ; divider
;    mov rbx, 10
;
;    mov rdx, 0
;
;    call .print_loop
;    mov rax, 60
;    mov rdi, 0
;    syscall

;.print_loop:
;    xor rdx, rdx ; ensure 0
;    div rbx ; if its 0 mean we done
;    add rdx, '0' ; convert to ascii
;    mov [rcx], dl ; write to buffer
;    dec rcx ; move backwards to go to next character as we started at end of buffer
;    test rax, rax; sets zf if rax == 0
;    jnz .print_loop ; if not zero repeat since we aint done

;    inc rcx ; move forward by 1 to go to first character

    ; GET LENGTH
;    mov rdx, buffer + 32
;    sub rdx, rcx

;    mov rax, 1
;    mov rdi, 1
;    mov rsi, rcx ; buffer in rcx
;    syscall

;    mov rax, 1
;    mov rdi, 1
;    lea rsi, [rel newline]
;    mov rdx, 1
;    syscall


; IF STATEMENTS

global _start:

section .bss
  buffer resb 32

section .data
  ifstrfalse db "false"
  ifstrlenfalse equ $ - ifstrfalse

  ifstrtrue db "true"
  ifstrlentrue equ $ - ifstrtrue
  newline db 10


section .text
_start:
  mov rax, 0
  ;mov rax, 1
  
  test rax, rax
  jnz .if_true

  ; if else
  mov rax, 1
  mov rdi, 1
  lea rsi, [rel ifstrfalse]
  mov rdx, ifstrlenfalse
  syscall

  mov rax, 1
  mov rdi, 1
  lea rsi, [rel newline]
  mov rdx, 1
  syscall

  ;call _start ; i was curious

  mov rax, 60
  mov rdi, 0
  syscall

.if_true:
  mov rax, 1
  mov rdi, 1,
  lea rsi, [rel ifstrtrue]
  mov rdx, ifstrlentrue
  syscall

  mov rax, 1
  mov rdi, 1
  lea rsi, [rel newline]
  mov rdx, 1
  syscall
  
  ; done

  mov rax, 60
  mov rdi, 0
  syscall
