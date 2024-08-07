CC := g++
# Compiler flags
CFLAGS := -Wall -Wno-narrowing -fPIC -std=c++20 -Iinclude 

# Directories
SRCDIR := src
BUILDDIR := build
LIBDIR := lib

# Files to compile
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

# Output shared library name
LIBNAME := libhook.so

# Targets
all: $(LIBDIR)/$(LIBNAME)

# Rule to build object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build shared library
$(LIBDIR)/$(LIBNAME): $(OBJS)
	@mkdir -p $(LIBDIR)
	$(CC) -shared  -fPIC -o $@ $^ -Llib -ldl -ljsoncpp -lZydis 

# Clean command
clean:
	rm -rf $(BUILDDIR) $(LIBDIR)/*.so

# Phony targets
.PHONY: all clean
