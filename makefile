COMPILER = g++
OBJ_DIR := ./obj
UTIL_DIR := ./util
SRC_FILES := $(wildcard $(UTIL_DIR)/*.hpp)

CPPFLAGS = -O3 -fopenmp -std=c++17
all: $(METHOD)

buildCompatWindows: buildCompatWindows.cc util/IO.hpp util/indexItem.hpp util/cw.hpp util/new_utils.hpp
	${COMPILER} ${CPPFLAGS} -o $@ $<

searchDuplicate: searchDuplicate.cc util/IO.hpp util/indexItem.hpp util/cw.hpp util/new_utils.hpp util/query.hpp util/dupSearch/segmentTree.hpp
	${COMPILER} ${CPPFLAGS} -o $@ $<

$(OBJ_DIR)/%.o: $(UTIL_DIR)/%.hpp
	${COMPILER} $(CPPFLAGS) -c -o $@ $<

