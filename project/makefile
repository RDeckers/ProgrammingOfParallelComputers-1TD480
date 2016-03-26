rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *, %,$2),$d))
directories=$(sort $(dir $(wildcard $/*)))
ifeq ($(OS),Windows_NT)
FIX_PATH = $(subst /,\,$1)
	REPORT = @echo $1
	CHK_DIR_EXISTS = if not exist "$(strip $1)" mkdir "$(strip $1)"
	NUKE = rmdir /s /q
	COPY_DIR = xcopy $(call FIX_PATH,$1 $2) /E /H /Y
	COPY_CONTENT = xcopy /s /Y $(call FIX_PATH,$1 $2)
	COPY = xcopy $(call FIX_PATH,$1 $2) /Y
	INSTALL_LIB_DIR := Z:/lib/
	INSTALL_BIN_DIR := Z:/bin/
	INSTALL_INCLUDE_DIR := Z:/include/
	LIB_SUFFIX =.dll
	OPENCL_HEADERS :=$(call rwildcard, Z:/include/CL/, *.h)
else
	REPORT = @echo -e "\e[4;1;37m$1\033[0m"
	CHK_DIR_EXISTS = test -d $1 || mkdir -p $1
	NUKE = rm -r $1
	COPY_DIR = cp -rv $1 $2
	FIX_PATH = $1
	INSTALL_LIB_DIR := ~/lib/
	INSTALL_BIN_DIR := ~/bin/
	INSTALL_INCLUDE_DIR := ~/include/
	LIB_SUFFIX :=.so
	OPENCL_HEADERS :=/usr/include/CL/*.h
endif

PROJECT_DIR :=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))
OBJ_DIR := $(PROJECT_DIR)obj
BIN_DIR := $(PROJECT_DIR)bin
EXAMPLE_BIN_DIR := $(BIN_DIR)/examples
SRC_DIR := $(PROJECT_DIR)src
EXAMPLE_DIR := $(PROJECT_DIR)/examples

C_FILES := $(wildcard $(SRC_DIR)/*.c)
BINARIES := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(C_FILES))

COMMON_C_FILES := $(wildcard $(SRC_DIR)/common/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/common/%.c,$(OBJ_DIR)/%.o,$(COMMON_C_FILES))

LD_FLAGS += --std=gnu99 -march=native -lOpenCL -lclu -lutilities -L$(INSTALL_LIB_DIR)
C_FLAGS += --std=gnu99 -O2 -pipe -march=native -I$(PROJECT_DIR)headers -I$(INSTALL_INCLUDE_DIR)

all : binaries
	$(call REPORT, $(C_FILES))

binaries: $(BINARIES)

$(BIN_DIR)% : $(SRC_DIR)/%.c $(OBJ_FILES)
	$(call REPORT,Building $@)
	$(call CHK_DIR_EXISTS, $(dir $@))
	gcc $(C_FLAGS) -o "$@" "$<" $(OBJ_FILES) $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/common/%.c
	$(call REPORT,Compiling $@)
	$(call CHK_DIR_EXISTS, $(dir $@))
	@gcc $(C_FLAGS) -o "$@" -c "$<"

clean:
	$(call REPORT,Cleaning...)
	-$(NUKE) "$(OBJ_DIR)" "$(BIN_DIR)"
