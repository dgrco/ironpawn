BINARY=ironpawn
WASM_OUT=engine.js
CODEDIRS=src
EMCODEDIRS=wasm
INCDIRS=include

EMCC=emcc
EMFLAGS = -sEXPORTED_FUNCTIONS=_wasm_process_uci_command,_wasm_init_bitboards,_wasm_init_magic -sEXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "UTF8ToString"]' -sALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -sENVIRONMENT=web

CC=gcc
OPT=-O3
CFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT)

CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
EMCFILES=$(foreach D,$(EMCODEDIRS),$(wildcard $(D)/*.c))
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

rook_magic: $(BINARY)
	./$(BINARY) rook_magic

bishop_magic: $(BINARY)
	./$(BINARY) bishop_magic

wasm: $(CFILES) $(EMCFILES)
	$(EMCC) $(CFLAGS) $(EMFLAGS) $(CFILES) $(EMCFILES) -o $(WASM_OUT) 

.PHONY: wasm

clean:
	rm -rf $(BINARY) out engine.js engine.wasm
