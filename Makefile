CC     = gcc
LEX    = flex
YACC   = bison
CFLAGS = -Wall -std=c11 -D_POSIX_C_SOURCE=200809L

SRC_DIR   = src
BUILD_DIR = build
TARGET    = mc-compiler

# On native Windows (MSYS2/MinGW-w64), gcc's linker names the output
# mc-compiler.exe regardless -- make that explicit here so every other
# rule/reference to $(TARGET) (linking, `make clean`, etc.) stays correct.
# $(OS) is a Windows-set environment variable ("Windows_NT"), inherited by
# MSYS2's bash/make same as any other Windows env var, so this check works
# whether you're building from an MSYS2 shell or a plain Windows one.
ifeq ($(OS),Windows_NT)
    TARGET := mc-compiler.exe
endif

OBJS = $(BUILD_DIR)/lex.yy.o \
       $(BUILD_DIR)/parser.tab.o \
       $(BUILD_DIR)/ast.o \
       $(BUILD_DIR)/symbol_table.o \
       $(BUILD_DIR)/semantic.o \
       $(BUILD_DIR)/codegen.o \
       $(BUILD_DIR)/main.o

.PHONY: all clean test

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# --- Generate parser from Bison grammar (also produces parser.tab.h) ---
$(SRC_DIR)/parser/parser.tab.c $(SRC_DIR)/parser/parser.tab.h: $(SRC_DIR)/parser/parser.y
	$(YACC) -d -o $(SRC_DIR)/parser/parser.tab.c $(SRC_DIR)/parser/parser.y

# --- Generate lexer from Flex spec (depends on parser.tab.h for token defs) ---
$(SRC_DIR)/lexer/lex.yy.c: $(SRC_DIR)/lexer/lexer.l $(SRC_DIR)/parser/parser.tab.h
	$(LEX) -o $(SRC_DIR)/lexer/lex.yy.c $(SRC_DIR)/lexer/lexer.l

# --- Compile each translation unit ---
$(BUILD_DIR)/lex.yy.o: $(SRC_DIR)/lexer/lex.yy.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/parser.tab.o: $(SRC_DIR)/parser/parser.tab.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ast.o: $(SRC_DIR)/ast/ast.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/symbol_table.o: $(SRC_DIR)/symbol_table/symbol_table.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/semantic.o: $(SRC_DIR)/semantic/semantic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/codegen.o: $(SRC_DIR)/codegen/codegen.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Link everything into the final executable ---
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# --- Run against every file in tests/ (useful for a quick sanity check) ---
test: all
	@for f in tests/*.mc; do \
		echo "==================================================="; \
		echo "Running: $$f"; \
		echo "==================================================="; \
		./$(TARGET) $$f; \
		echo ""; \
	done

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f $(SRC_DIR)/parser/parser.tab.c $(SRC_DIR)/parser/parser.tab.h
	rm -f $(SRC_DIR)/lexer/lex.yy.c
