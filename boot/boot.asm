[org 0x7c00]

KERNEL_OFFSET equ 0x1000    ; C 커널이 로드될 메모리 주소 설정

; 1. 16비트 리얼 모드 초기화
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

; 2. 디스크에서 C 커널 읽어오기 (BIOS 인터럽트 사용)
    mov bx, KERNEL_OFFSET   ; 데이터를 받을 메모리 주소 (ES:BX = 0x0000:0x1000)
    mov dh, 15              ; 읽어올 디스크 섹터 수 (C 커널이 커질 것을 대비해 넉넉히 15섹터)
    
    mov ah, 0x02            ; BIOS 읽기 기능 번호
    mov al, dh              ; 읽을 섹터 수
    mov ch, 0x00            ; 실린더 번호 (0)
    mov dh, 0x00            ; 헤드 번호 (0)
    mov cl, 0x02            ; 시작 섹터 번호 (2번째 섹터부터 커널이 저장됨)
    mov dl, 0x00            ; 드라이브 번호 (0 = 첫 번째 플로피/가상 디스크)
    int 0x13                ; 디스크 읽기 인터럽트 호출

; 3. 32비트 보호 모드로 전환 준비
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    ; 4. 메모리 0x1000 주소에 로드된 C 커널 함수로 제어권을 넘깁니다!
    call KERNEL_OFFSET

    jmp $

; ------------------------------------------------------------------
; GDT (Global Descriptor Table)
; ------------------------------------------------------------------
gdt_start:
    dd 0x0, 0x0
gdt_code:
    dw 0xffff, 0x0
    db 0x0, 10011010b, 11001111b, 0x0
gdt_data:
    dw 0xffff, 0x0
    db 0x0, 10010010b, 11001111b, 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 부트 섹터 512바이트 시그니처
times 510 - ($ - $$) db 0
dw 0xaa55