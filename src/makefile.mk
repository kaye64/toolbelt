SUBDIRS = src/jag

include $(addsuffix /makefile.mk, $(SUBDIRS))
