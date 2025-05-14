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

LMAP  = lmap
TEST1 = test1

EXT   = .elf
BINS += ${LMAP}${EXT} ${TEST1}${EXT}

# SRCS += ${LLIST}.c ${LMAP}.c


.PHONY: all t1run t1dbg clean

# all:${LMAP}
all: ${TEST1}


# loading screen tip: Makefile cmd will be executed if the dependencies change


# build lmap
lmap: $(LMAP).c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $(LMAP)${EXT}

# lmap_run: $(LMAP)${EXT}
# 	$(RUNAS) $(ENV) ./$<
# lmap_dbg: $(LMAP)${EXT}
# 	$(RUNAS) $(DBG) -iex=$(DBG_CMD) ./$<


test1: ${TEST1}.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o ${TEST1}${EXT}

t1run: ${TEST1}${EXT}
	$(RUNAS) $(ENV) ./$<

t1dbg: ${TEST1}${EXT}
	$(RUNAS) $(DBG) -iex=$(DBG_CMD) ./$<


clean:
	$(RM) $(BINS)
