COMPILER		:= clang++
CFLAGS			:= -std=c++17 -g -O3 -Wall -Wextra -fdiagnostics-color=always
CFLAGS_ORIG		:= $(CFLAGS)
LDFLAGS			:=
CC				 = $(COMPILER) $(CFLAGS) $(CHECKFLAGS)
CHECKFLAGS		:=
MKBUILD			:= mkdir -p build
OUTPUT			:= build/spjalla

CHECK			:= asan

ifeq ($(CHECK), asan)
	CHECKFLAGS += -fsanitize=address -fno-common
else ifeq ($(CHECK), msan)
	CHECKFLAGS += -fsanitize=memory -fno-common
else
	CHECKFLAGS +=
endif

.PHONY: all test clean depend
all: Makefile


# Peter Miller, "Recursive Make Considered Harmful" (http://aegis.sourceforge.net/auug97.pdf)
SRCDIR_PP		:= pingpong/src
MODULES			:= core test commands messages lib
CFLAGS			+= -Ipingpong/include
COMMONSRC		:=
SRC				:=
include $(patsubst %,$(SRCDIR_PP)/%/module.mk,$(MODULES))
SRC				+= $(COMMONSRC)
COMMONSRC_PP	:= $(COMMONSRC)
COMMONOBJ_PP	:= $(patsubst src/%.cpp,pingpong/build/%.o, $(filter %.cpp,$(COMMONSRC)))
OBJ_PP			:= $(patsubst src/%.cpp,pingpong/build/%.o, $(filter %.cpp,$(SRC)))
sinclude $(patsubst %,$(SRCDIR_PP)/%/targets.mk,$(MODULES))
SRC_PP			:= $(patsubst %,pingpong/%,$(SRC))

MODULES			:= core
COMMONSRC		:=
SRC				:=
CFLAGS			+= -Iinclude
include $(patsubst %,src/%/module.mk,$(MODULES))
SRC				+= $(COMMONSRC)
COMMONOBJ		:= $(patsubst src/%.cpp,build/%.o, $(filter %.cpp,$(COMMONSRC))) $(COMMONOBJ_PP) 
OBJ				:= $(patsubst src/%.cpp,build/%.o, $(filter %.cpp,$(SRC)))

OBJ_ALL			:= $(OBJ) $(OBJ_PP)
SRC_ALL			:= $(SRC) $(SRC_PP)

sinclude $(patsubst %,src/%/targets.mk,$(MODULES))

include pingpong/conan.mk

all: $(COMMONOBJ) $(OUTPUT)

pingpong/build/%.o: pingpong/src/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

test: $(OUTPUT)
	./$(OUTPUT)

grind: $(OUTPUT)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=yes ./$(OUTPUT)

clean:
	rm -rf build
	$(MAKE) -C pingpong clean

DEPFILE  = .dep
DEPTOKEN = "\# MAKEDEPENDS"
DEPFLAGS = -f $(DEPFILE) -s $(DEPTOKEN)

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CC) -- $(SRC_ALL) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
