.SUFFIXES:
.DEFAULT:
MAKEFLAGS += -r --no-print-directory

BUILDDIR ?= build
CC ?= cc
CXX ?= c++
CFLAGS += -Wall -Wextra -Werror -pedantic -I$(STAMESKA_BASEDIR) -I$(STAMESKA_BASEDIR)/3p/atto -I$(STAMESKA_BASEDIR)/3p -I$(STAMESKA_BASEDIR)/src
CXXFLAGS += -std=c++17 $(CFLAGS)
LIBS = -lX11 -lXfixes -lGL -lasound -lm -pthread

YAML_MAJOR=0
YAML_MINOR=2
YAML_PATCH=1

CFLAGS += \
	-I3p/libyaml/include \
	-DYAML_MAJOR=$(YAML_MAJOR) \
	-DYAML_MINOR=$(YAML_MINOR) \
	-DYAML_PATCH=$(YAML_PATCH) \
	-DYAML_VERSION_MAJOR=$(YAML_MAJOR) \
	-DYAML_VERSION_MINOR=$(YAML_MINOR) \
	-DYAML_VERSION_PATCH=$(YAML_PATCH) \
	-DYAML_VERSION_STRING="\"$(YAML_MAJOR).$(YAML_MINOR).$(YAML_PATCH)\""

ifeq ($(DEBUG), 1)
	CONFIG = dbg
	CFLAGS += -O0 -g -DDEBUG
else
	CONFIG = rel
	CFLAGS += -O3
endif

ifeq ($(ASAN), 1)
	CONFIG:=$(CONFIG)-asan
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LIBS += -fsanitize=address
endif

PLATFORM=linux-x11
COMPILER ?= $(CC)

DEPFLAGS = -MMD -MP
COMPILE.c = $(CC) -std=gnu99 $(CFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d
COMPILE.cc = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d

OBJDIR ?= $(BUILDDIR)/$(PLATFORM)-$(CONFIG)

all: stameska

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) -c $< -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.cc) -c $< -o $@

STAMESKA_EXE = $(OBJDIR)/stameska
STAMESKA_SOURCES += \
	$(STAMESKA_BASEDIR)/3p/atto/src/app_linux.c \
	$(STAMESKA_BASEDIR)/3p/atto/src/app_x11.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/api.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/loader.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/parser.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/reader.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/scanner.c \
	$(STAMESKA_BASEDIR)/src/PolledFile.cpp \
	$(STAMESKA_BASEDIR)/src/PolledPipelineDesc.cpp \
	$(STAMESKA_BASEDIR)/src/PolledShaderProgram.cpp \
	$(STAMESKA_BASEDIR)/src/PolledShaderSource.cpp \
	$(STAMESKA_BASEDIR)/src/ProjectSettings.cpp \
	$(STAMESKA_BASEDIR)/src/RenderDesc.cpp \
	$(STAMESKA_BASEDIR)/src/Resources.cpp \
	$(STAMESKA_BASEDIR)/src/ShaderSource.cpp \
	$(STAMESKA_BASEDIR)/src/Timeline.cpp \
	$(STAMESKA_BASEDIR)/src/VideoEngine.cpp \
	$(STAMESKA_BASEDIR)/src/YamlParser.cpp \
	$(STAMESKA_BASEDIR)/src/tool.cpp \
	$(STAMESKA_BASEDIR)/src/utils.cpp \
	$(STAMESKA_BASEDIR)/src/video.cpp \

# TODO how to handle ../
STAMESKA_OBJS = $(STAMESKA_SOURCES:%=$(OBJDIR)/%.o)
STAMESKA_DEPS = $(STAMESKA_OBJS:%=%.d)
-include $(STAMESKA_DEPS)

$(STAMESKA_BASEDIR)/3p/rocket/lib/librocket.a:
	MAKEFLAGS= make -C $(STAMESKA_BASEDIR)/3p/rocket lib/librocket.a

$(STAMESKA_EXE): $(STAMESKA_OBJS) $(STAMESKA_BASEDIR)/3p/rocket/lib/librocket.a
	$(CXX) $(LIBS) $(STAMESKA_BASEDIR)/3p/rocket/lib/librocket.a $^ -o $@

stameska: $(STAMESKA_EXE)

clean:
	rm -rf build

.PHONY: all clean
