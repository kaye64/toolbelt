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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <error.h>
#include <sys/stat.h>
#include <runite/archive.h>

#include <jag/args.h>

char* program_name;
extern char* program_invocation_name;

jag_args_t jag_args = {
	.mode = MODE_NONE,
	.archive = ""
};

static void jag_list(char* archive);
static void jag_exit();

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

/**
 * main
 */
int main(int argc, char** argv)
{
	/* setup */
	set_program_name(argv[0]);

	if (atexit(jag_exit) != 0) { /* ensure we clean up after ourselves */
		err(EXIT_FAILURE, "atexit() failed\n");
	}

	object_init(list, &jag_args.input_files);

	/* parse arguments */
	if (!parse_args(&jag_args, argc, argv)) {
		print_usage();
		return EXIT_FAILURE;
	}

	/* verify arguments */
	int num_inputs = list_count(&jag_args.input_files);
	if (jag_args.mode == MODE_NONE) {
		print_error("no mode specified", EXIT_FAILURE);
	}

	if (strcmp(jag_args.archive, "") == 0) {
		print_error("no archive specified", EXIT_FAILURE);
	}

	if ((jag_args.mode == MODE_EXTRACT || jag_args.mode == MODE_LIST) && num_inputs > 0) {
		print_error("unnecessary input files specified", EXIT_FAILURE);
	}

	if (jag_args.mode == MODE_CREATE && num_inputs == 0) {
		print_error("no input files specified", EXIT_FAILURE);
	}

	if (!resolve_input_files(&jag_args)) {
		print_error("unable to resolve input files", EXIT_FAILURE);
	}

	if (jag_args.mode == MODE_EXTRACT || jag_args.mode == MODE_LIST) {
		struct stat fstat;
		if (stat(jag_args.archive, &fstat) != 0) {
			char message[255];
			sprintf(message, "%s: Cannot stat", jag_args.archive);
			print_error(message, EXIT_FAILURE);
		}
	}

	/* do the work.. */
	switch (jag_args.mode) {
	case MODE_EXTRACT:

		break;
	case MODE_LIST:
		jag_list(jag_args.archive);
		break;
	case MODE_CREATE:

		break;
	}

	return EXIT_SUCCESS;
}

/**
 * Lists the contents of an archive
 */
static void jag_list(char* archive_path)
{
	file_t archive_file;
	if (!file_read(&archive_file, archive_path)) {
		print_error("unable to read archive", EXIT_FAILURE);
	}
	archive_t* archive = object_new(archive);
	if (!archive_decompress(archive, &archive_file)) {
		object_free(archive);
		free(archive_file.data);
		print_error("unable to decompress archive", EXIT_FAILURE);
	}
	free(archive_file.data);

	archive_file_t* file;
	list_for_each(&archive->files) {
		list_for_get(file);
		printf("0x%x\t%zu\n", file->identifier, file->file.length);
	}

	object_free(archive);
}

/**
 * Called on exit
 */
static void jag_exit()
{
	object_free(&jag_args.input_files);
}
