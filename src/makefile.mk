SUBDIRS = src/jag src/jhash

include $(addsuffix /makefile.mk, $(SUBDIRS))
