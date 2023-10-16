BUILD_DIR = ./build/$(if $(DEBUG),debug,release)
CXX := ./ccache clang++

SRC_ROOT ?= ./src
OBJ_ROOT ?= $(BUILD_DIR)/$(ENGINE_CORE_SRC_ROOT)
EXE_NAME ?= SlugReimplementation.exe
OBJ_EXTENSION ?= object

# NOTE: -Wpadded reports bloating of structs with padding !!
CFLAGS = $(if $(DEBUG),-O0 -g, -O2) -std=c++17 -fno-exceptions -fno-rtti \
	-Weverything \
	-Wno-switch-enum \
	-Wno-c++98-compat-pedantic \
	-Wno-c++98-compat \
	-Wno-c++98-c++11-compat-pedantic \
	-Wno-padded \
	-Wno-newline-eof \
	-Wno-missing-prototypes \
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

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
INCLUDES = -I./vendor/glm


SRCS := main.cpp
OBJS := $(addprefix $(OBJ_ROOT)/, $(SRCS:.cpp=.$(OBJ_EXTENSION)))
SRCS := $(addprefix $(SRC_ROOT)/, $(SRCS))
# -- .cpp from source dir -> .o object files in build dir
$(OBJS): $(OBJ_ROOT)/%.$(OBJ_EXTENSION): $(SRC_ROOT)/%.cpp
	mkdir -p $(OBJ_ROOT)
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDES)

# -- .o from build dir -> executable in build dir
$(BUILD_DIR)/$(EXE_NAME): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# vendor lib :: glm
VENDOR_GLM_SRC := ./vendor/glm/glm/detail/glm.cpp
VENDOR_GLM_OBJ := $(BUILD_DIR)/vendor/glm/glm.$(OBJ_EXTENSION)
$(VENDOR_GLM_OBJ): $(BUILD_DIR)/vendor/glm/%.$(OBJ_EXTENSION): ./vendor/glm/glm/detail/%.cpp
	mkdir -p $(BUILD_DIR)/vendor/glm
	$(CXX) $(CFLAGS) -Wno-everything -fPIC -c $< -o $@ -I./vendor/glm
$(BUILD_DIR)/vendor/glm.a: $(VENDOR_GLM_OBJ)
	ar r $@ $^

ifneq ($(f),) # force rebulid
.PHONY: $(OBJS)
endif

.PHONY: build
build: $(BUILD_DIR)
	$(MAKE) $(BUILD_DIR)/$(EXE_NAME)

.PHONY: run
run: build
	@echo "\n=== RUNNING == $(BUILD_DIR)/$(EXE_NAME) =="
	@$(BUILD_DIR)/$(EXE_NAME)

.PHONY: rm
rm:
	rm -rf $(BUILD_DIR)/$(subst .,*.,$(EXE_NAME))
	rm -rf $(BUILD_DIR)/**/*.$(OBJ_EXTENSION)
	find $(BUILD_DIR) -name '*.$(OBJ_EXTENSION)' -delete


.PHONY: install_linux
install_linux: vendor/glm
	sudo apt update
	sudo apt install libglfw3-dev
	wget https://github.com/ccache/ccache/releases/download/v4.8.3/ccache-4.8.3-linux-x86_64.tar.xz \
		-O ccache.tar.xz && mkdir -p ccache_prebuilt \
		&& tar -xJf ccache.tar.xz --directory ccache_prebuilt \
		&& cp ccache_prebuilt/*/ccache . \
		&& rm -rf ccache_prebuilt


vendor/glm:
#git submodule add -b tags/0.9.9.8 https://github.com/g-truc/glm.git vendor/glm
	mkdir -p $(BUILD_DIR)/vendor/glm
	wget https://github.com/g-truc/glm/releases/download/0.9.9.7/glm-0.9.9.7.zip \
		-O /tmp/glm.zip && mkdir -p ./vendor && unzip /tmp/glm.zip -d ./vendor/

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
