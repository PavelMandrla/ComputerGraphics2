#pragma once

#include <string>
#include "surface.h"
#include "material.h"
#include "texture.h"
#include <memory>

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

	std::shared_ptr<Texture3f> background;

public:
	Scene(std::string &file_name, std::string &background_file);

	std::vector<MyVertex> getVerticies() { return this->verticies; }

	int getVertexStride() { return this->vertex_stride; };

	Texture3f getPerfectIrradianceMap(int width, int height);

	Texture3f getIrradianceMap(int width, int height);

	Texture3f getPrefilteredEnvMap(float alpha, int width, int height);

	Texture3f getIntegrationMap(int width, int height);

};

