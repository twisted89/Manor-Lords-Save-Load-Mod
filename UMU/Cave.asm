extern lpPatch5Return:QWORD
extern lpPatch5ROrig:QWORD

.code 
hkPatch5 proc
	cmp rcx,0
	jnz jmpback
	mov rcx,lpPatch5Return
	jmp rcx
jmpback:
	jmp lpPatch5ROrig
hkPatch5 endp
end