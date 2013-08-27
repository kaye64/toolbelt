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

#include <crack_jhash/args.h>

char* program_name;
extern char* program_invocation_name;

crack_args_t crack_args = {
	.mode = MODE_NONE,
	.table_path = "",
	.hash_str = "",
	.target_hash = 0,
	.charset = charset_std,
	.max_len = 10,
	.verbose = false,
	.ident_mode = HASH_HEXADECIMAL
};

static void crack_exit();

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

	if (atexit(crack_exit) != 0) { /* ensure we clean up after ourselves */
		err(EXIT_FAILURE, "atexit() failed\n");
	}

	/* parse arguments */
	if (!parse_args(&jag_args, argc, argv)) {
		print_usage();
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

static void crack_exit()
{
	
}
