#include <stdio.h>
#include <string.h>
#include "targetcode.h"

static const char *op_mnemonic(const char *op) {
    if (!strcmp(op, "+")) return "ADD";
    if (!strcmp(op, "-")) return "SUB";
    if (!strcmp(op, "*")) return "MUL";
    if (!strcmp(op, "/")) return "DIV";
    if (!strcmp(op, "%")) return "MOD";
    if (!strcmp(op, "<")) return "CMPLT";
    if (!strcmp(op, ">")) return "CMPGT";
    if (!strcmp(op, "<=")) return "CMPLE";
    if (!strcmp(op, ">=")) return "CMPGE";
    if (!strcmp(op, "==")) return "CMPEQ";
    if (!strcmp(op, "!=")) return "CMPNE";
    if (!strcmp(op, "&&")) return "AND";
    if (!strcmp(op, "||")) return "OR";
    if (!strcmp(op, "!")) return "NOT";
    if (!strcmp(op, "neg")) return "NEG";
    return "OP";
}

void generate_target_code(char **lines, int count) {
    for (int i = 0; i < count; i++) {
        char line[256];
        strncpy(line, lines[i], sizeof line - 1);
        line[sizeof line - 1] = 0;
        size_t l = strlen(line);

        /* "LABEL:" */
        if (l && line[l - 1] == ':') { printf("%s\n", line); continue; }

        /* "goto L" */
        if (!strncmp(line, "goto ", 5)) { printf("    JMP %s\n", line + 5); continue; }

        /* "ifFalse X goto L" */
        if (!strncmp(line, "ifFalse ", 8)) {
            char cond[64], label[64];
            if (sscanf(line + 8, "%63s goto %63s", cond, label) == 2) {
                printf("    CMP %s, #0\n", cond);
                printf("    JZ %s\n", label);
            }
            continue;
        }

        /* "print X" */
        if (!strncmp(line, "print ", 6)) { printf("    PRINT %s\n", line + 6); continue; }

        /* "A = ..." forms */
        char *eq = strstr(line, " = ");
        if (!eq) { printf("    ; %s\n", line); continue; }
        *eq = 0;
        const char *dst = line;
        char rhs[192];
        strncpy(rhs, eq + 3, sizeof rhs - 1); rhs[sizeof rhs - 1] = 0;

        char tok1[64] = "", tok2[64] = "", tok3[64] = "";
        int n = sscanf(rhs, "%63s %63s %63s", tok1, tok2, tok3);

        if (n == 1) {
            /* plain copy / literal: A = X */
            printf("    MOV %s, %s\n", dst, tok1);
        } else if (n == 2) {
            /* unary op: A = op X */
            printf("    MOV R1, %s\n", tok2);
            printf("    %s R1\n", op_mnemonic(tok1));
            printf("    MOV %s, R1\n", dst);
        } else if (n == 3) {
            /* binary op: A = X op Y */
            printf("    MOV R1, %s\n", tok1);
            printf("    MOV R2, %s\n", tok3);
            printf("    %s R1, R2\n", op_mnemonic(tok2));
            printf("    MOV %s, R1\n", dst);
        } else {
            printf("    ; %s = %s\n", dst, rhs);
        }
    }
}
