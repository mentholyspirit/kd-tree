CXX ?= g++
LD := $(CXX)
CXXFLAGS := -std=c++17 -pedantic -Werror
CXXFLAGS += -O3

LDFLAGS :=  -L/usr/local/opt/opencv@2/lib -lopencv_core -lopencv_highgui
CXXFLAGS += -I/usr/local/opt/opencv@2/include

ALL_SOURCES := $(wildcard *.cpp)
ALL_HEADERS := $(wildcard *.h)
MAIN_CPP := kd_tree_raytracer.cpp
TEST_CPP := unit_test.cpp
MAIN_EXECUTABLE := kd_tree_raytracer
TEST_EXECUTABLE := unit_test
UNIT_TEST_SOURCES := $(filter-out $(MAIN_CPP), $(ALL_SOURCES))
MAIN_SOURCES := $(filter-out $(TEST_CPP), $(ALL_SOURCES))

.PHONY : build
build : $(MAIN_SOURCES) $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(MAIN_SOURCES) -o $(MAIN_EXECUTABLE)

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
