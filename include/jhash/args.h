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

#ifndef _JHASH_ARGS_H_
#define _JHASH_ARGS_H_

#include <stdbool.h>
#include <runite/util/list.h>
#include <runite/hash.h>

#define MODE_NONE 0
#define MODE_HASH 1
#define MODE_GEN_TABLE 2
#define MODE_CRACK 3

#define HASH_HEXADECIMAL 0
#define HASH_DECIMAL 1

typedef struct jhash_args jhash_args_t;

struct jhash_args {
	int mode; /* one of MODE_{GEN_TABLE,CRACK} */
	char table_path[255];
	char target_string[32];
	jhash_t target_hash;
	char* charset;
	int max_len;
	bool verbose;
	int ident_mode;
	unsigned int heap_mb;
};

bool parse_args(jhash_args_t* args, int argc, char** argv);
void print_help();
void print_usage();
void print_error(char* message, int status);

#endif /* _JHASH_ARGS_H_ */
