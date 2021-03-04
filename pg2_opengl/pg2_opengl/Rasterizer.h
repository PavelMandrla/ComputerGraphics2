#pragma once

#include "vector3.h"
#include "Scene.h"
#include <memory>

class Rasterizer {
private:
	int width, height;
	float fovY;
	Vector3 viewFrom, viewAt;

	char* LoadShader(const char* file_name);
		
	GLint CheckShader(const GLenum shader);

	bool check_gl(const GLenum error);

	std::shared_ptr<Scene> scene;

	//INIT DEVICE
	GLFWwindow* window;

	//INIT PROGRAMS
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;

	//INIT BUFFERS
	GLuint vao, vbo, ebo;

public:
	Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float something, float somethingElse); //TODO -> fix something and somethingElse params

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
	void loadScene(std::string &file_name);

	/// <summary>
	/// Initialization of VAO and VBO buffers
	/// </summary>
	void initBuffers();

	void mainLoop();
};

