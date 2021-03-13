#pragma once

#include <string>
#include "surface.h"
#include "material.h"

//p0.x, p0.y, p0.z, t0.u, t0.v
typedef struct MyVertex{
	float x, y, z, u, v;
};

class Scene {
private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;

	std::vector<MyVertex> verticies;

	int vertex_stride;

public:
	Scene(std::string &file_name);

	std::vector<MyVertex> getVerticies() { return this->verticies; }

	int getVertexStride() { return this->vertex_stride; };
};

