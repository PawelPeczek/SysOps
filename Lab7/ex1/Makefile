CC=gcc -Wall -std=gnu99
define BARBER_SRC
sources/barber/app/main.c sources/barber/utils/argument_parser.c \
sources/barber/utils/syncro_utils.c sources/global_utils/queue.c
endef
BARBER_NAME=barber
define CLIENT_SRC
sources/client/app/main.c sources/client/utils/argument_parser.c \
sources/client/utils/syncro_utils.c sources/global_utils/queue.c
endef
CLIENT_NAME=client
define SUPERV_SRC
sources/supervisor/app/main.c sources/supervisor/utils/argument_parser.c \
sources/supervisor/utils/process_supervision_utils.c
endef
SUPERV_NAME=supervisor

all:
	mkdir -p bin
	$(CC) $(BARBER_SRC) -o $(BARBER_NAME)
	$(CC) $(CLIENT_SRC) -o $(CLIENT_NAME)
	$(CC) $(SUPERV_SRC) -o $(SUPERV_NAME)
	make move

move:
	mv $(CLIENT_NAME) $(BARBER_NAME) $(SUPERV_NAME) bin
	rm -f *.o, *.out

clear:
	rm -r -f bin