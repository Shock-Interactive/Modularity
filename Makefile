# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP

# Detect OS for cleanup and executable extension
ifeq ($(OS),Windows_NT)
    RM := del /Q
    EXE := .exe
    LIBS := -lglfw3 -lopengl32 -lgdi32
else
    RM := rm -f
    EXE :=
    LIBS := -lglfw -lGL -ldl -lpthread
endif

# Target executable
TARGET := main$(EXE)

# Find all source files recursively
SRC := $(shell find . -type f -name '*.cpp')
OBJ := $(SRC:.cpp=.o)
DEPS := $(OBJ:.o=.d)

# Default target
all: $(TARGET)

# Link object files
$(TARGET): $(OBJ)
	@if [ -z "$(SRC)" ]; then \
		echo "Error: No source files found."; \
		exit 1; \
	fi
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compile .cpp -> .o (with header dependencies)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Include auto-generated dependency files
-include $(DEPS)

# Clean build artifacts
clean:
	$(RM) $(OBJ) $(DEPS) $(TARGET)

# Run the program
run: all
	./$(TARGET)

.PHONY: all clean run
