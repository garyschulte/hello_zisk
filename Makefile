# Toolchain auto-detection
TOOLCHAIN_PREFIX = riscv64-unknown-elf-

# Check for toolchain in common locations
TOOLCHAIN_PATHS = /opt/homebrew/bin /usr/local/bin /opt/riscv/bin
TOOLCHAIN_DIR = $(firstword $(foreach dir,$(TOOLCHAIN_PATHS),$(shell test -x $(dir)/$(TOOLCHAIN_PREFIX)gcc && echo $(dir))))

ifneq ($(TOOLCHAIN_DIR),)
    TOOLCHAIN = $(TOOLCHAIN_DIR)/$(TOOLCHAIN_PREFIX)
else
    # Fall back to assuming it's in PATH
    TOOLCHAIN = $(TOOLCHAIN_PREFIX)
endif

CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCOPY = $(TOOLCHAIN)objcopy
OBJDUMP = $(TOOLCHAIN)objdump
SIZE = $(TOOLCHAIN)size

# Target architecture
ARCH = rv64im
ABI = lp64

# Custom newlib installation
NEWLIB_DIR = $(abspath newlib-rv64im)
NEWLIB_INC = $(NEWLIB_DIR)/riscv64-unknown-elf/include
NEWLIB_LIB = $(NEWLIB_DIR)/riscv64-unknown-elf/lib

# Flags
ARCH_FLAGS = -march=$(ARCH) -mabi=$(ABI) -mcmodel=medlow
CFLAGS = $(ARCH_FLAGS) -O2 -g -Wall -Wextra -nostdlib -ffreestanding -isystem $(NEWLIB_INC)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fno-use-cxa-atexit
ASFLAGS = $(ARCH_FLAGS)
LDFLAGS = -T link.ld -nostdlib -nostartfiles -L$(NEWLIB_LIB) -L$(shell dirname $(shell $(CC) $(ARCH_FLAGS) -print-libgcc-file-name))
LIBS = -lc -lm -lgcc

# Source files
SOURCES_C = syscalls.c
SOURCES_CXX = main.cpp
SOURCES_ASM = start.S

# Object files
OBJECTS = $(SOURCES_ASM:.S=.o) $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

# Output
TARGET = hello_world

.PHONY: all clean disasm size

all: $(TARGET).elf $(TARGET).bin
	@echo "Build complete!"
	@$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

disasm: $(TARGET).elf
	$(OBJDUMP) -d $< > $(TARGET).disasm
	@echo "Disassembly written to $(TARGET).disasm"

size: $(TARGET).elf
	$(SIZE) -A $<

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).bin $(TARGET).disasm
