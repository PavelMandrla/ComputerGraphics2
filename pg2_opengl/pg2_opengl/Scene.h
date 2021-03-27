#pragma once

#include <string>
#include "surface.h"
#include "material.h"
#include "SphericalMap.h"
#include "texture.h"

typedef struct MyVertex{
	float x, y, z;			// position
	float n_x, n_y, n_z;	// normal
	float t_x, t_y, t_z;	// tangent
	float u, v;				// texture coordinates
};

class Scene {
private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;

	std::vector<MyVertex> verticies;

	int vertex_stride;

	SphericalMap background;

public:
	Scene(std::string &file_name, std::string &background_file);

	std::vector<MyVertex> getVerticies() { return this->verticies; }

	int getVertexStride() { return this->vertex_stride; };

	Texture3f getIrradianceMap(); //TODO
};

