SHELL = /bin/sh

SUBDIRS =  $(wildcard [0-9]*)

all:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir $@; \
		done;\
	fi

prog:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir $@; \
		done;\
	fi

clean:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir $@; \
		done;\
	fi

allclean:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir $@; \
		done;\
	fi

