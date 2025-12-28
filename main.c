#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "structs.h"
#include "configurations.h"
#include "asm_functions.h"

/*
 * Structure to hold all file handles used by the assembler
 * Keeps related file pointers grouped together for better organization
 */
typedef struct {
    FILE* program;    // Input assembly program file
    FILE* imemin;     // Output instruction memory file
    FILE* dmemin;     // Output data memory file
} FileHandles;

/*
 * Structure to maintain the assembler's state during processing
 * Groups related data used across different passes of the assembly process
 */
typedef struct {
    char* label_addresses[INSTR_MEM_DEPTH];
    int* data_memory;
    int instruction_count;
} AssemblerState;

/*
 * Opens all required files for the assembly process
 * @param argv Array of command line arguments containing file paths
 * @return FileHandles structure with opened file pointers
 */
static FileHandles open_files(char* argv[]) {
    FileHandles files = {
        .program = fopen(argv[1], "r"),  // Open assembly file for reading
        .imemin = fopen(argv[2], "w"),   // Open instruction memory file for writing
        .dmemin = fopen(argv[3], "w")    // Open data memory file for writing
    };
    return files;
}


/*
 * Processes a single line during the first pass of the assembler
 * Handles labels, pseudo-instructions, and counts regular instructions
 *
 * @param cleaned_line The preprocessed line without whitespace
 * @param temp_label Temporary storage for any label found
 * @param label_and_inst Boolean indicating if line has both label and instruction
 * @param state Pointer to the assembler state structure
 */
static void process_line_first_pass(const char* cleaned_line, char* temp_label,
    BOOL label_and_inst, AssemblerState* state) {
    if (temp_label) {
        // Store label and update instruction count if needed
        state->label_addresses[state->instruction_count] = temp_label;
        state->instruction_count += (int)label_and_inst;
    }
    else if (is_pseudo_inst(cleaned_line)) {
        // Handle pseudo-instructions (e.g., .word, .data)
        process_pseudo(cleaned_line, state->data_memory);
    }
    else if (!is_line_comment(cleaned_line) && strlen(cleaned_line) > 0) {
        // Count regular instructions
        state->instruction_count++;
    }
}

int main(int argc, char* argv[]) {


    if (argc < 4) {
        fprintf(stderr, "Error: Too few arguments! Expected 3, got %d\n", argc-1);
        exit(-1);
    }
        
    // Initialize file handles and open all required files
    FileHandles files = open_files(argv);

    // Initialize assembler state
    AssemblerState state = {
        .instruction_count = 0,
        .data_memory = (int*)calloc(DATA_MEM_DEPTH, sizeof(int))
    };
    memset(state.label_addresses, 0, sizeof(state.label_addresses));

    // Working variables for processing
    char cur_line[MAX_LINE_LENGTH];           // Buffer for current line
    char cur_cleaned_line[MAX_LINE_LENGTH];   // Buffer for preprocessed line
    Instruction sInstruction;                 // Structure to hold instruction details
    Line_Type Type_of_inst = 0;              // Type of current instruction

    // First pass: Process labels and count instructions
    while (!feof(files.program)) {
        // Read and validate current line
        if (current_line(files.program, cur_line) == 0) {
            continue;  // Skip empty lines
        }

        // Process line for labels
        BOOL label_and_inst = 0;
        clean_and_trim_line(cur_line, cur_cleaned_line);
        char* temp_label = parse_label(cur_cleaned_line, &label_and_inst);

        process_line_first_pass(cur_cleaned_line, temp_label, label_and_inst, &state);
    }

    // Prepare for second pass
    fseek(files.program, 0, SEEK_SET);

    // Second pass: Process instructions and generate machine code
    int line_counter = 1;
    while (!feof(files.program)) {
        if (current_line(files.program, cur_line) == 0) {
            continue;
        }

        // Process instruction and write to output file
        Type_of_inst = get_line_type(cur_line, &sInstruction,
            state.label_addresses, files.imemin);

        if (Type_of_inst == REGULAR_INST) {
            line_counter++;
        }
    }

    // Ensure all instruction memory is written
    fflush(files.imemin);

    // Write data memory contents
    int depth = find_last_non_zero((unsigned int*)state.data_memory, DATA_MEM_DEPTH);
    load_data_to_file(files.dmemin, (unsigned int*)state.data_memory, depth + 1);

    // Cleanup phase - free all allocated memory
    for (int i = 0; i < INSTR_MEM_DEPTH; i++) {
        free(state.label_addresses[i]);
    }
    free(state.data_memory);

    // Close all files
    fclose(files.imemin);
    fclose(files.dmemin);
    fclose(files.program);

    return 0;
}
