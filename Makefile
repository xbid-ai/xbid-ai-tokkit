ifneq ($(OS),Windows_NT)
    TARGET = tokkit

    CXX ?= g++
    LINKER = $(CXX)

    COMMON_FLAGS = -O3 -DNDEBUG -ffast-math -funroll-loops -pthread -std=c++20 -Iincl -march=native -flto -MMD -MP
    COMMON_LDFLAGS = -pthread -flto

    FLAGS = $(COMMON_FLAGS)
    LDFLAGS = $(COMMON_LDFLAGS)

    ifeq ($(shell uname),Darwin)
        ifeq ($(shell uname -m),arm64)
            FLAGS += -mcpu=native -mtune=native -fstrict-aliasing -flto=thin
            LDFLAGS := $(filter-out -flto,$(LDFLAGS)) -flto=thin
        endif
    endif

    SRCS = tokkit.cpp
    OBJS = tokkit.o
    DEPS = $(OBJS:.o=.d)

    .PHONY: all clean

    all: $(TARGET)

    $(TARGET): $(OBJS)
	    $(LINKER) -o $@ $(OBJS) $(LDFLAGS)

    tokkit.o: tokkit.cpp
	    $(CXX) $(FLAGS) -c $< -o $@

    clean:
	    rm -f $(TARGET) $(OBJS) $(DEPS)

    -include $(DEPS)
else
    TARGET = tokkit.exe
    CXX = cl

    COMMON_FLAGS   = /O2 /DNDEBUG /EHsc /std:c++20 /I"incl" /wd4819
    COMMON_LDFLAGS = /link

    FLAGS = $(COMMON_FLAGS)
    LDFLAGS = $(COMMON_LDFLAGS)
    SRCS = tokkit.cpp
    OBJS = tokkit.obj

    .PHONY: all clean

    all: $(TARGET)

    $(TARGET): $(OBJS)
	    $(CXX) $(OBJS) $(LDFLAGS) /OUT:$(TARGET)

    tokkit.obj: tokkit.cpp
	    $(CXX) $(FLAGS) /c $< /Fotokkit.obj

    clean:
	    del /Q $(TARGET) $(OBJS)
endif
