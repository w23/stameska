STAMESKA_BASEDIR = .

include stameska.mk

TEST_EXE = $(OBJDIR)/test_
TEST_SRCS = \
	$(LIBYAML_SOURCES) \
	src/utils.cpp \
	src/ShaderSource.cpp \
	src/YamlParser.cpp \
	src/RenderDesc.cpp \
	test/ShaderSource.cpp \
	test/RenderDesc.cpp \
	test/main.cpp

TEST_OBJS = $(TEST_SRCS:%=$(OBJDIR)/%.o)
TEST_DEPS = $(TEST_OBJS:%=%.d)
-include $(TEST_DEPS)

$(TEST_EXE): $(TEST_OBJS)
	$(CXX) $(LIBS) $^ -o $@

run_test: $(TEST_EXE)
	./$(TEST_EXE)

.PHONY: all clean
