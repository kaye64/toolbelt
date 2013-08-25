CFLAGS = -g -std=gnu99 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Lrunite/
INCLUDE_DIRS = -Iinclude/ -I../runite/include/
LIB_DIRS = -L../runite/
LIBS = -lrunite -lbz2
SUBDIRS = src/
RUNITE_PATH = ../runite/librunite.a
BIN_DIR = bin

TARGETS :=
OBJECTS :=

include $(addsuffix /makefile.mk, $(SUBDIRS))

all: $(TARGETS)

$(TARGETS): $(BIN_DIR)

$(BIN_DIR):
	-mkdir -p $(BIN_DIR)

../runite/librunite.a:
	make -C ../runite

%.o: %.c
	gcc -c $(CFLAGS) $(LIBS) $(INCLUDE_DIRS) -o $@ $^

clean:
	-rm -f $(TARGETS) $(OBJECTS)

.PHONY: all clean
