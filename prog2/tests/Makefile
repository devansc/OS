SHELL = /bin/bash

SUBDIRS = $(wildcard [0-9][0-9]*)

LIBDIR  = ../lib

PUBDIRS = $(LIBDIR) ../testlib

PUBHOST = unix5.csc.calpoly.edu

all:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir LIBDIR=../$(LIBDIR) $@; \
		done;\
	fi

prog:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir LIBDIR=../$(LIBDIR) $@; \
		done;\
	fi

clean:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir LIBDIR=../$(LIBDIR) $@; \
		done;\
	fi

allclean:
	@if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			$(MAKE) -C $$dir LIBDIR=../$(LIBDIR) $@; \
		done;\
	fi

tryme:
	if [ -n "$(SUBDIRS)" ]; then \
	        for dir in $(SUBDIRS) ; do \
			echo "this is $$dir"; \
		done;\
	fi

pub:	
	rsync -rLtpvz $(PUBDIRS) pn-cs453@$(PUBHOST):lib/asgn2/

