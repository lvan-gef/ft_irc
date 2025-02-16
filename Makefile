NAME := ircserv
NAME_DEBUG := $(NAME)_debug
NAME_TEST := $(NAME)_test

CXX := c++
BASE_FLAGS := -std=c++11
WARNING_FLAGS := -Wall -Wextra -Werror -Wshadow -Wconversion -Wdouble-promotion -Woverloaded-virtual -Wpedantic
CXXFLAGS := $(BASE_FLAGS) $(WARNING_FLAGS)
DEP_FLAGS := -MMD -MP

DEBUG_FLAGS := -g3 -DDEBUG
RELEASE_FLAGS := -DNDEBUG

SRCDIR := src/
TESTDIR := tester/src
OBJDIR := obj/

OBJDIR_RELEASE := $(OBJDIR)release/
OBJDIR_DEBUG := $(OBJDIR)debug/

SRCFILES := Client.cpp main.cpp Server.cpp utils.cpp
TESTFILES := $(NAME)Tester.cpp
SRCS := $(addprefix $(SRCDIR), $(SRCFILES))
TESTSRCS := $(addprefix $(TESTDIR)/, $(TESTFILES))

OBJS := $(SRCFILES:%.cpp=$(OBJDIR_RELEASE)%.o)
OBJS_DEBUG := $(SRCFILES:%.cpp=$(OBJDIR_DEBUG)%.o)
TESTOBJS := $(TESTFILES:%.cpp=$(OBJDIR_RELEASE)%.o)
TESTOBJS_DEBUG := $(TESTFILES:%.cpp=$(OBJDIR_DEBUG)%.o)

DEPS := $(OBJS:.o=.d)
DEPS_DEBUG := $(OBJS_DEBUG:.o=.d)
TESTDEPS := $(TESTOBJS:.o=.d)
TESTDEPS_DEBUG := $(TESTOBJS_DEBUG:.o=.d)

HEADERS := -I include

BLUE := \033[36m
MARGENTA := \033[35m
NC := \033[0m

.PHONY: all
all: $(NAME)  ## Build the release version (default)

.PHONY: debug
debug: $(NAME_DEBUG)  ## Build the debug version

.PHONY: fmt
fmt:  ## Format code via clang-format
	@echo "Format code"
	@find . -type f -name "*.*pp" -print0 | xargs -0 clang-format -i

.PHONY: clean
clean:  ## Clean object files
	rm -rf $(OBJDIR)

.PHONY: fclean
fclean: clean  ## Clean all side effects
	rm -rf $(NAME) $(NAME_DEBUG) $(NAME_TEST)

.PHONY: re  ## Clean all and recompile
re: fclean all

.PHONY: help
help:  ## Get help
	@echo -e 'Usage: make ${BLUE}<target>${NC} ${MARGENTA}<options>${NC}'
	@echo -e 'Available targets:'
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  ${BLUE}%-15s${NC} %s\n", $$1, $$2}' $(MAKEFILE_LIST)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $^ $(HEADERS) -o $@

$(NAME_DEBUG): $(OBJS_DEBUG)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $^ $(HEADERS) -o $@

$(OBJDIR_RELEASE)%.o: $(SRCDIR)%.cpp | $(OBJDIR_RELEASE)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(DEP_FLAGS) $(HEADERS) -c $< -o $@

$(OBJDIR_RELEASE)%.o: $(TESTDIR)/%.cpp | $(OBJDIR_RELEASE)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(DEP_FLAGS) $(HEADERS) -c $< -o $@

$(OBJDIR_DEBUG)%.o: $(SRCDIR)%.cpp | $(OBJDIR_DEBUG)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(DEP_FLAGS) $(HEADERS) -c $< -o $@

$(OBJDIR_RELEASE) $(OBJDIR_DEBUG):
	mkdir -p $@

-include $(DEPS) $(DEPS_DEBUG) $(TESTDEPS) $(TESTDEPS_DEBUG)
