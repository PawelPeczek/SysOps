CC=gcc -Wall -std=gnu99
FLAGS=-lm -pthread
OUTPUT_DIR=bin
PROGRAM_NAME=prod_cons

all: main.o argument_parser.o prod_cons_problem_solver.o
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(FLAGS) main.o argument_parser.o prod_cons_problem_solver.o \
	-o $(OUTPUT_DIR)/$(PROGRAM_NAME)
	rm -rf *.o

main.o:
	$(CC) -c ./sources/app/main.c

argument_parser.o:
	$(CC) -c ./sources/utils/argument_parser.c

prod_cons_problem_solver.o:
	$(CC) -c ./sources/utils/prod_cons_problem_solver.c

clear:
	rm -rf bin