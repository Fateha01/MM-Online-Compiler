CC = gcc
CFLAGS = -Wall -g
BISON = bison
FLEX = flex

SRC_DIR = src
BUILD_DIR = build
BIN = mm_cpp

AST = $(SRC_DIR)/ast
SYM = $(SRC_DIR)/symbol_table
SEM = $(SRC_DIR)/semantic
GEN = $(SRC_DIR)/codegen
OPT = $(SRC_DIR)/optimizer
TGT = $(SRC_DIR)/targetcode
LEX = $(SRC_DIR)/lexer
PAR = $(SRC_DIR)/parser

INCLUDES = -I$(AST) -I$(SYM) -I$(SEM) -I$(GEN) -I$(OPT) -I$(TGT) -I$(BUILD_DIR)

.PHONY: all clean test valgrind

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h: $(PAR)/parser.y | $(BUILD_DIR)
	$(BISON) -d -o $(BUILD_DIR)/parser.tab.c $(PAR)/parser.y

$(BUILD_DIR)/lexer.yy.c: $(LEX)/lexer.l $(BUILD_DIR)/parser.tab.h | $(BUILD_DIR)
	$(FLEX) -o $(BUILD_DIR)/lexer.yy.c $(LEX)/lexer.l

$(BIN): $(AST)/ast.c $(SYM)/symbol_table.c $(SEM)/semantic.c $(GEN)/codegen.c \
        $(OPT)/optimizer.c $(TGT)/targetcode.c \
        $(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/lexer.yy.c $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BIN) $^

clean:
	rm -rf $(BUILD_DIR) $(BIN)

test: $(BIN)
	./$(BIN) examples/demo.mmcpp

valgrind: $(BIN)
	valgrind --leak-check=full --error-exitcode=1 ./$(BIN) examples/demo.mmcpp
