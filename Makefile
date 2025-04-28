NAME := ircserv
NAME_DEBUG := $(NAME)_debug

CXX := c++
BASE_FLAGS := -std=c++11
WARNING_FLAGS := -Wall -Wextra -Werror -Wshadow -Wconversion -Wdouble-promotion -Woverloaded-virtual -Wsign-conversion -Wpedantic -Wswitch-enum
CXXFLAGS := $(BASE_FLAGS) $(WARNING_FLAGS)
DEP_FLAGS := -MMD -MP

DEBUG_FLAGS := -g3 -DDEBUG -fsanitize=address
RELEASE_FLAGS := -DNDEBUG

SRCDIR := src/
OBJDIR := obj/

OBJDIR_RELEASE := $(OBJDIR)release/
OBJDIR_DEBUG := $(OBJDIR)debug/

SRCFILES := Channel.cpp Chatbot.cpp Client.cpp CommandEnum.cpp CommandHelper.cpp Enum.cpp FileDescriptors.cpp MessageHelper.cpp Server.cpp Token.cpp Utils.cpp main.cpp
SRCS := $(addprefix $(SRCDIR), $(SRCFILES))

OBJS := $(SRCFILES:%.cpp=$(OBJDIR_RELEASE)%.o)
OBJS_DEBUG := $(SRCFILES:%.cpp=$(OBJDIR_DEBUG)%.o)

DEPS := $(OBJS:.o=.d)
DEPS_DEBUG := $(OBJS_DEBUG:.o=.d)

HEADERS := -I include

BLUE := \033[36m
MARGENTA := \033[35m
NC := \033[0m

.PHONY: all
all: $(NAME_DEBUG)  ## Build the release version (default)

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
	$(CXX) -v $(CXXFLAGS) $(RELEASE_FLAGS) $^ $(HEADERS) -o $@

$(NAME_DEBUG): $(OBJS_DEBUG)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $^ $(HEADERS) -o $@

$(OBJDIR_RELEASE)%.o: $(SRCDIR)%.cpp | $(OBJDIR_RELEASE)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(DEP_FLAGS) $(HEADERS) -c $< -o $@

$(OBJDIR_DEBUG)%.o: $(SRCDIR)%.cpp | $(OBJDIR_DEBUG)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(DEP_FLAGS) $(HEADERS) -c $< -o $@

$(OBJDIR_RELEASE) $(OBJDIR_DEBUG):
	mkdir -p $@

-include $(DEPS) $(DEPS_DEBUG)
