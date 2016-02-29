rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *, %,$2),$d))
ifeq ($(OS),Windows_NT)
	CHK_DIR_EXISTS = if not exist "$(strip $1)" mkdir "$(strip $1)"
	NUKE = rmdir /s /q
	COPY_DIR = xcopy $1 $2 /E /H /Y
	COPY = xcopy $1 $2
	FIX_PATH = $(subst /,\,$1)
	LIB_SUFFIX = .dll
else
	CHK_DIR_EXISTS = @test -d $1 || mkdir -p $1
	NUKE = rm -r $1
	COPY_DIR = cp -rv $1 $2
	COPY = cp $1 $2
	FIX_PATH = $1
	LIB_SUFFIX = .so
	REPORT = @echo -e "\e[4;1;37m$1\033[0m"
endif
PROJECT_DIR :=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
OBJ_DIR := $(PROJECT_DIR)obj
BIN_DIR := $(PROJECT_DIR)bin
SRC_DIR := $(PROJECT_DIR)src
EXAMPLE_DIR := $(PROJECT_DIR)examples

C_FILES := $(call rwildcard, $(SRC_DIR), *.c)
COMMON_C_FILES := $(call rwildcard, $(SRC_DIR)/common/, *.c)
COMMON_OBJ := $(patsubst $(SRC_DIR)/common/%.c,$(OBJ_DIR)/%.o,$(COMMON_C_FILES))
BINARIES := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(wildcard $(SRC_DIR)/*.c))

C_FLAGS += --std=gnu99 -O2 -pipe -march=native -lpthread -lmultiX -lutilities -lrt -I$(PROJECT_DIR)headers

all: binaries

binaries : $(BINARIES)

$(OBJ_DIR)/%.o : $(SRC_DIR)/common/%.c
	$(call REPORT,Building $@)
	$(call CHK_DIR_EXISTS, $(OBJ_DIR))
	gcc $(C_FLAGS) -c -o $@ $<

$(BIN_DIR)/% : $(SRC_DIR)/%.c $(COMMON_OBJ)
	$(call REPORT,Building $@)
	$(call CHK_DIR_EXISTS, $(BIN_DIR))
	gcc $(C_FLAGS) -o $@ $< $(COMMON_OBJ)

clean:
	$(call REPORT,Cleaning...)
	-$(NUKE) "$(BIN_DIR)" "$(OBJ_DIR)"
