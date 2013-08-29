JHASH_OUT = $(BIN_DIR)/jhash
JHASH_OBJECTS = $(addprefix src/jhash/,jhash.o args.o)

TARGETS += $(JHASH_OUT)
OBJECTS += $(JHASH_OBJECTS)

$(JHASH_OUT): $(RUNITE_PATH) $(JHASH_OBJECTS)
	gcc $(CFLAGS) -o $@ $(JHASH_OBJECTS) $(LIBS) $(LIB_DIRS)
