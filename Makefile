PROJECT := novaos
BUILD_DIR := build
ISO_DIR := $(BUILD_DIR)/isofiles
KERNEL := $(BUILD_DIR)/$(PROJECT).kernel
ISO := $(BUILD_DIR)/$(PROJECT).iso
DISK := $(BUILD_DIR)/$(PROJECT).disk

CC := gcc
LD := ld
OBJCOPY := objcopy

CFLAGS := -m32 -std=c11 -ffreestanding -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -Wall -Wextra -Werror -O2
LDFLAGS := -m elf_i386 -T linker.ld -nostdlib

.PHONY: all kernel iso disk run smoke clean format

all: iso

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot/boot.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: kernel/main.c kernel/abi.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/arch.o: kernel/arch.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/interrupts.o: kernel/interrupts.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/process.o: kernel/process.c kernel/abi.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fs.o: kernel/fs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/storage.o: kernel/storage.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/persistent_fs.o: kernel/persistent_fs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/syscalls.o: kernel/syscalls.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/scheduler.o: kernel/scheduler.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

kernel: $(BUILD_DIR)/boot.o $(BUILD_DIR)/main.o $(BUILD_DIR)/arch.o $(BUILD_DIR)/interrupts.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/process.o $(BUILD_DIR)/fs.o $(BUILD_DIR)/storage.o $(BUILD_DIR)/persistent_fs.o $(BUILD_DIR)/syscalls.o $(BUILD_DIR)/scheduler.o linker.ld
	$(LD) $(LDFLAGS) -o $(KERNEL) $(BUILD_DIR)/boot.o $(BUILD_DIR)/main.o $(BUILD_DIR)/arch.o $(BUILD_DIR)/interrupts.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/process.o $(BUILD_DIR)/fs.o $(BUILD_DIR)/storage.o $(BUILD_DIR)/persistent_fs.o $(BUILD_DIR)/syscalls.o $(BUILD_DIR)/scheduler.o
	grub-file --is-x86-multiboot $(KERNEL)

iso: kernel boot/grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/novaos.kernel
	cp boot/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR)

disk: tools/mkfs.py | $(BUILD_DIR)
	python3 tools/mkfs.py $(DISK)

run: iso disk
	qemu-system-i386 -cdrom $(ISO) -drive file=$(DISK),format=raw,if=ide -serial stdio -display none -no-reboot -no-shutdown

smoke: iso disk
	@set -eu; \
	log_file=$$(mktemp); \
	(timeout 8s qemu-system-i386 -cdrom $(ISO) -drive file=$(DISK),format=raw,if=ide -serial file:$$log_file -display none -no-reboot -no-shutdown >/dev/null 2>&1 || true); \
	cat $$log_file; \
	grep -q 'NOVAOS_M10_SCHEDULER_OK' $$log_file; \
	rm -f $$log_file; \
	echo 'NovaOS M10 scheduler smoke test passed.'

clean:
	rm -rf $(BUILD_DIR)
