JAG_OUT = $(BIN_DIR)/jag
JAG_OBJECTS = $(addprefix src/jag/,jag.o args.o)

TARGETS += $(JAG_OUT)
OBJECTS += $(JAG_OBJECTS)

$(JAG_OUT): $(RUNITE_PATH) $(JAG_OBJECTS)
	gcc $(CFLAGS) -o $@ $(JAG_OBJECTS) $(LIBS) $(LIB_DIRS)
