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
#include <errno.h>
#include <sys/stat.h>
#include <libgen.h>
#include <runite/archive.h>
#include <runite/file.h>

#include <jag/args.h>

char* program_name;
extern char* program_invocation_name;

jag_args_t jag_args = {
	.mode = MODE_NONE,
	.archive = "",
	.verbose = false,
	.ident_mode = IDENT_HEXADECIMAL
};

static void jag_extract(char* archive_path);
static void jag_list(char* archive_path);
static void jag_create(char* archive_path, list_t* input_files);
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
		jag_extract(jag_args.archive);
		break;
	case MODE_LIST:
		jag_list(jag_args.archive);
		break;
	case MODE_CREATE:
		jag_create(jag_args.archive, &jag_args.input_files);
		break;
	}

	return EXIT_SUCCESS;
}

static void format_identifier(jhash_t identifier, char* out)
{
	switch (jag_args.ident_mode) {
	case IDENT_DECIMAL:
		sprintf(out, "%i", identifier);
		break;
	case IDENT_STRING:
	case IDENT_HEXADECIMAL:
		sprintf(out, "%x", identifier);
		break;
	}
}

/**
 * Extracts the contents of an archive
 */
static void jag_extract(char* archive_path)
{
	/* create the destination directory */
	char dir_name[255];
	strcpy(dir_name, basename(archive_path));
	if (strrchr(dir_name, '.') != NULL) { /* get rid of any extension */
		char* idx = strrchr(dir_name, '.');
		*idx = '\0';
	}
	int ret = mkdir(dir_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (ret != 0 && errno != EEXIST) {
		print_error("unable to create destination directory", EXIT_FAILURE);
	}

	/* decompress the archive */
	file_t archive_file;
	if (!file_read(&archive_file, archive_path)) {
		printf("%s\n", archive_path);
		print_error("unable to read archive", EXIT_FAILURE);
	}
	archive_t* archive = object_new(archive);
	if (!archive_decompress(archive, &archive_file)) {
		object_free(archive);
		free(archive_file.data);
		print_error("unable to decompress archive", EXIT_FAILURE);
	}
	free(archive_file.data);

	/* extract the contents to disk */
	archive_file_t* file;
	list_for_each(&archive->files) {
		list_for_get(file);
		char file_name[255];
		char file_path[255];
		format_identifier(file->identifier, file_name);
		file_path_join(dir_name, file_name, file_path);
		
		/* write the file */
		FILE* fd = fopen(file_path, "w+");
		if (fd == NULL) {
			char message[255];
			sprintf(message, "%s: unable to open file for writing", file_path);
			print_error(message, EXIT_FAILURE);
		}
		size_t written = fwrite(file->file.data, 1, file->file.length, fd);
		fclose(fd);
		if (written != file->file.length) {
			char message[255];
			sprintf(message, "%s: unable to write entire file", file_path);
			print_error(message, EXIT_FAILURE);			
		}

		if (jag_args.verbose) {
			printf("Extracted %s\n", file_path);
		}
	}

	object_free(archive);	
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

	if (jag_args.verbose) {
		printf("Identifier\tSize\n");
	}

	archive_file_t* file;
	list_for_each(&archive->files) {
		list_for_get(file);
		char fmt_identifier[20];
		format_identifier(file->identifier, fmt_identifier);
		printf("%-11s\t%zu\n", fmt_identifier, file->file.length);
	}
	
	if (jag_args.verbose) {
		printf("%d files\n", archive->num_files);
	}

	object_free(archive);
}

/**
 * Creates an archive from a list of input files
 */
static void jag_create(char* archive_path, list_t* input_files)
{
	archive_t* archive = object_new(archive);
	input_file_t* in_file;
	list_for_each(input_files) {
		list_for_get(in_file);

		/* figure out the identifier */
		jhash_t identifier = 0;
		char file_name[255];
		strcpy(file_name, basename(in_file->path));
		switch (jag_args.ident_mode) {
		case IDENT_DECIMAL:
			identifier = strtol(file_name, NULL, 10);
			break;
		case IDENT_HEXADECIMAL:
			 identifier = strtol(file_name, NULL, 16);
			 break;
		 case IDENT_STRING:
			 identifier = jagex_hash(file_name);
			 break;
		 }
		 if (identifier == 0) {
			 char message[100];
			 sprintf(message, "%s: unable to determine identifier (perhaps you meant to specify --string?)", in_file->path);
			 print_error(message, EXIT_FAILURE);
		 }

		 /* read the file from disk */
		 file_t file;
		 if (!file_read(&file, in_file->path)) {
			 char message[100];
			 sprintf(message, "%s: unable to read", in_file->path);
			 print_error(message, EXIT_FAILURE);
		 }

		 /* add the file to our archive */
		 archive_file_t* archive_file = archive_add_file(archive, identifier, &file);
		 if (archive_file == NULL) {
			 char message[100];
			 sprintf(message, "%s: unable to add file", in_file->path); /* probably a name collision */
			 print_error(message, EXIT_FAILURE);
		 }

		 if (jag_args.verbose) {
			 char fmt_identifier[20];
			 format_identifier(identifier, fmt_identifier);
			 printf("Added %s as %s\n", file_name, fmt_identifier);
		 }

		 free(file.data);
	}

	/* do the compression */
	file_t out_file;
	if (!archive_compress(archive, &out_file, ARCHIVE_COMPRESS_FILE)) {
		print_error("unable to compress archive", EXIT_FAILURE);
	}

	/* write the file out */
	if (!file_write(&out_file, archive_path)) {
		char message[100];
		sprintf(message, "%s: unable to write archive", archive_path);
		print_error(message, EXIT_FAILURE);
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
