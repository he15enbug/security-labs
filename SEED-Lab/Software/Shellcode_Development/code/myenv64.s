section .text
	global _start
_start:
	BITS 64
	jmp short two
one:
	pop rbx           

	; set terminating 0x00 for strings
	xor al, al
	mov [rbx+12], al
	mov [rbx+22], al
	mov [rbx+32], al
	mov [rbx+48], al

	; fill placeholders for argv
	mov [rbx+56], rbx
	xor rax     , rax
	mov [rbx+64], rax

	; fill placeholders for env
	lea rax     , [rbx+13]
	mov [rbx+72], rax
	lea rax     , [rbx+23]
	mov [rbx+80], rax
	lea rax     , [rbx+33]
	mov [rbx+88], rax
	xor rax     , rax
	mov [rbx+96], rax


	mov rdi, rbx
	lea rsi, [rbx+56]
	lea rdx, [rbx+72]
	mov rax, 0x1111113c
	sub rax, 0x11111101; rax = 59
	syscall
two:
	call one
	db '/usr/bin/env'      , 0xff ; 13 bytes
	db 'aaa=hello'         , 0xff ; 10 bytes
	db 'bbb=world'         , 0xff ; 10 bytes
	db 'ccc=hello world'   , 0xff ; 16 bytes

	db 'AAAAAAAA'      ; 8 bytes, place holder for argv[0]
	db 'AAAAAAAA'      ; 8 bytes, place holder for argv[1]
	db 'BBBBBBBB'      ; 8 bytes, Place holder for env[0]
	db 'CCCCCCCC'      ; 8 bytes, Place holder for env[1]
	db 'DDDDDDDD'      ; 8 bytes, Place holder for env[2]
	db 'EEEEEEEE'      ; 8 bytes, Place holder for env[3]
