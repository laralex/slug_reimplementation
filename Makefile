BUILD_DIR = ./build/$(if $(DEBUG),debug,release)
CXX := ./ccache clang++

SRC_ROOT ?= ./src
OBJ_ROOT ?= $(BUILD_DIR)/$(ENGINE_CORE_SRC_ROOT)
EXE_NAME ?= SlugReimplementation.exe

# NOTE: -Wpadded reports bloating of structs with padding !!
CFLAGS = $(if $(DEBUG),-O0 -g, -O2 -DNDEBUG) -std=c++17 -fno-exceptions -fno-rtti \
	-Weverything \
	-Wno-switch-enum \
	-Wno-c++98-compat-pedantic \
	-Wno-c++98-compat \
	-Wno-c++98-c++11-compat-pedantic \
	-Wno-padded \
	-Wno-newline-eof \
	-Wno-missing-prototypes \
	-Wno-reserved-id-macro \
#	-Wno-signed-enum-bitfield \
	-Wno-deprecated-register \
	-Wno-nested-anon-types \
	-Wno-gnu-anonymous-struct \
	-Wno-documentation \
	-Wno-documentation-unknown-command \
	-Wno-weak-vtables \
	-Wno-unused-const-variable \
	-Wno-format-nonliteral \
	-Wno-global-constructors \
	-Wno-exit-time-destructors \
	-Wno-error=padded

LDFLAGS = -lglfw -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
INCLUDES = -I./vendor/glm -I./vendor/glad/include -I./vendor/imgui -I./vendor/ttfparser


SRCS := main.cpp shader.cpp util.cpp
OBJS := $(addprefix $(OBJ_ROOT)/, $(SRCS:.cpp=.o))
SRCS := $(addprefix $(SRC_ROOT)/, $(SRCS))
# -- .cpp from source dir -> .o object files in build dir
$(OBJS): $(OBJ_ROOT)/%.o: $(SRC_ROOT)/%.cpp
	mkdir -p $(OBJ_ROOT)
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDES)

VENDOR_OBJS=\
	$(OBJ_ROOT)/vendor/glad/gl.o \
	$(OBJ_ROOT)/vendor/imgui/imgui.o \
	$(OBJ_ROOT)/vendor/imgui/imgui.o \
	$(OBJ_ROOT)/vendor/imgui/imgui_impl_opengl3.o \
	$(OBJ_ROOT)/vendor/imgui/imgui_impl_glfw.o \
	$(OBJ_ROOT)/vendor/imgui/imgui_draw.o \
	$(OBJ_ROOT)/vendor/imgui/imgui_widgets.o \
	$(OBJ_ROOT)/vendor/imgui/imgui_tables.o \

$(OBJ_ROOT)/vendor/glad/gl.o: vendor/glad/src/gl.c
$(OBJ_ROOT)/vendor/imgui/imgui.o: vendor/imgui/imgui.cpp
$(OBJ_ROOT)/vendor/imgui/imgui_impl_opengl3.o: vendor/imgui/backends/imgui_impl_opengl3.cpp
$(OBJ_ROOT)/vendor/imgui/imgui_impl_glfw.o: vendor/imgui/backends/imgui_impl_glfw.cpp
$(OBJ_ROOT)/vendor/imgui/imgui_draw.o: vendor/imgui/imgui_draw.cpp
$(OBJ_ROOT)/vendor/imgui/imgui_widgets.o: vendor/imgui/imgui_widgets.cpp
$(OBJ_ROOT)/vendor/imgui/imgui_tables.o: vendor/imgui/imgui_tables.cpp

$(VENDOR_OBJS):
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDES) -Wno-everything

# -- .o from build dir -> executable in build dir
$(BUILD_DIR)/$(EXE_NAME): $(VENDOR_OBJS) $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# vendor lib :: glm
VENDOR_GLM_SRC := ./vendor/glm/glm/detail/glm.cpp
VENDOR_GLM_OBJ := $(BUILD_DIR)/vendor/glm/glm.o
$(VENDOR_GLM_OBJ): $(BUILD_DIR)/vendor/glm/%.o: ./vendor/glm/glm/detail/%.cpp
	mkdir -p $(BUILD_DIR)/vendor/glm
	$(CXX) $(CFLAGS) -Wno-everything -fPIC -c $< -o $@ -I./vendor/glm
$(BUILD_DIR)/vendor/glm.a: $(VENDOR_GLM_OBJ)
	ar r $@ $^

ifneq ($(f),) # force rebulid
.PHONY: $(OBJS)
endif

INSTALL_SHADERS = shader.frag shader.vert
$(INSTALL_SHADERS):
	mkdir -p $(BUILD_DIR)/shaders
	cp src/shaders/$@ $(BUILD_DIR)/shaders

.PHONY: build
build: $(BUILD_DIR)
	$(MAKE) $(BUILD_DIR)/$(EXE_NAME)

.PHONY: run
run: build
	@echo "\n=== RUNNING == $(BUILD_DIR)/$(EXE_NAME) =="
	@$(BUILD_DIR)/$(EXE_NAME)

.PHONY: rm
rm:
	rm -f $(BUILD_DIR)/$(subst .,*.,$(EXE_NAME))
	rm -f $(BUILD_DIR)/**/*.o
	rm -f $(BUILD_DIR)/shaders/*
	find $(BUILD_DIR) -name '*.o' -delete


.PHONY: install_linux
install_linux: vendor/glm vendor/imgui vendor/ttfparser
	sudo apt update
	sudo apt install libglfw3-dev
	wget https://github.com/ccache/ccache/releases/download/v4.8.3/ccache-4.8.3-linux-x86_64.tar.xz \
		-O ccache.tar.xz && mkdir -p ccache_prebuilt \
		&& tar -xJf ccache.tar.xz --directory ccache_prebuilt \
		&& cp ccache_prebuilt/*/ccache . \
		&& rm -rf ccache_prebuilt

vendor/imgui: 
	wget https://github.com/ocornut/imgui/archive/refs/tags/v1.89.9.zip \
		-O /tmp/imgui.zip && mkdir -p ./vendor && unzip /tmp/imgui.zip -d ./vendor/
	mv ./vendor/imgui-1.89.9 ./vendor/imgui


vendor/glm:
#git submodule add -b tags/0.9.9.8 https://github.com/g-truc/glm.git vendor/glm
	wget https://github.com/g-truc/glm/releases/download/0.9.9.7/glm-0.9.9.7.zip \
		-O /tmp/glm.zip && mkdir -p ./vendor && unzip /tmp/glm.zip -d ./vendor/

vendor/ttfparser:
	mkdir -p ./vendor/ttfparser
	wget https://github.com/kv01/ttf-parser/blob/065fcd9db7c2d7256b1ae77480a269db1f689d8e/src/ttfParser.h
		-O ./vendor/ttfparser/ttfparser.h

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
