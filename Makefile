CC ?= gcc
AR = ar
INCLUDE = ./include
SRC = ./src
CFLAGS += -I $(INCLUDE)
CFLAGS += -O2
ARFLAGS = cru
LIB = libhttp.a

C_SRC = $(SRC)/http.c

OBJ = $(C_SRC:.c=.o)

EXAMPLES = simple

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

libhttp.a: $(OBJ)
	$(AR) $(ARFLAGS) $(LIB) $^

examples: $(EXAMPLES)

simple: examples/simple.c libhttp.a
	$(CC) $< $(CFLAGS) -o $@ libhttp.a

clean:
	@rm -f $(OBJ)
	@rm -f $(LIB)
	@rm -f $(EXAMPLES)
