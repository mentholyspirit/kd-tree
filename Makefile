SOURCES := hmq_test.cpp ply_reader.cpp tga_saver.cpp raytracer.cpp
HEADERS := ply_reader.h vector3.h triangle.h raytracer.h
OBJS := $(SOURCES:.cpp=.o)
CXXFLAGS += -std=c++17 -Os

all: hmq_test

hmq_test: $(OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o hmq_test $(OBJS) $(LFLAGS) $(LIBS)
