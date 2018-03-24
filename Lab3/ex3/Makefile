CC=gcc -Wall
define MAIN_SRC
sources/app/main.c sources/utils/argument_parser.c sources/utils/file_reader.c \
sources/utils/interpreter.c
endef
INF_REC_SRC = sources/app/inf.c 
all:
	mkdir -p bin
	$(CC) $(MAIN_SRC) -o app
	make move

move:
	mv app bin
	rm -f *.o

clear:
	rm -r -f bin

inf-rec-prog-cmp:
	mkdir -p bin
	$(CC) $(INF_REC_SRC) -o bad-app
	mv bad-app bin
	rm -f *.o