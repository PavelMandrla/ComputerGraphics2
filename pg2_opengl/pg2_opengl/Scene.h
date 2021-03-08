#pragma once

#include <string>
#include "surface.h"
#include "material.h"

class Scene {
private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;

	//GLfloat* vertices;
	std::vector<GLfloat> verticies;
	std::vector<unsigned int> indices;

	int no_vertices;
	int vertex_stride;


public:
	Scene(std::string &file_name);

	GLfloat* getVerticies() { return verticies.data(); }

	unsigned int getVerteciesCount() { return verticies.size(); }

	unsigned int* getIndicies() { return indices.data(); }

	unsigned int getIndiciesCount() { return indices.size(); }

	int getVertexStride() { return vertex_stride; };
};

