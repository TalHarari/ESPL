EXIT EQU 1                      ; Define system call number for exit
READ EQU 3                      ; Define system call number for read
WRITE EQU 4                     ; Define system call number for write
OPEN EQU 5                      ; Define system call number for open
CLOSE EQU 6                     ; Define system call number for close
STDIN EQU 0                     ; Standard input file descriptor
STDOUT EQU 1                    ; Standard output file descriptor
STDERR EQU 2                    ; Standard error file descriptor
O_RDONLY EQU 0                  ; Flag for read-only mode
O_WRONLY EQU 1                  ; Flag for write-only mode
O_CREATE EQU 64                 ; Flag to create file if it doesn't exist
O_RDRW EQU 2                    ; Flag for read and write mode
O_TRUNC EQU 512                 ; Flag to truncate file

section .data
    newline db 10               ; Define newline character
    Infile dd STDIN             ; Default input file descriptor is standard input
    Outfile dd STDOUT           ; Default output file descriptor is standard output

section .bss
    buffer resb 1               ; Reserve 1 byte for buffer

section .text
    global _start               ; Define entry point for the program
    global system_call          ; Declare system_call as global
    extern strlen               ; Declare strlen function as external

_start:
    pop dword ecx               ; Get the argument count (argc) from the stack
    mov esi, esp                ; Set esi to point to the argument vector (argv)
    ; lea eax, [esi+4*ecx+4]    ; Compute address of the environment pointer (envp)
    mov eax, ecx                ; Copy argc into eax
    shl eax, 2                  ; Multiply argc by 4 to compute size of argv in bytes
    add eax, esi                ; Add size of argv to its base address
    add eax, 4                  ; Skip NULL at the end of argv
    push dword eax              ; Push envp (environment pointer) onto the stack
    push dword esi              ; Push argv (argument vector) onto the stack
    push dword ecx              ; Push argc (argument count) onto the stack

    call main                   ; Call main(argc, argv, envp)

    mov ebx, eax                ; Move return value of main into ebx
    mov eax, EXIT               ; Load exit system call number into eax
    int 0x80                    ; Perform system call to exit
    nop                         ; No operation (padding)

system_call:
    push ebp                    ; Save base pointer of the caller
    mov ebp, esp                ; Set base pointer to current stack pointer
    sub esp, 4                  ; Allocate space for local variable on stack
    pushad                      ; Save general-purpose registers

    mov eax, [ebp+8]            ; Load first argument (system call number) into eax
    mov ebx, [ebp+12]           ; Load second argument into ebx
    mov ecx, [ebp+16]           ; Load third argument into ecx
    mov edx, [ebp+20]           ; Load fourth argument into edx
    int 0x80                    ; Perform system call
    mov [ebp-4], eax            ; Store return value of system call
    popad                       ; Restore general-purpose registers
    mov eax, [ebp-4]            ; Move return value to eax
    add esp, 4                  ; Restore stack pointer
    pop ebp                     ; Restore base pointer of the caller
    ret                         ; Return to caller

main:
    push ebp                    ; Save base pointer of the caller
    mov ebp, esp                ; Set base pointer to current stack pointer

argvs:
    mov ecx, [ebp+8]            ; Load argc (argument count) into ecx
    mov edx, [ebp+12]           ; Load argv (argument vector) into edx
    mov ebx, 0                  ; Initialize counter in ebx

nextArg:
    push ebx                    ; Save counter value
    push ecx                    ; Save argc value
    push edx                    ; Save argv pointer
    mov ecx, [edx+ebx*4]        ; Load address of current argument into ecx
    push ecx                    ; Push current argument onto stack

checkInRedirect:
    cmp word [ecx], "-i"        ; Check if argument is "-i" (input redirection)
    jnz checkOutRedirect        ; If not, jump to output redirection check
    add ecx, 2                  ; Skip "-i"
    ; open input
    mov eax, OPEN               ; Load open system call number into eax
    mov ebx, ecx                ; Load file name into ebx
    mov ecx, O_RDONLY           ; Load read-only flag into ecx
    int 0x80                    ; Perform system call to open file
    mov [Infile], eax           ; Store file descriptor in Infile
    jmp printArg                ; Jump to print argument

