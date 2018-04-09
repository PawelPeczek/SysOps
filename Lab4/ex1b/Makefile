CC=gcc -Wall
define MAIN_SRC
sources/app/main.c sources/utils/signal_utils.c
endef
SCRIPT=date_inf_loop.sh
APP_NAME=app
all:
	$(CC) $(MAIN_SRC) -o $(APP_NAME)
	make move

move:
	rm -f *.o

clear:
	rm -r -f $(APP_NAME)
