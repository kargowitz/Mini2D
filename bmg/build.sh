gcc -ffreestanding -fno-pic -fno-pie -nostdlib -m32 -g -c baremetalgame.c -o baremetal.o
nasm -f elf32 entry.asm -o entry.o
nasm -f bin boot.asm -o boot.bin

ld -m elf_i386 -o kernel.bin -Ttext 0x1000 entry.o baremetal.o --oformat binary
cat boot.bin kernel.bin > src.bin
dd if=/dev/zero of=hashna.img bs=512 count=2880
dd if=src.bin of=hashna.img bs=512
rm boot.bin
rm entry.o
rm baremetal.o
rm kernel.bin