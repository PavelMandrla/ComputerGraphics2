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

	GLfloat* getVerticies() { return (GLfloat*) &verticies[0]; }

	unsigned int* getIndicies() { return (unsigned int*) &indices[0]; }

};

