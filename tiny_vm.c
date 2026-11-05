/* Tiny VM for the educational intermediate-code format produced by project 3.
   Build: cc -std=c11 -Wall -Wextra -O2 tiny_vm.c -o tiny_vm
   Run:   ./tiny_vm program.tac
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 2048
#define MAX_VARS 128
#define MAX_STACK 2048
#define MAX_NAME 64

typedef struct { char name[MAX_NAME]; int value; int initialized; } Variable;
typedef struct { char op[16]; char arg[128]; } Instruction;

static Variable vars[MAX_VARS];
static int var_count = 0;
static int stack_data[MAX_STACK];
static int sp = 0;

static int variable_index(const char *name, int create) {
    for (int i = 0; i < var_count; ++i) if (strcmp(vars[i].name, name) == 0) return i;
    if (!create || var_count >= MAX_VARS) return -1;
    strncpy(vars[var_count].name, name, MAX_NAME - 1);
    vars[var_count].name[MAX_NAME - 1] = '\0'; vars[var_count].value = 0; vars[var_count].initialized = 0;
    return var_count++;
}
static void push(int value) { if (sp >= MAX_STACK) { fprintf(stderr, "stack overflow\n"); exit(1); } stack_data[sp++] = value; }
static int pop_value(void) { if (sp <= 0) { fprintf(stderr, "stack underflow\n"); exit(1); } return stack_data[--sp]; }
static int parse_int(const char *text) { char *end; long value = strtol(text, &end, 10); if (*end) { fprintf(stderr, "bad integer: %s\n", text); exit(1); } return (int)value; }

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s program.tac\n", argv[0]); return 2; }
    FILE *file = fopen(argv[1], "r"); if (!file) { perror(argv[1]); return 2; }
    Instruction code[MAX_LINES]; int count = 0; char line[256];
    while (fgets(line, sizeof line, file) && count < MAX_LINES) {
        char *trim = line; while (*trim == ' ' || *trim == '\t') ++trim;
        if (*trim == '\n' || *trim == '\0' || *trim == '#') continue;
        int pc; char op[16] = "", arg[128] = "";
        int fields = sscanf(trim, "%d: %15s %127[^\n]", &pc, op, arg);
        if (fields >= 2) { snprintf(code[count].op, sizeof code[count].op, "%s", op); snprintf(code[count].arg, sizeof code[count].arg, "%s", fields == 3 ? arg : ""); ++count; }
    }
    fclose(file);

    int pc = 0;
    while (pc >= 0 && pc < count) {
        Instruction *ins = &code[pc];
        if (strcmp(ins->op, "DECL") == 0) { int ix = variable_index(ins->arg, 1); if (ix < 0) return 1; vars[ix].value = 0; vars[ix].initialized = 1; }
        else if (strcmp(ins->op, "CONST") == 0) push(parse_int(ins->arg));
        else if (strcmp(ins->op, "LOAD") == 0) { int ix = variable_index(ins->arg, 0); if (ix < 0 || !vars[ix].initialized) { fprintf(stderr, "uninitialized variable: %s\n", ins->arg); return 1; } push(vars[ix].value); }
        else if (strcmp(ins->op, "STORE") == 0) { int ix = variable_index(ins->arg, 1); if (ix < 0) return 1; vars[ix].value = pop_value(); vars[ix].initialized = 1; }
        else if (strcmp(ins->op, "ADD") == 0) { int b = pop_value(), a = pop_value(); push(a + b); }
        else if (strcmp(ins->op, "SUB") == 0) { int b = pop_value(), a = pop_value(); push(a - b); }
        else if (strcmp(ins->op, "MUL") == 0) { int b = pop_value(), a = pop_value(); push(a * b); }
        else if (strcmp(ins->op, "DIV") == 0) { int b = pop_value(), a = pop_value(); if (!b) { fprintf(stderr, "division by zero\n"); return 1; } push(a / b); }
        else if (strcmp(ins->op, "GT") == 0) { int b = pop_value(), a = pop_value(); push(a > b); }
        else if (strcmp(ins->op, "LT") == 0) { int b = pop_value(), a = pop_value(); push(a < b); }
        else if (strcmp(ins->op, "EQ") == 0) { int b = pop_value(), a = pop_value(); push(a == b); }
        else if (strcmp(ins->op, "JZ") == 0) { int condition = pop_value(); if (!condition) { pc = parse_int(ins->arg); continue; } }
        else if (strcmp(ins->op, "JMP") == 0) { pc = parse_int(ins->arg); continue; }
        else if (strcmp(ins->op, "PRINT") == 0) printf("%d\n", pop_value());
        else if (strcmp(ins->op, "LABEL") == 0 || strcmp(ins->op, "HALT") == 0) { if (strcmp(ins->op, "HALT") == 0) break; }
        else { fprintf(stderr, "unknown opcode: %s\n", ins->op); return 1; }
        ++pc;
    }
    return 0;
}
