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


BUILD_DIR = ../build

IMGUI_SRCS := \
$(wildcard imgui/*.cpp) \
imgui/backends/imgui_impl_glfw.cpp \
imgui/backends/imgui_impl_opengl3.cpp
IMPLOT_SRCS := implot/implot.cpp implot/implot_items.cpp

IMGUI_OBJS := $(IMGUI_SRCS:%=$(BUILD_DIR)/%.o)
IMPLOT_OBJS := $(IMPLOT_SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(IMGUI_OBJS:.o=.d) $(IMPLOT_OBJS:.o=.d)

CC := g++
OPT := -std=c++20 -og
DEF := \

INC_DIRS := $(shell find . -type d)
LDFLAGS := \
-lGL \
-lglfw
CPPFLAGS := $(addprefix -D, $(DEF)) $(addprefix -I, $(INC_DIRS)) $(OPT) -Wall


.PHONY: all
.DEFAULT_GOAL := all
all: lib/libimgui.a lib/libimplot.a

lib/libimgui.a: $(IMGUI_OBJS) | $(BUILD_DIR)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(MAGENTA)AR$(END) $@"
	@mkdir -p $(dir $@) 
	@ar rcs $@ $(IMGUI_OBJS)

lib/libimplot.a: $(IMPLOT_OBJS) | $(BUILD_DIR)
	@echo -e "$(YELLOW_BLOD)  ->$(END) $(MAGENTA)AR$(END) $@"
	@mkdir -p $(dir $@) 
	@ar rcs $@ $(IMPLOT_OBJS)

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

-include $(DEPS)

