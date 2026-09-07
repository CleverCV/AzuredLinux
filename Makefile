ASM = nasm
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
           -mcmodel=small \
           -fno-pie -fno-pic

LDFLAGS = -T linker.ld

BUILD = build

KERNEL = $(BUILD)/kernel.elf
BOOT   = $(BUILD)/boot.o
KOBJ   = $(BUILD)/kernel.o $(BUILD)/idt.o $(BUILD)/pic.o $(BUILD)/keyboard.o

ISO = $(BUILD)/AzuredLinux.iso

all: $(ISO)

$(BUILD)/pic.o: kernel/pic.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/keyboard.o: kernel/keyboard.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/isr.o: kernel/isr.asm | $(BUILD)
	$(ASM) -f elf64 $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(BOOT): boot/boot.asm | $(BUILD)
	$(ASM) -f elf64 $< -o $@

$(BUILD)/kernel.o: kernel/kernel.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/idt.o: kernel/idt.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(KERNEL): $(BOOT) $(KOBJ) $(BUILD)/isr.o
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