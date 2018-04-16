CC=gcc -Wall
define MAIN_SRC
sources/app/main.c sources/utils/argument_parser.c sources/utils/file_reader.c \
sources/utils/interpreter.c 
endef
all:
	mkdir -p bin
	$(CC) $(MAIN_SRC) -o app
	make move

move:
	mv app bin
	rm -f *.o, *.out

clear:
	rm -r -f bin