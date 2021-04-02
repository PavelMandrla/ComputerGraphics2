#pragma once

#include "vector3.h"
#include "Scene.h"
#include <memory>
#include "matrix4x4.h";
#include "camera.h"
#include "Directional.h"
#include "ShaderProgram.h"

class Rasterizer {
private:
	bool check_gl(const GLenum error);

	std::shared_ptr<Scene> scene;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Directional> light;

	//INIT DEVICE
	GLFWwindow* window;

	std::shared_ptr<ShaderProgram> mainShader;
	std::shared_ptr<ShaderProgram> shadowMappingShader;

	//INIT BUFFERS
	GLuint vao, vbo;
	
	//SHADOW MAP
	GLuint fbo_shadow_map { 0 };  // shadow mapping FB
	GLuint tex_shadow_map { 0 };  // shadow map texture

	void initShadowProgram();

public:
	Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt);

	~Rasterizer();

	/// <summary>
	/// Initialization of OpenGL context
	/// </summary>
	/// <returns></returns>
	int initDevice();

	/// <summary>
	/// Initialization of VS and FS shaders
	/// </summary>
	void initPrograms();

	/// <summary>
	/// Loads the scene geometry
	/// </summary>
	void loadScene(std::string file_name, std::string background_file);

	/// <summary>
	/// Initialization of VAO and VBO buffers
	/// </summary>
	void initBuffers();

	/// <summary>
	/// Initialize shadow (depth) map texture and framebuffer for the first 
	/// </summary>
	/// <returns></returns>
	int InitShadowDepthBuffer();

	void resize(const int width, const int height);

	void mainLoop();

	std::shared_ptr<Camera> getCamera() { return this->camera; };
};

