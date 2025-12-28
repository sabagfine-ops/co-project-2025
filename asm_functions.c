#include "asm_functions.h"

/*
 <summary>
 Reads the current line from the input file.
 </summary>
 <param name="input_file">The file to read from.</param>
 <param name="line">The buffer to store the read line.</param>
 <returns>The number of characters read.</returns>
 */
int current_line(FILE* input_file, char* line) {
	if (!input_file) {
		*line = '\0';  // Using pointer dereference for clarity
		return 0;
	}

	int i = 0;
	int c = fgetc(input_file);  // Read the first character

	while (c != EOF && c != '\n' && c != '\0') {
		line[i++] = (char)c;
		c = fgetc(input_file);  // Read the next character
	}

	line[i] = '\0';  // Null-terminate the string
	return i;        // Return the number of characters read
}


/*
<summary>
Loads the instruction into imemin
</summary>
<param name="line">Instruction line</param>
<param name="inst">Pointer to Instruction structure</param>
<param name="labels">Array of label pointers</param>
<param name="imemin">Output file stream</param>
<returns>Returns 0 on success</returns> */
int encode_instruction(const char* line_in, Instruction* inst, char* labels[INSTR_MEM_DEPTH], FILE* imemin)
{
	const char* delim = " ,";
	INSTR_TYPE num = 0;
	int bit_index = INSTR_WIDTH_BITS;
	char hex[INSTR_WIDTH_CHARS + 1];
  char line[MAX_LINE_LENGTH];

  strcpy(line, line_in);

	// Parse and assign fields
	inst->opcode = fetch_opcode(strtok(line, delim));  // get opcode
	inst->rd = fetch_register(strtok(NULL, delim));     // get rd
	inst->rs = fetch_register(strtok(NULL, delim));     // get rs
	inst->rt = fetch_register(strtok(NULL, delim));     // get rt
	inst->rm = fetch_register(strtok(NULL, delim));     // get rm
	inst->immediate1 = parse_numeric_value(strtok(NULL, delim), labels);  // get imm1
	inst->immediate2 = parse_numeric_value(strtok(NULL, delim), labels);  // get imm2

	// Encode the instruction
	num |= ((INSTR_TYPE)(inst->opcode) << (bit_index -= OPCODE_SIZE_IN_BITS));
	num |= ((INSTR_TYPE)(inst->rd) << (bit_index -= RD_SIZE_IN_BITS));
	num |= ((INSTR_TYPE)(inst->rs) << (bit_index -= RS_SIZE_IN_BITS));
	num |= ((INSTR_TYPE)(inst->rt) << (bit_index -= RT_SIZE_IN_BITS));
	num |= ((INSTR_TYPE)(inst->rm) << (bit_index -= RM_SIZE_IN_BITS));
	num |= ((INSTR_TYPE)(inst->immediate1) << (bit_index -= IMM1_SIZE_IN_BITS)) & ((INSTR_TYPE)0xfff << bit_index);
	num |= ((INSTR_TYPE)(inst->immediate2) << (bit_index -= IMM2_SIZE_IN_BITS)) & ((INSTR_TYPE)0xfff << bit_index);

	// Convert the instruction to hexadecimal
	sprintf(hex, "%0*llX", INSTR_WIDTH_CHARS, num);  // zero-padded hex conversion

	// Write the instruction to the output file
	fwrite(hex, 1, INSTR_WIDTH_CHARS, imemin);
	fwrite("\n", 1, 1, imemin);  // Write newline character

	return 0;
}


