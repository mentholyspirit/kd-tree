SOURCES := hmq_test.cpp ply_reader.cpp
OBJS := $(SOURCES:.cpp=.o)
CXXFLAGS += -std=c++17

all: hmq_test

hmq_test: $(OBJS)
	$(CXX) $(CXXFLAGS) -o hmq_test $(OBJS) $(LFLAGS) $(LIBS)