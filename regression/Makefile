MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
TEST := $(patsubst %/,%,$(dir $(MKFILE_PATH)))

BUILD_DIR := $(TEST)/build
EXECUTABLE := $(BUILD_DIR)/Regression

# Source and Headers
SOURCES := $(shell find $(TEST) -name "*.cpp")
HEADERS := $(shell find $(TEST) -name "*.hpp")
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

INC_HEADER_DIRS := $(shell find $(TEST) -type d -name "include")
INC_HEADER_LIST := $(addprefix -I,$(INC_HEADER_DIRS))

INC := $(INC_HEADER_LIST) 
LIB := -lstdc++ -lpthread -g -L.
CXXFLAGS := -pipe -std=c++17 -O3

bold := $(shell tput bold)
sgr0 := $(shell tput sgr0)

multi:
	$(MAKE) -j8 all

all : $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	mkdir -p build
	@printf '$(bold)Linking $@ ----------------->$(sgr0)\n'
	$(CXX) -o $(EXECUTABLE) $(OBJECTS) $(LIB)

%.o : %.cpp
	@printf '$(bold)Compiling $< ----------------->$(sgr0)\n'
	$(CXX) $(CXXFLAGS) $(INC) -MMD -MP -c -o $@ $< $(LIB)

clean:
	rm -rf $(OBJECTS) $(DEPS) $(EXECUTABLE) $(BUILD_DIR) build.log

-include $(DEPS)
