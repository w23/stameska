ifeq ($(ASAN), 1)
	CONFIG:=$(CONFIG)-asan
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LIBS += -fsanitize=address
endif

ATTO_BASEDIR=$(STAMESKA_BASEDIR)/3p/atto
include $(STAMESKA_BASEDIR)/3p/atto/atto.mk

STAMESKA ?= stameska
BUILDDIR ?= build
CC ?= cc
CXX ?= c++
CFLAGS += -ggdb3
CFLAGS += -I$(STAMESKA_BASEDIR) -I$(STAMESKA_BASEDIR)/3p -I$(STAMESKA_BASEDIR)/src
#CFLAGS += -Wshadow
#CFLAGS += -Wnon-virtual-dtor
#CFLAGS += -Wold-style-cast
#CFLAGS += -Wcast-align
#CFLAGS += -Wunused
#CFLAGS += -Woverloaded-virtual
#CFLAGS += -Wpedantic
#CFLAGS += -Wconversion
#CFLAGS += -Wsign-conversion
#CFLAGS += -Wnull-dereference
#CFLAGS += -Wdouble-promotion
#CFLAGS += -Wformat=2
#CFLAGS += -Wduplicated-cond
#CFLAGS += -Wduplicated-branches
#CFLAGS += -Wlogical-op
#CFLAGS += -Wuseless-cast
# only experimental clang support CFLAGS += -Wlifetime
CXXFLAGS += -std=c++17 -fno-exceptions -fno-rtti $(CFLAGS)
LIBS += -lstdc++fs

YAML_MAJOR=0
YAML_MINOR=2
YAML_PATCH=1

CFLAGS += \
	-I$(STAMESKA_BASEDIR)/3p/libyaml/include \
	-DYAML_MAJOR=$(YAML_MAJOR) \
	-DYAML_MINOR=$(YAML_MINOR) \
	-DYAML_PATCH=$(YAML_PATCH) \
	-DYAML_VERSION_MAJOR=$(YAML_MAJOR) \
	-DYAML_VERSION_MINOR=$(YAML_MINOR) \
	-DYAML_VERSION_PATCH=$(YAML_PATCH) \
	-DYAML_VERSION_STRING="\"$(YAML_MAJOR).$(YAML_MINOR).$(YAML_PATCH)\""

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG
endif

ifneq ($(RASPBERRY), 1)
	LIBS += -lasound
endif

COMPILE.cc = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d

$(OBJDIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.cc) -c $< -o $@

$(OBJDIR)/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	$(COMPILE.cc) -c $< -o $@

LIBYAML_SOURCES= \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/api.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/loader.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/parser.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/reader.c \
	$(STAMESKA_BASEDIR)/3p/libyaml/src/scanner.c \

STAMESKA_EXE = $(OBJDIR)/$(STAMESKA)
STAMESKA_SOURCES += \
	$(LIBYAML_SOURCES) \
	$(STAMESKA_BASEDIR)/src/AutomationBasic.cpp \
	$(STAMESKA_BASEDIR)/src/Export.cpp \
	$(STAMESKA_BASEDIR)/src/OpenGL.cpp \
	$(STAMESKA_BASEDIR)/src/PolledFile.cpp \
	$(STAMESKA_BASEDIR)/src/PolledPipelineDesc.cpp \
	$(STAMESKA_BASEDIR)/src/PolledShaderProgram.cpp \
	$(STAMESKA_BASEDIR)/src/PolledShaderSource.cpp \
	$(STAMESKA_BASEDIR)/src/Program.cpp \
	$(STAMESKA_BASEDIR)/src/ProjectSettings.cpp \
	$(STAMESKA_BASEDIR)/src/RenderDesc.cpp \
	$(STAMESKA_BASEDIR)/src/Resources.cpp \
	$(STAMESKA_BASEDIR)/src/ShaderSource.cpp \
	$(STAMESKA_BASEDIR)/src/Rocket.cpp \
	$(STAMESKA_BASEDIR)/src/VideoEngine.cpp \
	$(STAMESKA_BASEDIR)/src/YamlParser.cpp \
	$(STAMESKA_BASEDIR)/src/MIDI.cc \
	$(STAMESKA_BASEDIR)/src/tool.cpp \
	$(STAMESKA_BASEDIR)/src/format.cpp \
	$(STAMESKA_BASEDIR)/src/video.cpp \

STAMESKA_OBJS = $(STAMESKA_SOURCES:%=$(OBJDIR)/%.o)
STAMESKA_DEPS = $(STAMESKA_OBJS:%=%.d)
-include $(STAMESKA_DEPS)

$(STAMESKA_BASEDIR)/3p/rocket/lib/librocket.a:
	MAKEFLAGS= make -C $(STAMESKA_BASEDIR)/3p/rocket lib/librocket.a

$(STAMESKA_EXE): $(ATTO_OBJS) $(STAMESKA_OBJS) $(STAMESKA_BASEDIR)/3p/rocket/lib/librocket.a
	$(CXX) $(CXXFLAGS) $^ $(STAMESKA_BASEDIR)/3p/rocket/lib/librocket.a $(LIBS) -o $@

stameska: $(STAMESKA_EXE)

.PHONY: all clean
