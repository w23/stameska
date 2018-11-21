.SUFFIXES:
.DEFAULT:
MAKEFLAGS += -r --no-print-directory -j $(shell nproc)

INTRO=example
BUILDDIR ?= build
CC ?= cc
CXX ?= c++
CFLAGS += -Wall -Wextra -Werror -pedantic -I. -I3p/atto -I3p -Isrc
CXXFLAGS += -std=c++17 $(CFLAGS)
LIBS = -lX11 -lXfixes -lGL -lasound -lm -pthread

ifeq ($(DEBUG), 1)
	CONFIG = dbg
	CFLAGS += -O0 -g
else
	CONFIG = rel
	CFLAGS += -O3
endif

PLATFORM=linux-x11
COMPILER ?= $(CC)

DEPFLAGS = -MMD -MP
COMPILE.c = $(CC) -std=gnu99 $(CFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d
COMPILE.cc = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d

OBJDIR ?= $(BUILDDIR)/$(PLATFORM)-$(CONFIG)

all: tool

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) -c $< -o $@

$(OBJDIR)/%.asm.obj: %.asm shaders/intro.inc shaders/blitter.inc
	@mkdir -p $(dir $@)
	nasm -f win32 -i4klang_win32/ $< -o $@

$(OBJDIR)/%.c.o32: %.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) -m32 -c $< -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.cc) -c $< -o $@

$(OBJDIR)/4klang.o32: 4klang.asm ./4klang_linux/4klang.inc
	nasm -f elf32 -I./4klang_linux/ 4klang.asm -o $@

TOOL_EXE = $(OBJDIR)/tool
TOOL_SRCS = \
	3p/atto/src/app_linux.c \
	3p/atto/src/app_x11.c \
	src/utils.cpp \
	src/ShaderSource.cpp \
	src/video.cpp \
	src/tool.cpp

TOOL_OBJS = $(TOOL_SRCS:%=$(OBJDIR)/%.o)
TOOL_DEPS = $(TOOL_OBJS:%=%.d)
-include $(TOOL_DEPS)

$(TOOL_EXE): $(TOOL_OBJS)
	$(CXX) $(LIBS) $^ -o $@

tool: $(TOOL_EXE)

TEST_EXE = $(OBJDIR)/test_
TEST_SRCS = \
	src/ShaderSource.cpp \
	test/ShaderSource.cpp

TEST_OBJS = $(TEST_SRCS:%=$(OBJDIR)/%.o)
TEST_DEPS = $(TEST_OBJS:%=%.d)
-include $(TEST_DEPS)

$(TEST_EXE): $(TEST_OBJS)
	$(CXX) $(LIBS) $^ -o $@

run_test: $(TEST_EXE)
	./$(TEST_EXE)

clean:
	rm $(BUILDDIR)

.PHONY: all clean