/*
<summary>
Processes the current line to determine its type (instruction, label, comment, etc.)
</summary>
<param name="cur_line">Current line being processed.</param>
<param name="sInstruction">Pointer to the Instruction structure.</param>
<param name="label_addresses_lst">Array of label addresses.</param>
<param name="fImemin">File pointer for the output memory file.</param>
<returns>The type of the line (comment, label, instruction, etc.).</returns> */
Line_Type get_line_type(char* cur_line, Instruction* sInstruction, char* label_addresses_lst[INSTR_MEM_DEPTH], FILE* fImemin) {
	char cleaned_line[MAX_LINE_LENGTH];

	// Clean the line and get the cleaned length
	int cleaned_length = clean_and_trim_line(cur_line, cleaned_line);

	if (cleaned_length == 0) {
		// The line is a comment or empty
		return COMMENT;
	}

	// Check if the line contains a label
	if (has_label(cleaned_line)) {
		strtok(cleaned_line, ":");  // Extract the label part
		char* remaining_instruction = strtok(NULL, "");  // Get the remaining part (if any)

		// If there's more after the label, re-evaluate the remaining part for type
		return remaining_instruction ?
			get_line_type(remaining_instruction, sInstruction, label_addresses_lst, fImemin) :
			LABEL;
	}

	// Check if the line is a comment
	if (is_line_comment(cleaned_line)) {
		return COMMENT;
	}

	// Check if the line contains a pseudo-instruction
	if (is_pseudo_inst(cleaned_line)) {
		return PSEUDO;
	}

	// If none of the above, handle it as a regular instruction
	encode_instruction(cleaned_line, sInstruction, label_addresses_lst, fImemin);
	return REGULAR_INST;
}


/*
<summary>
Cleans the given line by removing trailing, leading, and extra spaces/tabs.
</summary>
<param name="line_to_fix">The original line that needs cleaning.</param>
<param name="line_fixed">The cleaned version of the line.</param>
<returns>The length of the cleaned line.</returns> */
int clean_and_trim_line(const char* line_to_fix, char* line_fixed) {
	int i = 0, j = 0;
	int spaced = 0;  // Tracks whether the last character was a space/tab
	int first_non_space_found = 0;

	// Iterate through the original line
	while (line_to_fix[i] != '\0') {
		char c = line_to_fix[i];

		if (c == '\t' || c == ' ') {
			if (first_non_space_found && !spaced) {
				line_fixed[j++] = ' ';
				spaced = 1;  // Entered a space state
			}
		}
		else {
			first_non_space_found = 1;
			line_fixed[j++] = c;
			spaced = 0;  // Exited space state
		}

		i++;
	}

	// Remove trailing space if it exists
	if (j > 0 && line_fixed[j - 1] == ' ') {
		j--;
	}
	line_fixed[j] = '\0';  // Null-terminate the cleaned string

	// Remove any occurrences of " ," (space before comma)
	char* ptr;
	while ((ptr = strstr(line_fixed, " ,")) != NULL) {
		memmove(ptr, ptr + 1, strlen(ptr));  // Shift the string to remove the space
	}

	return j;
}


/*
<summary>
Returns the enum value of an opcode based on the given string.
</summary>
<param name="opcode">The opcode string.</param>
<returns>The corresponding OpCode enum value.</returns> */
OpCode fetch_opcode(const char* opcode) {
	static const char* opcodes_str[] = {
		"add", "sub", "mac", "and", "or", "xor",
		"sll", "sra", "srl", "beq", "bne", "blt",
		"bgt", "ble", "bge", "jal", "lw", "sw",
		"reti", "in", "out", "halt"
	};
	const int num_opcodes = sizeof(opcodes_str) / sizeof(opcodes_str[0]);

	// Create a lowercase version of the opcode
	char lower_opcode[MAX_LINE_LENGTH];
	to_lowercase(opcode, lower_opcode);

	// Use a loop to find the matching opcode
	for (int i = 0; i < num_opcodes; i++) {
		if (strcmp(lower_opcode, opcodes_str[i]) == 0) {
			return (OpCode)i;
		}
	}

  return (OpCode)(-1); // Fix warnings
}



/*
<summary>
Returns the enum value of a register based on the given string.
</summary>
<param name="reg">The register string.</param>
<returns>The corresponding Register enum value.</returns> */
Register fetch_register(const char* reg_in)
{
	char reg[MAX_LINE_LENGTH]; // Assume MAX_REG_NAME is defined appropriately
	to_lowercase(reg_in, reg); // Converts 'reg' to lowercase and stores in 'reg'

	// Array of register names in lowercase to match the lowercase input
	const char* reg_names_str[] = {
		"$zero", "$imm1", "$imm2", "$v0", "$a0", "$a1", "$a2", "$t0",
		"$t1", "$t2", "$s0", "$s1", "$s2", "$gp", "$sp","$ra"
	};

	for (int i = 0; i < 16; i++) {
		if (strcmp(reg, reg_names_str[i]) == 0) {
			return (Register)i;
		}
	}
  return (Register)(-1); // Fix warnings
}

