CC=gcc -Wall
define MASTER_SRC
sources/app/master.c sources/utils/master_arg_parser.c sources/utils/master_utils.c
endef
define SLAVE_SRC
sources/app/slave.c sources/utils/slave_arg_parser.c sources/utils/slave_utils.c
endef
MASTER_NAME=master
SLAVE_NAME=slave

all:
	mkdir -p bin
	$(CC) $(MASTER_SRC) -o $(MASTER_NAME)
	$(CC) $(SLAVE_SRC) -o $(SLAVE_NAME)
	make move

move:
	mv $(MASTER_NAME) $(SLAVE_NAME) bin
	rm -f *.o, *.out

clear:
	rm -r -f bin