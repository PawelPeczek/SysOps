CC=gcc -Wall -std=gnu99
FLAGS=-lm -pthread
OUTPUT_DIR=bin
PROGRAM_NAME=image_filtration

all: main.o argument_parser.o file_reader.o file_writer.o file_utils.o image_transformation_utils.o
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(FLAGS) main.o argument_parser.o file_reader.o file_writer.o file_utils.o \
			image_transformation_utils.o -o $(OUTPUT_DIR)/$(PROGRAM_NAME)
	rm -rf *.o

main.o:
	$(CC) -c ./sources/app/main.c

argument_parser.o:
	$(CC) -c ./sources/utils/argument_parser.c

file_reader.o:
	$(CC) -c ./sources/utils/file_reader.c

file_writer.o:
	$(CC) -c ./sources/utils/file_writer.c

file_utils.o:
	$(CC) -c ./sources/utils/file_utils.c

image_transformation_utils.o:
	$(CC) -c ./sources/utils/image_transformation_utils.c

clear:
	rm -rf bin