/*
<summary>
Converts every ASCII character in the input string to lowercase.
</summary>
<param name="input">The input string.</param>
<param name="output">The output string (lowercased).</param> */
void to_lowercase(const char* input, char* output) {
	if (!input || !output) {
		if (output) output[0] = '\0';  // Handle null input, ensure output is empty
		return;
	}

	int i = 0;
	while (input[i] != '\0') {
		output[i] = (char)tolower((unsigned char)input[i]);  // Convert to lowercase
		i++;
	}

	output[i] = '\0';  // Null-terminate the output string
}

/*
<summary>
returns a numeric value inside field, using labels as a map to labels indices
</summary>
<param name="field"></param>
<param name="label_addresses"></param>
<returns></returns> */
int parse_numeric_value(const char* field, char* label_addresses[INSTR_MEM_DEPTH]) {
	char temp[MAX_LINE_LENGTH];
	int result = 0;

	// Create lowercase copy of input
	for (int i = 0; field[i]; i++) {
		temp[i] = (char)tolower((unsigned char)field[i]);
		temp[i + 1] = '\0';
	}

	// Check for hex number
	if (strlen(temp) > 2 && temp[0] == '0' && (temp[1] == 'x' || temp[1] == 'x')) {
		char* endptr;
		result = (int)strtol(temp + 2, &endptr, 16);
		return result;
	}

	// Check for label
	if (isalpha((unsigned char)temp[0])) {
		int i = 0;
		while (i < INSTR_MEM_DEPTH) {
			if (label_addresses[i] != NULL) {
				char label_temp[MAX_LINE_LENGTH];
				int j;

				// Convert label to lowercase for comparison
				for (j = 0; label_addresses[i][j]; j++) {
					label_temp[j] = (char)tolower((unsigned char)label_addresses[i][j]);
				}
				label_temp[j] = '\0';

				if (strcmp(label_temp, temp) == 0) {
					return i;
				}
			}
			i++;
		}
	}

	// Handle decimal number
	char* endptr;
	result = (int)strtol(temp, &endptr, 10);
	return result;
}

/*
 * Extracts and processes a label from the given line of assembly code.
 * Handles both standalone labels and inline labels followed by instructions.
 * @param cur_cleaned_line Pointer to the line of code to process. Modified during processing.
 * @param label_and_inst Output parameter indicating if label is followed by an instruction.
 * @return Dynamically allocated string containing the lowercase label, or NULL if no label found.
 * Caller is responsible for freeing the returned memory. */
char* parse_label(char* cur_cleaned_line, BOOL* label_and_inst)
{
	char* str_Label = NULL;
	char* token = NULL;
	char working_copy[MAX_LINE_LENGTH];

	// Create a working copy to preserve original string during tokenization
	strncpy(working_copy, cur_cleaned_line, MAX_LINE_LENGTH - 1);
	working_copy[MAX_LINE_LENGTH - 1] = '\0';

	clean_and_trim_line(working_copy, working_copy);

	if (has_label(working_copy)) {
		// Extract the label portion before the colon
		token = strtok(working_copy, ":");
		if (token) {
			size_t len = strlen(token);
			// Allocate memory for label and initialize to zero
			str_Label = (char*)calloc(len + 1, sizeof(char));

			if (str_Label) {
				// Convert label to lowercase while copying
				size_t i;
				for (i = 0; i < len; i++) {
					str_Label[i] = (char)tolower((unsigned char)token[i]);
				}
				str_Label[i] = '\0';

				// Check if there's an instruction after the label
				token = strtok(NULL, ":");
				if (token) {
					char temp[MAX_LINE_LENGTH];
					strncpy(temp, token, MAX_LINE_LENGTH - 1);
					temp[MAX_LINE_LENGTH - 1] = '\0';
					clean_and_trim_line(temp, cur_cleaned_line);

					// Set flag if non-comment instruction follows
					if (!is_line_comment(cur_cleaned_line)) {
						*label_and_inst = TRUE;
					}
				}
			}
		}
	}
	return str_Label;
}


/*
 * Processes a .word pseudo-instruction and stores the value in memory.
 * Expected format: .word <address> <value>
 * Both address and value can be in decimal or hexadecimal format.
 * @param cur_line Input string containing the pseudo-instruction
 * @param memory_data Array representing memory where data will be stored
 * @note Assumes memory_data has sufficient space at the specified address */
