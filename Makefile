ASM      = nasm
CXX = g++
LD  = ld

CXXFLAGS = -ffreestanding \
           -fno-exceptions \
           -fno-rtti \
           -fno-stack-protector \
           -mno-red-zone \
           -mno-mmx \
           -mno-sse \
           -mno-sse2 \
           -mcmodel=kernel\
		   -fno-pie -fno-pic

LDFLAGS  = -T linker.ld

BUILD = build

KERNEL = $(BUILD)/kernel.elf
BOOT   = $(BUILD)/boot.o
KOBJ   = $(BUILD)/kernel.o

ISO = $(BUILD)/AzuredLinux.iso

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BOOT): boot/boot.asm | $(BUILD)
	$(ASM) -f elf64 $< -o $@

$(KOBJ): kernel/kernel.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(KERNEL): $(BOOT) $(KOBJ)
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(KERNEL)
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	cp iso/boot/grub/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BUILD)/iso

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

clean:
	rm -rf $(BUILD)

.PHONY: all run clean