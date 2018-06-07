// This code is "Public Domain", no rights reserved.

#include "ply_reader.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace std;

std::unique_ptr<PLY_Model> Read_PLY_Model(const char *filename)
{
	FILE *file = fopen(filename, "r");

	// Error checks omitted for simplicity.
	assert(file);

	// Parse header
	char header_field[1024] = "\0";
	int  vertex_count = 0;
	int  face_count = 0;

	while (strcmp(header_field, "end_header"))
	{
		fscanf(file, "%s", header_field);

		if (!strcmp(header_field, "vertex"))
			fscanf(file, "%d", &vertex_count);

		if (!strcmp(header_field, "face"))
			fscanf(file, "%d", &face_count);
	}

	// Construct the target buffers
	std::unique_ptr<PLY_Model> res = std::make_unique<PLY_Model>();
	std::vector<float> vertex_data;
	vertex_data.reserve(vertex_count);
	res->triangles.reserve(face_count);
	res->triangleNormals.reserve(face_count);

	// Read vertex data
	for (int i = 0; i < vertex_count * 3; ++i)
	{
		float val = 0;
		fscanf(file, "%f", &val);
		vertex_data.push_back(val);
		res->aabb.min[(Axis)(i % 3)] = std::min(val, res->aabb.min[(Axis)(i % 3)]);
		res->aabb.max[(Axis)(i % 3)] = std::max(val, res->aabb.max[(Axis)(i % 3)]);
	}

	// Read face (triangles) data
	for (int i = 0; i < face_count; ++i)
	{
		int val = 0;
		fscanf(file, "%d", &val);
		// Beginning of the face data row, assert that we have triangles
		assert(val == 3);
		int pa, pb, pc;
		fscanf(file, "%d %d %d", &pa, &pb, &pc);
		Triangle t = Triangle(
				Vector3(vertex_data[pa * 3], vertex_data[pa * 3 + 1], vertex_data[pa * 3 + 2]),
				Vector3(vertex_data[pb * 3], vertex_data[pb * 3 + 1], vertex_data[pb * 3 + 2]),
				Vector3(vertex_data[pc * 3], vertex_data[pc * 3 + 1], vertex_data[pc * 3 + 2]));
		res->triangles.push_back(t);
		res->triangleNormals.push_back(t.GetNormal());
	}

	fclose(file);

	return res;
}