void process_pseudo(const char* cur_line, int* memory_data) {
	char line_copy[MAX_LINE_LENGTH];
	char* token;
	Word pseudo;
	// Create a safe working copy of the input line
	strncpy(line_copy, cur_line, MAX_LINE_LENGTH - 1);
	line_copy[MAX_LINE_LENGTH - 1] = '\0';
	// Skip the .word instruction token
	token = strtok(line_copy, " \t");
	if (!token) return;
	// Parse the address parameter
	token = strtok(NULL, " \t");
	if (token) {
		pseudo.address = parse_numeric_value(token, NULL);
	}
	else {
		return;
	}
	// Parse the value parameter
	token = strtok(NULL, " \t");
	if (token) {
		pseudo.value = parse_numeric_value(token, NULL);
		// Store value in memory if address is valid
		if (pseudo.address >= 0) {
			memory_data[pseudo.address] = pseudo.value;
		}
	}
}


/*
 * Writes memory contents to an output file in hexadecimal format.
 * Each memory value is written as an 8-digit uppercase hexadecimal number
 * followed by a newline character.
 * @param output_file Pointer to the opened output file
 * @param output_data Array of memory values to write
 * @param line_num Number of lines (memory values) to write
 * @note File should be opened in binary write mode before calling
 * @note Function will stop writing and report error if any write operation fails */
void load_data_to_file(FILE* output_file, DATA* output_data, int line_num) {
	const char* format = "%08X\n";  // Format: 8-digit hex uppercase with newline
	int i = 0;

	// Validate input parameters
	if (!output_file || !output_data) {
		fprintf(stderr, "Invalid file or data pointer.\n");
		return;
	}

	// Write each memory value to file
	while (i < line_num) {
		if (fprintf(output_file, format, output_data[i]) < 0) {
			fprintf(stderr, "Error writing data at line %d.\n", i + 1);
			break;
		}
		i++;
	}

	// Ensure all data is written to disk
	fflush(output_file);
}

/*
 * Locates the last non-zero value in memory array
 * @returns Index of last non-zero element */
int find_last_non_zero(DATA* data_memory, int memory_size)
{
	int last_index = 0;

	// Validate input parameters
	if (!data_memory || memory_size <= 0) {
		return 0;
	}

	// Search from end to start for efficiency
	// Stop at first non-zero value found
	for (int i = memory_size - 1; i >= 0; i--) {
		if (data_memory[i] != 0) {
			last_index = i;
			break;  // Exit loop once found
		}
	}

	return last_index;
}

/*
 * Checks if line contains a valid label (ending with colon)
 * @returns TRUE if label found, FALSE otherwise */
BOOL has_label(const char* cur_line) {
	// Check for null pointer
	if (!cur_line) {
		return FALSE;
	}

	// Scan through line character by character
	while (*cur_line) {
		switch (*cur_line) {
		case '#':  // Comments invalidate labels
			return FALSE;
		case ':':  // Colon indicates valid label
			return TRUE;
		default:
			cur_line++;  // Move to next character
		}
	}

	// No label found in line
	return FALSE;
}

/*
 * Checks if line contains a pseudo-instruction (starts with dot)
 * @returns TRUE if pseudo-instruction found */
BOOL is_pseudo_inst(const char* cur_line) {
	// Check for null pointer
	if (!cur_line) {
		return FALSE;
	}

	// Skip leading whitespace
	while (*cur_line && isspace((unsigned char)*cur_line)) {
		cur_line++;
	}

	// Check if line starts with comment
	if (*cur_line == '#') {
		return FALSE;
	}

	// Check for dot indicating pseudo-instruction
	return *cur_line == '.';
}

/*
 * Checks if line is empty or contains only comment
 * @returns TRUE if line is comment or empty */
BOOL is_line_comment(const char* cleared_line) {
	// Check for null pointer
	if (!cleared_line) {
		return TRUE;
	}

	// Skip leading whitespace
	while (*cleared_line && isspace((unsigned char)*cleared_line)) {
		cleared_line++;
	}

	// Line is comment if it starts with # or is empty
	return (*cleared_line == '#' || *cleared_line == '\0');
}
