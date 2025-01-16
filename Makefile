BINARY=ironpawn
CODEDIRS=lib src
INCDIRS=include

CC=gcc
OPT=-O0
CFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT)

CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c, out/%.o, $(CFILES))

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^

out/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(BINARY)
	./$(BINARY)

debug: $(BINARY)
	./$(BINARY) debug

clean:
	rm -rf $(BINARY) out
