RED := \033[0;31m
RED_BLOD := \033[1;31m
GREEN := \033[0;32m
GREEN_BLOD := \033[1;32m
YELLOW := \033[0;33m
YELLOW_BLOD := \033[1;33m
BLUE := \033[0;34m
BLUE_BLOD := \033[1;34m
MAGENTA := \033[0;35m
MAGENTA_BLOD := \033[1;35m
CYAN := \033[0;36m
CYAN_BLOD := \033[1;36m
END := \033[0m

TARGET := $(shell basename "$$PWD")

DEBUG := 1
USE_WEBOTS := 1

CC := g++
OPT := -std=c++20 -og
DEF := \

BUILD_DIR := ./build
SRCS := 
SRC_DIRS := \
src
INC_DIRS := \
src \
3rdparty \
3rdparty/imgui \
3rdparty/imgui/backends
LDFLAGS := \
-lm \
-ldl \
-lpthread \
-lGL \
-lglfw
STATIC_LIBS = 3rdparty/lib/libimgui.a 3rdparty/lib/libimplot.a

SRCS += $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.cc' -or -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
CPPFLAGS := $(addprefix -D, $(DEF)) $(addprefix -I, $(INC_DIRS)) $(OPT) -Wall
ifeq ($(DEBUG), 1)
	CPPFLAGS += -g -gdwarf-2
endif

.PHONY: all run clean
.DEFAULT_GOAL := all
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) $(STATIC_LIBS) | $(BUILD_DIR)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(CYAN)LD$(END) $@"
	@$(CC) $(OBJS) $(STATIC_LIBS) $(LDFLAGS) -o $@

$(STATIC_LIBS): Makefile | $(BUILD_DIR)
	@$(MAKE) --no-print-directory -C 3rdparty -j

$(BUILD_DIR)/%.c.o: %.c Makefile | $(BUILD_DIR)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(GREEN)CC$(END) $<"
	@mkdir -p $(dir $@) 
	@$(CC) -c $(CPPFLAGS) $< -o $@
$(BUILD_DIR)/%.cc.o: %.cc Makefile | $(BUILD_DIR)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(GREEN)CC$(END) $<"
	@mkdir -p $(dir $@) 
	@$(CC) -c $(CPPFLAGS) $< -o $@
$(BUILD_DIR)/%.cpp.o: %.cpp Makefile | $(BUILD_DIR)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(GREEN)CC$(END) $<"
	@mkdir -p $(dir $@) 
	@$(CC) -c $(CPPFLAGS) $< -o $@

$(BUILD_DIR):
	@mkdir $@

run:
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(YELLOW)starting...$(END)"
	@-$(BUILD_DIR)/$(TARGET)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(YELLOW)end$(END)"

clean:
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(RED)cleaning...$(END)"
	@-rm -fR $(BUILD_DIR)

clean_lib:
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(RED)cleaning...$(END)"
	@-rm -fR 3rdparty/lib

-include $(DEPS)


