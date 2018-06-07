// This code is "Public Domain", no rights reserved.

#ifndef PLY_READER_H
#define PLY_READER_H

#include <vector>
#include <memory>
#include <assert.h>
#include <float.h>

#include "triangle.h"

struct PLY_Model
{
	std::vector<Triangle> triangles;
	std::vector<Vector3> triangleNormals;
};

// Note: This is not a general PLY model reader, it works only with the
// data from Stanford 3D Scanning Repository.  For example, the happy
// buddha model from http://graphics.stanford.edu/data/3Dscanrep/ .

std::unique_ptr<PLY_Model> Read_PLY_Model(const char *filename);

#endif
