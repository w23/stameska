.SUFFIXES:
.DEFAULT:
MAKEFLAGS += -r --no-print-directory -j $(shell nproc)

INTRO=example
OBJDIR ?= .obj
MIDIDEV ?= ''
WIDTH ?= 1280
HEIGHT ?= 720
SHMIN=mono shader_minifier.exe
CC ?= cc
CXX ?= c++
CFLAGS += -Wall -Wextra -Werror -pedantic -DTOOL -DWIDTH=$(WIDTH) -DHEIGHT=$(HEIGHT) -I. -I3p/atto -I3p -O0 -g
CXXFLAGS += -std=c++11 $(CFLAGS)
LIBS = -lX11 -lXfixes -lGL -lasound -lm -pthread

DEPFLAGS = -MMD -MP
COMPILE.c = $(CC) -std=gnu99 $(CFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d
COMPILE.cc = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d

all: run_tool

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

TOOL_EXE = $(OBJDIR)/src/tool
TOOL_SRCS = \
	3p/atto/src/app_linux.c \
	3p/atto/src/app_x11.c \
	src/video.cpp \
	src/tool.cpp

TOOL_OBJS = $(TOOL_SRCS:%=$(OBJDIR)/%.o)
TOOL_DEPS = $(TOOL_OBJS:%=%.d)

-include $(TOOL_DEPS)

$(TOOL_EXE): $(TOOL_OBJS)
	$(CXX) $(LIBS) $^ -o $@

tool: $(TOOL_EXE)

DUMP_AUDIO_EXE = $(OBJDIR)/dump_audio
DUMP_AUDIO_SRCS = example/dump_audio.c
DUMP_AUDIO_OBJS = $(DUMP_AUDIO_SRCS:%=$(OBJDIR)/%.o32)
DUMP_AUDIO_DEPS = $(DUMP_AUDIO_OBJS:%=%.d)

INTRO_WIN32_SRCS = example/4klang.asm example/$(INTRO).asm
INTRO_WIN32_OBJS = $(INTRO_WIN32_SRCS:%=$(OBJDIR)/%.obj)

-include $(DUMP_AUDIO_DEPS)

$(DUMP_AUDIO_EXE): $(DUMP_AUDIO_OBJS) $(OBJDIR)/4klang.o32
	$(CC) -m32 $(LIBS) $^ -o $@

$(OBJDIR)/audio.raw: $(DUMP_AUDIO_EXE)
	$(DUMP_AUDIO_EXE) $@

clean:
	rm -f $(TOOL_OBJS) $(TOOL_DEPS) $(TOOL_EXE)
	rm -f $(DUMP_AUDIO_OBJS) $(DUMP_AUDIO_DEPS) $(DUMP_AUDIO_EXE) $(OBJDIR)/audio.raw
	rm -f $(INTRO).sh $(INTRO).gz $(INTRO).elf $(INTRO) $(INTRO).dbg

run_tool: $(TOOL_EXE) $(OBJDIR)/audio.raw
	$(TOOL_EXE) -w $(WIDTH) -h $(HEIGHT) -m $(MIDIDEV)

debug_tool: $(TOOL_EXE) $(OBJDIR)/audio.raw
	gdb --args $(TOOL_EXE) -w $(WIDTH) -h $(HEIGHT) -m $(MIDIDEV)

$(INTRO).sh: linux_header $(INTRO).gz
	cat linux_header $(INTRO).gz > $@
	chmod +x $@

$(INTRO).gz: $(INTRO).elf
	cat $< | 7z a dummy -tGZip -mx=9 -si -so > $@

%.h: %.glsl
	$(SHMIN) -o $@ --no-renaming-list S,F,main --preserve-externals $<

%.inc: %.glsl
	$(SHMIN) -o $@ --no-renaming-list S,F,main --format nasm --preserve-externals $<

#.h.seq: timepack
#	timepack $< $@

#timepack: timepack.c
#	$(CC) -std=c99 -Wall -Werror -Wextra -pedantic -lm timepack.c -o timepack

# '-nostartfiles -DCOMPACT' result in a libSDL crash on my machine (making older 1k/4k also crash) :(
$(INTRO).elf: $(OBJDIR)/4klang.o32 intro.c
	$(CC) -m32 -Os -Wall -Wno-unknown-pragmas -I. \
		-DFULLSCREEN `pkg-config --cflags --libs sdl` -lGL \
		$^ -o $@
	sstrip $@

$(INTRO).dbg: $(OBJDIR)/4klang.o32 intro.c
	$(CC) -m32 -O0 -g  -D_DEBUG -Wall -Wno-unknown-pragmas -I. \
		`pkg-config --cflags --libs sdl` -lGL \
		$^ -o $@

CRINKLER=wine link.exe
INTRO_WIN32_LIBS = /LIBPATH:. opengl32.lib winmm.lib kernel32.lib user32.lib gdi32.lib
INTRO_WIN32_CRINKLER_OPTS = /ENTRY:entrypoint /CRINKLER /UNSAFEIMPORT /NOALIGNMENT /NOINITIALIZERS /RANGE:opengl32 /PRINT:IMPORTS /PRINT:LABELS /TRANSFORM:CALLS /TINYIMPORT /NODEFAULTLIB /SUBSYSTEM:WINDOWS

win32-fast: $(INTRO)-fast.exe

$(INTRO)-fast.exe: $(INTRO_WIN32_OBJS)
	$(CRINKLER) \
		$(INTRO_WIN32_CRINKLER_OPTS) \
		/COMPMODE:FAST /REPORT:report-fast.html \
		$(INTRO_WIN32_LIBS) \
		$(INTRO_WIN32_OBJS) \
		/OUT:$@

win32-slow: $(INTRO)-slow.exe

$(INTRO)-slow.exe: $(INTRO_WIN32_OBJS)
	$(CRINKLER) \
		$(INTRO_WIN32_CRINKLER_OPTS) \
		/HASHTRIES:3000 /COMPMODE:SLOW /ORDERTRIES:6000 /REPORT:report-slow.html /SATURATE /HASHSIZE:900 /UNSAFEIMPORT \
		$(INTRO_WIN32_LIBS) \
		$(INTRO_WIN32_OBJS) \
		/OUT:$@

$(INTRO).capture: shaders/intro.h shaders/blitter.h intro.c
	$(CC) -O3 -Wall -Wno-unknown-pragmas -I. \
		-DCAPTURE `pkg-config --cflags --libs sdl` -lGL \
		$^ -o $@

FFMPEG_ARGS = \
	-s:v $(WIDTH)x$(HEIGHT) -pix_fmt rgb24 \
	-y -f rawvideo -vcodec rawvideo \
	-framerate 60 \
	-i - \
	-f f32le -ar 44100 -ac 2 \
	-i audio.raw \
	-s:v 3840:2160 \
	-c:a aac -b:a 160k \
	-c:v libx264 -vf vflip \
	-movflags +faststart \
	-level 4.1 -profile:v high -preset veryslow -crf 14.0 -pix_fmt yuv420p \
	-tune film

# -vf scale=3840:2160:flags=neighbor \
#	-x264-params keyint=600:bframes=3:scenecut=60:ref=3:qpmin=10:qpstep=8:vbv-bufsize=24000:vbv-maxrate=24000:merange=32 \

capture: $(INTRO)_$(WIDTH)_$(HEIGHT).mp4
test-capture: test_$(INTRO)_$(WIDTH)_$(HEIGHT).mp4

$(INTRO)_$(WIDTH)_$(HEIGHT).mp4: $(INTRO).capture audio.raw
	./$(INTRO).capture | ffmpeg \
	$(FFMPEG_ARGS) \
	$@

test_$(INTRO)_$(WIDTH)_$(HEIGHT).mp4: $(INTRO).capture audio.raw
	./$(INTRO).capture | ffmpeg \
	-t 48 \
	$(FFMPEG_ARGS) \
	$@

	# "//-crf 18 -preset slow -vf vflip  \
	# " -level:v 4.2 -profile:v high -preset slower -crf 20.0 -pix_fmt yuv420p"


.PHONY: all clean run_tool debug_tool
