#ifndef _FUNCTIONS
#define _FUNCTIONS

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "structs.h"
#include "configurations.h"


int current_line(FILE* input, char* line);
int encode_instruction(const char* line, Instruction* output, char* labels[INSTR_MEM_DEPTH], FILE* imemin);
Line_Type get_line_type(char* cur_line, Instruction* sInstruction, char* label_addresses_lst[INSTR_MEM_DEPTH], FILE* fImemin);
int clean_and_trim_line(const char* to_fix, char* fixed);
OpCode fetch_opcode(const char* opcode);
Register fetch_register(const char* reg);
void to_lowercase(const char* input, char* output);
int parse_numeric_value(const char* field, char* labels[INSTR_MEM_DEPTH]);
char* parse_label(char* cleaned_line, BOOL* inline_label);
void process_pseudo(const char* line, int* memory_data);
void load_data_to_file(FILE* output_stream, DATA* output_data, int size);
int find_last_non_zero(DATA* data_memory, int max_size);
BOOL has_label(const char* line);
BOOL is_pseudo_inst(const char* line);
BOOL is_line_comment(const char* line);
#endif