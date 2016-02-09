rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *, %,$2),$d))
ifeq ($(OS),Windows_NT)
	CHK_DIR_EXISTS = if not exist "$(strip $1)" mkdir "$(strip $1)"
	NUKE = rmdir /s /q
	COPY_DIR = xcopy $1 $2 /E /H /Y
	COPY = xcopy $1 $2
	FIX_PATH = $(subst /,\,$1)
	LIB_SUFFIX = .dll
else
	CHK_DIR_EXISTS = test -d $1 || mkdir -p $1
	NUKE = rm -r $1
	COPY_DIR = cp -rv $1 $2
	COPY = cp $1 $2
	FIX_PATH = $1
	LIB_SUFFIX = .so
endif

PROJECT_DIR :=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
OBJ_DIR := $(PROJECT_DIR)obj
BIN_DIR := $(PROJECT_DIR)bin
SRC_DIR := $(PROJECT_DIR)src

C_FILES := $(call rwildcard, $(SRC_DIR), *.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_FILES))

C_FLAGS += --std=gnu99 -flto -O3 -pipe -march=native -I$(PROJECT_DIR)headers
CC = mpicc

main: $(SRC_DIR)/main.c $(OBJ_DIR)/matrix.o
	$(call CHK_DIR_EXISTS, $(BIN_DIR))
	$(CC) $(C_FLAGS) -o $(BIN_DIR)/main $(SRC_DIR)/main.c $(OBJ_DIR)/matrix.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(call CHK_DIR_EXISTS, $(OBJ_DIR))
	$(CC) $(C_FLAGS) -o $@ -c $<

all : main

clean:
	$(NUKE) "$(BIN_DIR)" "$(OBJ_DIR)"
