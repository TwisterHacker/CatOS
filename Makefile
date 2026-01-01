	OBJECTS = obj/loader.o 						 \
			  obj/main.o 						 \
			  obj/memory/memory_managment.o 	 \
			  obj/common/common.o				 \
			  obj/interrupts/descriptor_tables.o \
			  obj/interrupts/interrupts.o        \
			  obj/interrupts/descriptors.o       \
			  obj/interrupts/isr.o               \
			  obj/drivers/keyboard.o             \
			  obj/drivers/timer.o                \
			  obj/drivers/terminal.o             \
			  obj/drivers/shell.o                \
			  obj/drivers/floppy_disk_controller_driver.o \
			  obj/fs/fat12.o                     \
			  obj/drivers/fs.o                   \

	#as objects

    as_obj = 

    CC = gcc
    CFLAGS = -m32 -nostdlib -Iinclude -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -nostartfiles -c
    GCCPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -Iinclude -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
    LDFLAGS = -T linker.ld -melf_i386
    AS = nasm
    ASFLAGS = -f elf32

    all: kernel.iso
		

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS)  $< -o $@

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	g++ $(GCCPARAMS) -c -o $@ $<


obj/%.o: src/%.asm
	mkdir -p $(@D)
	as --32 $< -o $@

obj/%.o: src/%.s
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

kernel.iso: $(OBJECTS)
	clear
	nasm -f elf32 src/loader.s 
	ld $(LDFLAGS) $(OBJECTS) $(as_obj) -o kernel.elf

	
	sudo rm -rf iso	

	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub

	cp kernel.elf iso/boot
	cp grub.cfg iso/boot/grub/
	clear

	grub-mkrescue -o CatOS.iso iso



	rm -rf obj

	#'/mnt/d/Program Files/qemu/qemu-system-i386.exe' -soundhw pcspk -vga std -fda floppy.img -fdb floppy1.img -kernel kernel.elf -boot d

	'/mnt/d/Program Files/qemu/qemu-system-i386.exe' -soundhw pcspk -vga std -fda floppy11.img -cdrom CatOS.iso -boot d

