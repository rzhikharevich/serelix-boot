HOST_OS != uname -s
EDK_ARCH = X64
ARCH     = x86_64
BITS     = 64

MDE   = ~/Development/UEFI/EDK2/MdePkg
CROSS = $(ARCH)-w64-mingw32-

C_STD = gnu11

COMMON_CFLAGS = \
	-flto \
	-std=$(C_STD) \
	-Ofast \
	-fstrict-aliasing \
	-fomit-frame-pointer

COMMON_CROSS_CFLAGS = \
	$(COMMON_CFLAGS) \
	-ffreestanding \
	-fno-stack-check -fno-stack-protector -mno-stack-arg-probe \
	-fpic \
	-fshort-wchar \
	-fstrict-aliasing \
	-mno-red-zone

CWARNS = \
	-Wall -Wextra \
	-Wno-missing-field-initializers \
	-Werror \
	-Wformat=2 \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-qual \
	-Wunreachable-code \
	-Winline \
	-Wno-unused-function

CFLAGS = \
	$(COMMON_CFLAGS) \
	$(CWARNS)

LDFLAGS = -flto -Ofast

CROSS_CC = $(CROSS)gcc
CROSS_CFLAGS = \
	$(COMMON_CROSS_CFLAGS) \
	$(COMMON_CFLAGS) \
	$(CWARNS) \
	-I $(MDE)/Include -I $(MDE)/Include/$(EDK_ARCH)

MUSL_CFLAGS = $(COMMON_CROSS_CFLAGS)

YASM = yasm

CROSS_LD = $(CROSS_CC)
CROSS_LDFLAGS  = -e Init -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -flto -Ofast -Wl,--discard-all
OBJS    != find src musl -name '*.c' | sed 's/\.c$$/\.o/g'
GHDRS   != find src -name '*.gh' | sed 's/\.gh$$/\.h/g'

OVMF = ~/Development/UEFI/OVMF-X64-r15214

QEMU = qemu-system-$(ARCH)
QEMUFLAGS = \
	-cpu qemu$(BITS) -m 128 \
	-L $(OVMF) -bios OVMF.fd \
	-drive format=raw,media=disk,file=boot.img

TOOLS = tools/template

all: build

build: BOOTX64.EFI

BOOTX64.EFI: $(TOOLS) $(GHDRS) $(OBJS) src/alotware.awf
	$(CROSS_LD) $(CROSS_LDFLAGS) $(OBJS) -o $@

%.awf: %.S
	yasm -f bin $< -o $@

%.h: %.gh
	tools/template < $< > $@

src/%.o: src/%.c
	$(CROSS_CC) $(CROSS_CFLAGS) -c $< -o $@

musl/%.o: musl/%.c
	$(CROSS_CC) $(MUSL_CFLAGS) -c $< -o $@

boot.img: BOOTX64.EFI boot.cfg src/alotware.awf
	tools/boot-image-$(HOST_OS).sh

qemu-ovmf-run-monitor: boot.img
	$(QEMU) $(QEMUFLAGS) -monitor stdio

qemu-ovmf-run-serial: boot.img
	$(QEMU) $(QEMUFLAGS) -serial stdio
