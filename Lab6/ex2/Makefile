CC=gcc -Wall -std=gnu99
define SERVER_SRC
sources/server/app/main.c sources/server/utils/communication_utils.c \
sources/server/utils/message_utils.c
endef
SERVER_NAME=server
define CLIENT_SRC
sources/client/app/main.c sources/client/utils/communication_utils.c
endef
CLIENT_NAME=client

all:
	mkdir -p bin
	$(CC) $(SERVER_SRC) -o $(SERVER_NAME) -lrt
	$(CC) $(CLIENT_SRC) -o $(CLIENT_NAME) -lrt
	make move

move:
	mv $(CLIENT_NAME) $(SERVER_NAME) bin
	rm -f *.o, *.out

clear:
	rm -r -f bin