CC=gcc -Wall -std=gnu99
FLAGS=-lm -pthread
OUTPUT_DIR=bin
CL_NAME=client
SRV_NAME=server

all:
	make client
	make server

server: server.o server_parser.o server_cyclic_list.o server_hashtable.o server_utils.o
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(FLAGS) server.o server_parser.o server_cyclic_list.o server_hashtable.o server_utils.o \
	-o $(OUTPUT_DIR)/$(SRV_NAME)
	rm -rf *.o

server.o:
	$(CC) -c ./sources/app/server.c

server_parser.o:
	$(CC) -c ./sources/utils/server/server_parser.c

server_cyclic_list.o:
	$(CC) -c ./sources/utils/server/server_cyclic_list.c

server_hashtable.o:
	$(CC) -c ./sources/utils/server/server_hashtable.c

server_utils.o:
	$(CC) -c ./sources/utils/server/server_utils.c

client: client.o client_parser.o client_utils.o
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(FLAGS) client.o client_parser.o client_utils.o \
	-o $(OUTPUT_DIR)/$(CL_NAME)
	rm -rf *.o

client.o:
	$(CC) -c ./sources/app/client.c

client_parser.o:
	$(CC) -c ./sources/utils/client/client_parser.c

client_utils.o:
	$(CC) -c ./sources/utils/client/client_utils.c

clear:
	rm -rf bin