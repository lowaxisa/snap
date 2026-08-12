SHELL := /bin/bash
CC := gcc

# target file
f ?= main.c
# target dir
d ?= ./
# executable name
o ?= snap

C_FLAGS := -Wshadow -Wall -Wextra -ldl -rdynamic
C_SOURCES = $(d)/$(f) $(wildcard src/*.c)
C_OUT := .temp/$(o)
COMPILE = $(CC) $(C_SOURCES) -I include -o $(C_OUT) $(C_FLAGS)

MODULES_SOURCES = $(wildcard modules/*.c)
MODULES_SO = $(patsubst modules/%.c, .temp/%.so, $(MODULES_SOURCES))

.PHONY: all init modules compile run
all: init modules compile run

.temp/%.so: modules/%.c
	@mkdir -p .temp
	gcc -fPIC -shared $< -I include -o $@

modules: $(MODULES_SO)

init:
	@echo "--- snap build system ---"
	@mkdir -p .temp
	@echo
	@sleep 0.2

compile:
	@echo "> compiling..."
	$(COMPILE)
	@echo
	@sleep 0.2

run:
	@echo
	@for i in $$(seq 1 9); do \
		sub=$$(expr 10 - $$i); \
		echo -e "\033[2A"; \
		echo -ne "> running in $$sub\r"; \
		echo; \
		sleep 0.1; \
	done;

	@printf "\n\033[2A> running...   \r\n"
	@sleep 0.4
	@echo
	# you need the package time
	# cd .temp && /usr/bin/time -f "runtime: %E" ./$(o)
	cd .temp && ./$(o)
	@echo