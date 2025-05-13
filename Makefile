# Makefile to build and clean subdirectory Makefiles for lib and kmodule

SUBDIRS := lib kmodule

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done