CC=gcc -Wall
LIB_NAME=blockalloc
MAIN_NAME=main
LIB_PATH=sources/lib/utils/
LIB_SRC=$(LIB_PATH)$(LIB_NAME)_static.c $(LIB_PATH)$(LIB_NAME)_dynamic.c $(LIB_PATH)$(LIB_NAME)_shared.c
LIB_OBJ_NAMES=$(LIB_NAME)_static.o $(LIB_NAME)_dynamic.o $(LIB_NAME)_shared.o
MAIN_SRC=sources/app/$(MAIN_NAME).c
APP_NAME=test-app
UTILS_SRC=sources/app/utils/
DYN_UT=dynamic_utils
ST_UT=static_utils
SH_UT=shared_utils
APP_LINK_OBS=$(MAIN_NAME).o $(DYN_UT).o $(ST_UT).o $(SH_UT).o
APP_COMP_SRCS=$(MAIN_SRC) $(UTILS_SRC)$(DYN_UT).c $(UTILS_SRC)$(ST_UT).c $(UTILS_SRC)$(SH_UT).c

all:
	make static shared dynamic

allO:
	make staticO sharedO dynamicO

allO1:
	make staticO1 sharedO1 dynamicO1

allO2:
	make staticO2 sharedO2 dynamicO2

allO3:
	make staticO3 sharedO3 dynamicO3

allOs:
	make staticOs sharedOs dynamicOs

static:
	$(CC) -c $(LIB_SRC)
	ar rcs $(LIB_NAME).a $(LIB_OBJ_NAMES)
	$(CC) -c $(APP_COMP_SRCS)
	$(CC) $(APP_LINK_OBS) $(LIB_NAME).a -o $(APP_NAME)-static
	make move-static

shared:
	$(CC) -fPIC -c $(LIB_SRC)
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-shared $(APP_COMP_SRCS) -l$(LIB_NAME)
	make move-shared

dynamic:
	$(CC) -fPIC -c $(LIB_SRC)
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -ldl -D DYNAMIC -o $(APP_NAME)-dynamic $(APP_COMP_SRCS)
	make move-shared

staticO:
	$(CC) -c $(LIB_SRC) -O
	ar rcs $(LIB_NAME).a $(LIB_OBJ_NAMES)
	$(CC) -c $(APP_COMP_SRCS) -O
	$(CC) $(APP_LINK_OBS) $(LIB_NAME).a -o $(APP_NAME)-static
	make move-static

sharedO:
	$(CC) -fPIC -c $(LIB_SRC) -O
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-shared $(APP_COMP_SRCS) -l$(LIB_NAME) -O
	make move-shared

dynamicO:
	$(CC) -fPIC -c $(LIB_SRC) -O
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -ldl -D DYNAMIC -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-dynamic $(APP_COMP_SRCS) -l$(LIB_NAME) -O
	make move-shared

staticO1:
	$(CC) -c $(LIB_SRC) -O1
	ar rcs $(LIB_NAME).a $(LIB_OBJ_NAMES)
	$(CC) -c $(APP_COMP_SRCS) -O1
	$(CC) $(APP_LINK_OBS) $(LIB_NAME).a -o $(APP_NAME)-static
	make move-static

sharedO1:
	$(CC) -fPIC -c $(LIB_SRC) -O1
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-shared $(APP_COMP_SRCS) -l$(LIB_NAME) -O1
	make move-shared

dynamicO1:
	$(CC) -fPIC -c $(LIB_SRC) -O1
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -ldl -D DYNAMIC -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-dynamic $(APP_COMP_SRCS) -l$(LIB_NAME) -O1
	make move-shared

staticO2:
	$(CC) -c $(LIB_SRC) -O2
	ar rcs $(LIB_NAME).a $(LIB_OBJ_NAMES)
	$(CC) -c $(APP_COMP_SRCS) -O2
	$(CC) $(APP_LINK_OBS) $(LIB_NAME).a -o $(APP_NAME)-static
	make move-static

sharedO2:
	$(CC) -fPIC -c $(LIB_SRC) -O2
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-shared $(APP_COMP_SRCS) -l$(LIB_NAME) -O2
	make move-shared

dynamicO2:
	$(CC) -fPIC -c $(LIB_SRC) -O2
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -ldl -D DYNAMIC -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-dynamic $(APP_COMP_SRCS) -l$(LIB_NAME) -O2
	make move-shared

staticO3:
	$(CC) -c $(LIB_SRC) -O3
	ar rcs $(LIB_NAME).a $(LIB_OBJ_NAMES)
	$(CC) -c $(APP_COMP_SRCS) -O3
	$(CC) $(APP_LINK_OBS) $(LIB_NAME).a -o $(APP_NAME)-static
	make move-static

sharedO3:
	$(CC) -fPIC -c $(LIB_SRC) -O3
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-shared $(APP_COMP_SRCS) -l$(LIB_NAME) -O3
	make move-shared

dynamicO3:
	$(CC) -fPIC -c $(LIB_SRC) -O3
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -ldl -D DYNAMIC -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-dynamic $(APP_COMP_SRCS) -l$(LIB_NAME) -O3
	make move-shared

staticOs:
	$(CC) -c $(LIB_SRC) -Os
	ar rcs $(LIB_NAME).a $(LIB_OBJ_NAMES)
	$(CC) -c $(APP_COMP_SRCS) -Os
	$(CC) $(APP_LINK_OBS) $(LIB_NAME).a -o $(APP_NAME)-static
	make move-static

sharedOs:
	$(CC) -fPIC -c $(LIB_SRC) -Os
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-shared $(APP_COMP_SRCS) -l$(LIB_NAME) -Os
	make move-shared

dynamicOs:
	$(CC) -fPIC -c $(LIB_SRC) -Os
	$(CC) -fPIC -shared -o lib$(LIB_NAME).so $(LIB_OBJ_NAMES)
	$(CC) -ldl -D DYNAMIC -Wl,-rpath=$(shell pwd)/bin -L. -o $(APP_NAME)-dynamic $(APP_COMP_SRCS) -l$(LIB_NAME) -Os
	make move-shared

move-static:
	mkdir -p bin
	rm *.a
	make move

move-shared:
	mkdir -p bin
	mv *.so bin
	make move

move:
	mkdir -p bin
	rm *.o
	mv $(APP_NAME)-* bin 2>/dev/null

clear:
	rm -f -r bin
