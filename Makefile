SENTENCEPIECE ?= 0

ifneq ($(OS),Windows_NT)
    TARGET = tokkit

    CXX ?= g++
    LINKER = $(CXX)

    COMMON_FLAGS = -O3 -DNDEBUG -ffast-math -funroll-loops -pthread -std=c++20 -Iincl -march=native -flto -MMD -MP
    COMMON_LDFLAGS = -pthread -flto

    FLAGS = $(COMMON_FLAGS)
    LDFLAGS = $(COMMON_LDFLAGS)

ifeq ($(SENTENCEPIECE),1)
    SP_CFLAGS := $(shell pkg-config --cflags sentencepiece 2>/dev/null)
    SP_LIBS := $(shell pkg-config --libs sentencepiece 2>/dev/null)
    ifeq ($(strip $(SP_LIBS)),)
        SP_CFLAGS :=
        SP_LIBS := -lsentencepiece -lprotobuf
    endif
    FLAGS += -DSENTENCEPIECE=1 $(SP_CFLAGS)
    LDFLAGS += $(SP_LIBS)
endif

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

    COMMON_FLAGS = /O2 /DNDEBUG /EHsc /std:c++20 /I"incl" /wd4819 /MD
    COMMON_LDFLAGS = /link /MACHINE:X64

    FLAGS = $(COMMON_FLAGS)
    LDFLAGS = $(COMMON_LDFLAGS)

ifeq ($(SENTENCEPIECE),1)
    VCPKG_ROOT := C:/vcpkg
    SP_DIR := $(VCPKG_ROOT)/installed/x64-windows

    PROTOBUF_LIBS := $(wildcard $(SP_DIR)/lib/libprotobuf*.lib)
    ABSL_MONO := $(wildcard $(SP_DIR)/lib/abseil_dll.lib)
    ABSL_PARTS := $(wildcard $(SP_DIR)/lib/absl_*.lib)

    FLAGS += /DSENTENCEPIECE=1 /I"$(SP_DIR)/include"
    LDFLAGS += /LIBPATH:"$(SP_DIR)/lib" sentencepiece.lib $(PROTOBUF_LIBS) $(ABSL_MONO) $(ABSL_PARTS)
endif

    SRCS = tokkit.cpp
    OBJS = tokkit.obj

    .PHONY: all clean

    all: $(TARGET)

    $(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) /OUT:$(TARGET)

    tokkit.obj: tokkit.cpp
	$(CXX) $(FLAGS) /c $< /Fo$@

    clean:
	-del /Q $(TARGET) $(OBJS) 2>nul
endif
