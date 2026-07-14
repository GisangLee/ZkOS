NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy
QEMU = "C:\Program Files\qemu\qemu-system-i386.exe"

all: os_image.bin

# 1. 부트로더 컴파일
boot/boot.bin: boot/boot.asm
	$(NASM) -f bin boot/boot.asm -o boot/boot.bin

# 2. 커널 C 소스들 컴파일 (SSE 및 실수 연산 장치 비활성화 옵션 추가)
kernel/kernel.o: kernel/kernel.c
	$(GCC) -m32 -ffreestanding -mno-sse -mno-80387 -c kernel/kernel.c -o kernel/kernel.o

kernel/idt.o: kernel/idt.c
	$(GCC) -m32 -ffreestanding -mno-sse -mno-80387 -c kernel/idt.c -o kernel/idt.o

# 3. 커널 링킹 및 순수 바이너리 추출
kernel/kernel.bin: kernel/kernel.o kernel/idt.o
	$(LD) -m i386pe -Ttext 0x1000 -e _kernel_main -o kernel/kernel.tmp kernel/kernel.o kernel/idt.o
	$(OBJCOPY) -O binary kernel/kernel.tmp kernel/kernel.bin

# 4. 최종 OS 이미지 병합
os_image.bin: boot/boot.bin kernel/kernel.bin
	powershell -Command "gc boot/boot.bin, kernel/kernel.bin -Encoding Byte -ReadCount 0 | sc os_image.bin -Encoding Byte"

run: os_image.bin
	$(QEMU) -fda os_image.bin

clean:
	powershell -Command "Remove-Item -ErrorAction Ignore boot/*.bin, kernel/*.o, kernel/*.bin, kernel/*.tmp, os_image.bin"