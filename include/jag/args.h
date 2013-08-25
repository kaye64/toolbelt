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

#ifndef _JAG_H_
#define _JAG_H_

#include <stdbool.h>
#include <runite/util/list.h>

#define MODE_NONE 0
#define MODE_EXTRACT 1
#define MODE_LIST 2
#define MODE_CREATE 3

typedef struct input_file input_file_t;
typedef struct jag_args jag_args_t;

struct input_file {
	list_node_t node;
	char path[255];
};

struct jag_args {
	int mode; /* one of MODE_{EXTRACT,LIST,CREATE} */
	char archive[255];
	list_t input_files;
	bool verbose;
};

bool parse_args(jag_args_t* args, int argc, char** argv);
void print_help();
void print_usage();
void print_error(char* message, int status);
bool resolve_input_files(jag_args_t* args);

#endif /* _JAG_H_ */
