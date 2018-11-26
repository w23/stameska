STAMESKA_BASEDIR = .
STAMESKA_SOURCES = \
	src/video.cpp \

include stameska.mk

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

.PHONY: all clean
