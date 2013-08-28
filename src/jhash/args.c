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

#include <jhash/args.h>

#include <stdio.h>
#include <argp.h>
#include <string.h>
#include <error.h>
#include <runite/file.h>

#define GROUP_OTHERS -1

#define OPTION_GEN_TABLE 'g'
#define OPTION_CRACK 'c'
#define OPTION_VERBOSE 'v'
#define OPTION_EXTD_CHARSET 'e'
#define OPTION_MAX_LEN 'l'
#define OPTION_DECIMAL 'd'
#define OPTION_HEXADECIMAL 'h'
#define OPTION_HEAP_SIZE 'H'

static error_t parse_opt(int key, char *arg, struct argp_state *state);

extern char* program_name;

char charset_std[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789\0"; /* i believe most (all?) production hashes are in this charset */
char charset_extd[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789\0";

static char const doc[] = "Brute forces a hash by means of a precalculated lookup table.\n\
\n\
Examples:\n\
  jhash -g lookup_table          # Generate a lookup table with standard options\n\
  jhash -c lookup_table de3bdc91 # Attempt to crack a hash using a given lookup table\n\
";

const struct argp_option options[] = {
	{ 0, 0, 0, 0, "Main operation mode:\n" },
	{ "gen-table", OPTION_GEN_TABLE, 0, 0, "Generate a lookup table" },
	{ "crack", OPTION_CRACK, 0, 0, "Attempt to crack a hash" },
	{ 0, 0, 0, 0, "Operation modifiers:\n" },
	{ "decimal", OPTION_DECIMAL, 0, 0, "Treat identifiers as decimal" },
	{ "hex", OPTION_HEXADECIMAL, 0, 0, "Treat identifiers as hexadecimal" },
	{ "extended", OPTION_EXTD_CHARSET, 0, 0, "Use the extended char set to generate a lookup table" },
	{ "max-length", OPTION_MAX_LEN, "length", 0, "Set the maximum hash string length" },
	{ "heap-size", OPTION_HEAP_SIZE, "megabytes", 0, "Set the heap size in megabytes" },
	{ 0, 0, 0, 0, "Other options:", GROUP_OTHERS },
	{ "verbose", OPTION_VERBOSE, 0, 0, "Enable verbose output", GROUP_OTHERS },
	{ 0 }
};

const struct argp parser = {
	.options = options,
	.parser = parse_opt,
	.args_doc = "[LOOKUP TABLE] [HASH]",
	.doc = doc,
	.children = NULL,
	.help_filter = NULL,
	.argp_domain = NULL
};

/**
 * Parses argv into args
 */
bool parse_args(jhash_args_t* args, int argc, char** argv)
{
	int next_arg = 0;
	error_t error = argp_parse(&parser, argc, argv, 0, &next_arg, args);
	if (error != 0) {
		return false;
	}

	/* verify arguments */
	if (args->mode == MODE_NONE) {
		print_error("no mode specified", EXIT_FAILURE);
	}

	if (strcmp(args->table_path, "") == 0) {
		print_error("no lookup table specified", EXIT_FAILURE);
	}

	if (args->max_len > 16) {
		print_error("maximum value of max length is 16", EXIT_FAILURE);
	}

	if (args->mode == MODE_CRACK) {
		switch (args->ident_mode) {
		case HASH_DECIMAL:
			args->target_hash = strtol(args->hash_str, NULL, 10);
			break;
		case HASH_HEXADECIMAL:
			args->target_hash = strtol(args->hash_str, NULL, 16);
			break;
		}
		if (args->target_hash == 0) {
			print_error("invalid or missing hash", EXIT_FAILURE);
		}
	}

	if (args->heap_mb == 0) {
		print_error("invalid heap size specified", EXIT_FAILURE);
	}

	if (args->mode == MODE_GEN_TABLE && args->max_len == 0) {
		print_error("invalid max length specified", EXIT_FAILURE);
	}

	return true;
}

/**
 * Prints the usage message
 */
void print_usage()
{
	argp_help(&parser, stderr, ARGP_HELP_STD_USAGE, program_name);
}

/**
 * Prints the help message
 */
void print_help()
{
	argp_help(&parser, stderr, ARGP_HELP_STD_HELP, program_name);
}

/**
 * Prints an error message and exits with status
 */
void print_error(char* message, int status)
{
	error(0, errno, message);
	argp_help(&parser, stderr, ARGP_HELP_STD_ERR, program_name);
	exit(status);
}

/**
 * argp's option parser
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	jhash_args_t* jhash_args = (jhash_args_t*)state->input;
	int new_mode = 0;
	switch (key) {
	case OPTION_GEN_TABLE:
		new_mode = MODE_GEN_TABLE;
		break;
	case OPTION_CRACK:
		new_mode = MODE_CRACK;
		break;
	case OPTION_DECIMAL:
		jhash_args->ident_mode = HASH_DECIMAL;
		break;
	case OPTION_HEXADECIMAL:
		jhash_args->ident_mode = HASH_HEXADECIMAL;
		break;
	case OPTION_VERBOSE:
		jhash_args->verbose = true;
		break;
	case OPTION_EXTD_CHARSET:
		jhash_args->charset = charset_extd;
		break;
	case OPTION_MAX_LEN:
		jhash_args->max_len = strtol(arg, NULL, 10);
		break;
	case OPTION_HEAP_SIZE:
		jhash_args->heap_mb = strtol(arg, NULL, 10);
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num == 0) { /* first arg = lookup table */
			strcpy(jhash_args->table_path, arg);
		} else if (state->arg_num == 1) { /* hash to crack */
			strcpy(jhash_args->hash_str, arg);
		} else {
			return ARGP_ERR_UNKNOWN;
		}
		break;
	}
	if (new_mode != 0) {
		if (jhash_args->mode != 0 && jhash_args->mode != new_mode) {
			print_error("multiple modes specified", EXIT_FAILURE);
			return 0;
		}
		jhash_args->mode = new_mode;
	}
	return 0;
}
