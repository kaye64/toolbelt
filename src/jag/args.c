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

#include <jag/args.h>

#include <stdio.h>
#include <argp.h>
#include <dirent.h>
#include <string.h>
#include <error.h>
#include <sys/stat.h>
#include <runite/file.h>

#define GROUP_OPERATIONS 0

#define OPTION_EXTRACT 'x'
#define OPTION_LIST 'l'
#define OPTION_CREATE 'c'

static error_t parse_opt(int key, char *arg, struct argp_state *state);

extern char* program_name;

static char const doc[] = "Operates on archives in the jagex bz2 container format ('jag')\n\
\n\
Examples:\n\
  jag -c archive.jag foo bar  # Create archive.jag from files foo and bar.\n\
  jag -l archive.jag          # List all files in archive.jag.\n\
  jag -x archive.jag          # Extract all files from archive.jag.\n";

const struct argp_option options[] = {
	{ 0, 0, 0, 0, "Main operation mode:", GROUP_OPERATIONS },
	{ "extract", OPTION_EXTRACT, 0, 0, "Extract a given archive", GROUP_OPERATIONS },
	{ "list", OPTION_LIST, 0, 0, "List the contents of a given archive", GROUP_OPERATIONS },
	{ "create", OPTION_CREATE, 0, 0, "Create an archive from the given input files", GROUP_OPERATIONS },
	{ 0 }
};

const struct argp parser = {
	.options = options,
	.parser = parse_opt,
	.args_doc = "[ARCHIVE] [FILE]...",
	.doc = doc,
	.children = NULL,
	.help_filter = NULL,
	.argp_domain = NULL
};

/**
 * Parses argv into args
 */
bool parse_args(jag_args_t* args, int argc, char** argv)
{
	int next_arg = 0;
	error_t error = argp_parse(&parser, argc, argv, 0, &next_arg, args);
	if (error != 0) {
		return false;
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
	jag_args_t* jag_args = (jag_args_t*)state->input;
	int new_mode = 0;
	switch (key) {
	case OPTION_EXTRACT:
		new_mode = MODE_EXTRACT;
		break;
	case OPTION_LIST:
		new_mode = MODE_LIST;
		break;
	case OPTION_CREATE:
		new_mode = MODE_CREATE;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num == 0) { /* first arg = archive */
			strcpy(jag_args->archive, arg);
		} else if (state->arg_num > 0) { /* input files */
			input_file_t* input_file = (input_file_t*)malloc(sizeof(input_file_t));
			strcpy(input_file->path, arg);
			list_push_back(&jag_args->input_files, &input_file->node);
		}
		break;
	}
	if (new_mode != 0) {
		if (jag_args->mode != 0 && jag_args->mode != new_mode) {
			print_error("multiple modes specified", EXIT_FAILURE);
			return 0;
		}
		jag_args->mode = new_mode;
	}
	return 0;
}

/**
 * Iterates a list of files, recursing into subdirectories
 */
static bool expand_directory(list_t* files, input_file_t* directory)
{
	char* base_dir = directory->path;

	list_node_t* insert_point = &directory->node;

	DIR* dir = opendir(base_dir);
	if (!dir) {
		return false;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		input_file_t* input_file = (input_file_t*)malloc(sizeof(input_file_t));
		file_path_join(base_dir, entry->d_name, input_file->path);
		list_insert_after(files, insert_point, &input_file->node);
		insert_point = &input_file->node;
	}

	list_erase(files, &directory->node);

	closedir(dir);
	return true;
}

/**
 * Verifies all input files exist, and recurses into subdirectories
 */
bool resolve_input_files(jag_args_t* args)
{
	bool unresolved_paths = true;
	while (unresolved_paths) {
		unresolved_paths = false;
		input_file_t* file;
		list_for_each(&args->input_files) {
			list_for_get(file);
			struct stat fstat;
			if (stat(file->path, &fstat) != 0) {
				char message[255];
				sprintf(message, "%s: Cannot stat", file->path);
				print_error(message, EXIT_FAILURE);
				return false;
			}

			if (fstat.st_mode & S_IFDIR) { /* path is a directory */
				expand_directory(&args->input_files, file);
				unresolved_paths = true;
				break;
			}
		}
	}
	return true;
}
