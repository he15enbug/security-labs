section .text
	global _start
_start:
	xor  rdx, rdx       ; 3rd argument (stored in rdx)
	push rdx

	mov rax, 'la'
	push rax
	mov rax, 'lo; ls -'
	push rax
	mov rax, 'echo hel'
	push rax

	mov rax, '-c'
	push rax

	push rdx
	mov rax, 'h'
	push rax
	mov rax, '/bin/bas'
	push rax
	mov rdi, rsp        ; 1st argument (stored in rdi)

	push rdx
	lea  rax, [rdi+32]
	push rax
	lea  rax, [rdi+24]
	push rax
	push rdi
	mov rsi, rsp        ; 2nd argument (stored in rsi)

	xor  rax, rax
	mov  al, 59        ; execve()
	syscall
