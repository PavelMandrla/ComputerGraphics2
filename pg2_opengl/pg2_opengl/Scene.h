#pragma once

#include <string>
#include "surface.h"
#include "material.h"
#include "texture.h"
#include <memory>
#include <map>

typedef struct MyVertex{
	float x, y, z;			// position
	float n_x, n_y, n_z;	// normal
	float t_x, t_y, t_z;	// tangent
	float u, v;				// texture coordinates
	int material_index;
};

#pragma pack(push, 1)
struct GLMaterial {
	// DIFFUSE
	Color3f albedo;
	GLbyte pad0[4];
	GLuint64 tex_diffuse_handle{ 0 };
	GLbyte pad1[8];

	// ROUGHNESS, METALNESS, ALPHA
	Color3f rma;	// alpha = 1
	GLbyte pad2[4];
	GLuint64 tex_rma_handle{ 0 };
	GLbyte pad3[8];

	// NORMALS
	Color3f normal;	// (0,0,1) if bump map is not present
	GLbyte pad4[4];
	GLuint64 tex_normal_handle{ 0 };
	GLbyte pad5[8];
};
#pragma

class Scene {
private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;

	std::vector<GLMaterial> materials;
	std::vector<MyVertex> verticies;

	std::map<Material*, int> pointerIndexMap;

	int vertex_stride;

	std::shared_ptr<Texture3f> background;

	void loadMaterials();
	void loadTextures();


public:
	Scene(std::string &file_name, std::string &background_file);

	std::vector<MyVertex> getVerticies() { return this->verticies; }

	std::vector<GLMaterial> getMaterials() { return this->materials; }

	int getVertexStride() { return this->vertex_stride; };

	Texture3f getPerfectIrradianceMap(int width, int height);

	Texture3f getIrradianceMap(int width, int height);

	Texture3f getPrefilteredEnvMap(float alpha, int width, int height);

	Texture3f getIntegrationMap(int width, int height);

	

};