checkOutRedirect:
    cmp word [ecx], "-o"        ; Check if argument is "-o" (output redirection)
    jnz printArg                ; If not, jump to print argument
    add ecx, 2                  ; Skip "-o"
    ; open output
    mov eax, OPEN               ; Load open system call number into eax
    mov ebx, ecx                ; Load file name into ebx
    mov ecx, O_WRONLY | O_CREATE | O_TRUNC ; Load flags for write, create, and truncate
    mov edx, 0q700              ; Set permissions to rwx for user
    int 0x80                    ; Perform system call to open file
    mov [Outfile], eax          ; Store file descriptor in Outfile

printArg:
    pop ecx                     ; Restore argument pointer
    push ecx                    ; Push argument pointer for strlen
    call strlen                 ; Call strlen to get length of argument
    add esp, 4                  ; Clean up stack
    ; prepare for Write
    mov edx, eax                ; Move length of argument into edx
    mov eax, WRITE              ; Load write system call number into eax
    mov ebx, STDERR             ; Load STDERR as file descriptor
    int 0x80                    ; Perform system call to write argument
    mov eax, WRITE              ; Prepare to write newline
    mov ecx, newline            ; Load newline character address into ecx
    mov edx, 1                  ; Set length of newline to 1
    int 0x80                    ; Perform system call to write newline
    pop edx                     ; Restore argv pointer
    pop ecx                     ; Restore argc
    pop ebx                     ; Restore counter
    inc ebx                     ; Increment counter
    cmp ebx, ecx                ; Compare counter with argc
    jnz nextArg                 ; If not equal, continue processing arguments

; encoder
read:
    mov eax, READ               ; Load read system call number into eax
    mov ebx, [Infile]           ; Load input file descriptor into ebx
    mov ecx, buffer             ; Load buffer address into ecx
    mov edx, 1                  ; Read 1 byte at a time
    int 0x80                    ; Perform system call to read
    cmp byte [buffer], 0        ; Check if buffer contains null character
    jz finish                   ; If null, jump to finish
    cmp eax, 0                  ; Check if end of file
    jle finish                   ; If end of file, jump to finish

encode:
    mov al, [buffer]            ; Load byte from buffer into al
    cmp al, 'A'                 ; Check if character is below 'A'
    jl print                    ; If less, jump to print
    cmp al, 'Z'                 ; Check if character is above 'Z'
    jle upper                   ; If within range, jump to upper
    cmp al, 'a'                 ; Check if character is below 'a'
    jl print                    ; If less, jump to print
    cmp al, 'z'                 ; Check if character is above 'z'
    jle lower                   ; If within range, jump to lower
    jmp print                   ; Otherwise, jump to print

upper:
    cmp al, 'Z'                 ; Check if character is 'Z'
    jz ZCase                    ; If yes, jump to ZCase
    inc al                      ; Increment character
    jmp print                   ; Jump to print

ZCase:
    mov al, 'A'                 ; Wrap around to 'A'
    jmp print                   ; Jump to print

lower:
    cmp al, 'z'                 ; Check if character is 'z'
    jz zCase                    ; If yes, jump to zCase
    inc al                      ; Increment character
    jmp print                   ; Jump to print

zCase:
    mov al, 'a'                 ; Wrap around to 'a'
    jmp print                   ; Jump to print

print:
    mov [buffer], al            ; Store encoded character
    mov eax, WRITE
    mov ebx, [Outfile]
    mov ecx, buffer
    mov edx, 1
    int 0x80 ; print
    jmp read
finish:
close:
    mov eax, CLOSE
    mov ebx, [Infile]
    int 0x80
    mov eax, CLOSE
    mov ebx, [Outfile]
    int 0x80
done:
    mov esp, ebp
    pop ebp
    ret 