DBG		= gdb			# debugger
DBG_CMD		= "unset environment"	# debugger commands (initialization)
# try to mimic FreeRTOS settings for a typical board
RUNAS		= setarch i686 -3 -R	# x86, 3GB address space, no ASLR
ENV		= env -i		# empty environment
# LDFLAGS		+= -z execstack	-z norelro # executable stack, no RELRO
LDFLAGS		+= -z norelro
# x86 (32-bit) code, no {PIC, PIE},
# debug information, warnings
CFLAGS		+= -m32 -no-pie -fno-pic -ggdb3 -Wall -Wpedantic \
			-Wno-stringop-overflow

LMAP  = linked_map
TEST1 = posix_test
DHAKA = dhaka

EXT   = .elf
BINS += ${TEST1}${EXT}

SRCS += posix_test.c ${LMAP}.c ${DHAKA}.c ${DHAKA}.h


.PHONY: all 1run 1dbg clean


all: ${TEST1} 1run


# loading screen tip: Makefile cmd will be executed if the dependencies change


${TEST1}: ${TEST1}.c ${SRCS}
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o ${TEST1}${EXT}

1run: ${TEST1}${EXT} ${SRCS}
	$(RUNAS) $(ENV) ./$<

1dbg: ${TEST1}${EXT} ${SRCS}
	$(RUNAS) $(DBG) -iex=$(DBG_CMD) ./$<


clean:
	$(RM) $(BINS)
