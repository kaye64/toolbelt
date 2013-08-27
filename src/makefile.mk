SUBDIRS = src/jag src/crack_jhash

include $(addsuffix /makefile.mk, $(SUBDIRS))
