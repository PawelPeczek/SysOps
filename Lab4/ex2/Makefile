CC=gcc -Wall
define MAIN_SRC
sources/app/main.c sources/utils/signals_utils.c \
sources/utils/queue.c sources/utils/argument_parser.c \
sources/headers/program_input.c
endef
MAIN_NAME=parent_program
CHILD_SRC=sources/app/child.c
CHILD_NAME=child_program

all:
	$(CC) $(MAIN_SRC) -o $(MAIN_NAME)
	$(CC) $(CHILD_SRC) -o $(CHILD_NAME)

clear:
	rm -f $(MAIN_NAME) $(CHILD_NAME) *.o
