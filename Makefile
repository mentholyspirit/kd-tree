CXX ?= g++
LD := $(CXX)
CXXFLAGS := -std=c++17 -pedantic -Werror
CXXFLAGS += -O3

ALL_SOURCES := $(wildcard *.cpp)
ALL_HEADERS := $(wildcard *.h)
MAIN_CPP := hmq_test.cpp
TEST_CPP := unit_test.cpp
MAIN_EXECUTABLE := hmq_test
TEST_EXECUTABLE := unit_test
UNIT_TEST_SOURCES := $(filter-out $(MAIN_CPP), $(ALL_SOURCES))
MAIN_SOURCES := $(filter-out $(TEST_CPP), $(ALL_SOURCES))

.PHONY : build
build : $(MAIN_SOURCES) $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(MAIN_SOURCES) -o $(MAIN_EXECUTABLE)

.PHONY : build_test
build_test : $(UNIT_TEST_SOURCES) $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(UNIT_TEST_SOURCES) -o $(TEST_EXECUTABLE)

.PHONY : test
test : build_test
	./unit_test

.PHONY : all
all : build run_test

.PHONY : clean
clean :
	rm -f *.o $(MAIN_EXECUTABLE) $(TEST_EXECUTABLE)
