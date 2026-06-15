global task_switch
extern scheduler_next

task_switch:
push ebp
mov ebp, esp

push ebx
push esi
push edi

mov eax, [ebp + 8]

mov [eax + 16], esp

push eax
call scheduler_next
add esp, 4

; eax = task_t*

mov eax, [eax + 4]

mov esp, [eax + 16]

mov ebx, [eax + 0]
mov ecx, [eax + 4]
mov edx, [eax + 8]
mov esi, [eax + 24]
mov edi, [eax + 28]

mov esp, ebp
pop ebp

ret