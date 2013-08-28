/**
 *  This file is part of Gem.
 *
 *  Gem is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Gem is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Gem.  If not, see <http://www.gnu.org/licenses/\>.
 */

#include <stdio.h>
#include <err.h>
#include <string.h>
#include <jhash/args.h>

extern char charset_std[];
extern char charset_extd[];

char* program_name;
extern char* program_invocation_name;

jhash_args_t jhash_args = {
	.mode = MODE_NONE,
	.table_path = "",
	.hash_str = "",
	.target_hash = 0,
	.charset = charset_std,
	.max_len = 10,
	.heap_mb = 256,
	.verbose = false,
	.ident_mode = HASH_HEXADECIMAL
};

static void generate_table(const char* table_path, char* charset, int max_length, unsigned int heap_mb);
static void jhash_exit();

/**
 * Sets the program_name and program_invocation_name global
 */
static void set_program_name(char* program)
{
	if (strrchr(program, '/') != NULL) { /* get rid of any path in program */
		program = strrchr(program, '/')+1;
	}
	program_name = program;
	program_invocation_name = program;
}

int main(int argc, char** argv) {
	/* setup */
	set_program_name(argv[0]);

	if (atexit(jhash_exit) != 0) { /* ensure we clean up after ourselves */
		err(EXIT_FAILURE, "atexit() failed\n");
	}

	/* parse arguments */
	if (!parse_args(&jhash_args, argc, argv)) {
		print_usage();
		return EXIT_FAILURE;
	}

	switch (jhash_args.mode) {
	case MODE_GEN_TABLE:
		generate_table(jhash_args.table_path, jhash_args.charset, jhash_args.max_len, jhash_args.heap_mb);
		break;
	case MODE_CRACK:
		break;
	}

	return EXIT_SUCCESS;
}

typedef struct table_entry table_entry_t;
struct table_entry {
	jhash_t hash;
	char string[16];
};

static void flush_table_entries(FILE* fd, table_entry_t* entries, int num) {
	fwrite(entries, sizeof(table_entry_t), num, fd);
}

static void generate_table(const char* table_path, char* charset, int max_length, unsigned int heap_mb) {
	/* open the table path */
	FILE* fd = fopen(table_path, "w+");
	if (!fd) {
		char message[255];
		sprintf(message, "%s: unable to open table for writing", table_path);
		print_error(message, EXIT_FAILURE);
	}

	/* allocate memory to store the table in */
	size_t num_entries = (size_t)(heap_mb*1024*1024)/(size_t)(sizeof(table_entry_t));
	table_entry_t* entries = (table_entry_t*)malloc((size_t)sizeof(table_entry_t)*num_entries);
	size_t cur_entry = 0;
	memset(entries, 0, (size_t)sizeof(table_entry_t)*num_entries);

	/* Generate the table */
	for (int length = 1; length <= max_length; length++) {
		/* Adapted from 'Jerome' @ stackoverflow (http://goo.gl/dvVArI) */
		char* buffer[length];
		int i;
		for (i = 0; i < length; i++) {
			buffer[i] = &charset[0];
		}
		do {
			/* store the previous result */
			for (i = 0; i < length; i++) {
				entries[cur_entry].string[i] = *buffer[i];
			}
			entries[cur_entry].hash = jagex_hash(entries[cur_entry].string);

			/* If necessary flush to file, reset our buffer */
			if (++cur_entry > num_entries) {
				flush_table_entries(fd, entries, num_entries);
				cur_entry = 0;
				memset(entries, 0, (size_t)sizeof(table_entry_t)*num_entries);
			}

			/* calculate the next permutation */
			for(i = 0; i < length && *(++buffer[i]) == '\0'; i++) {
				buffer[i] = &charset[0];
			}
		} while (i < length);
	}

	/* flush and exit */
	flush_table_entries(fd, entries, cur_entry);

	free(entries);
	fclose(fd);
}

static void jhash_exit()
{

}
