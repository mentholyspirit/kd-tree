SOURCES := hmq_test.cpp ply_reader.cpp tga_saver.cpp raytracer.cpp aabb.cpp kdtree.cpp kdnode.cpp
HEADERS := ply_reader.h tga_saver.h vector3.h triangle.h ray.h raytracer.h aabb.h kdtree.h kdnode.h
OBJS := $(SOURCES:.cpp=.o)
CXXFLAGS += -std=c++17 -Os

all: hmq_test

hmq_test: $(OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o hmq_test $(OBJS) $(LFLAGS) $(LIBS)
