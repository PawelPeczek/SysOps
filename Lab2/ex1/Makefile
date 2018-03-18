CC=gcc -Wall
define MAIN_SRC
sources/app/main.c sources/utils/parser/argument_parser.c sources/utils/copy_utils/system_copy.c \
sources/utils/generation_utils/system_generate.c sources/utils/sort_utils/system_sort.c \
sources/utils/sort_utils/lib_sort.c sources/utils/copy_utils/lib_copy.c
endef
all:
	mkdir -p bin
	$(CC) $(MAIN_SRC) -o app
	make move

move:
	mv app bin
	rm -f *.o

clear:
	rm -r -f bin

tests:
	make all
	bash tets.sh