﻿#include "pch.h"
#include "Rasterizer.h"
#include "utils.h"
#include "glutils.h"
#include "mymath.h"

// OpenGL check state
bool Rasterizer::check_gl(const GLenum error) {
	if ( error != GL_NO_ERROR ) {
		//const GLubyte * error_str;
		//error_str = gluErrorString( error );
		//printf( "OpenGL error: %s\n", error_str );

		return false;
	}
	return true;
} 

// glfw callback
void glfw_callback_1( const int error, const char * description ) {
	printf( "GLFW Error (%d): %s\n", error, description );
}

// OpenGL messaging callback
void GLAPIENTRY gl_callback_1( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * user_param ) {
	printf( "GL %s type = 0x%x, severity = 0x%x, message = %s\n",
		( type == GL_DEBUG_TYPE_ERROR ? "Error" : "Message" ),
		type, severity, message );
}

// invoked when window is resized
void framebuffer_resize_callback_1( GLFWwindow * window, int width, int height ) {
	auto rasterizer = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window));
	rasterizer->resize(width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		auto rasterizer = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window));
		auto camera = rasterizer->getCamera();
		if (key == GLFW_KEY_W) camera->moveForward();
		if (key == GLFW_KEY_S) camera->moveBackward();
		if (key == GLFW_KEY_A) camera->moveLeft();
		if (key == GLFW_KEY_D) camera->moveRight();
	}

}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	//printf("x:%f y:%f\n", xpos, ypos);
	auto camera = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window))->getCamera();
	camera->adjustYaw(xpos);
	camera->adjustPitch(ypos);

	glfwSetCursorPos(window, 0, 0);
}

void Rasterizer::initIrradianceMapTexture() {
	int level_count = 8;

	glGenTextures(1, &tex_irradiance_map);
	glBindTexture(GL_TEXTURE_2D, tex_irradiance_map);

	if (glIsTexture(tex_irradiance_map)) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level_count - 1);

		for (int i = 0; i < level_count; i++) {
			auto tex = Texture3f("D:\\prg\\cpp\\ComputerGraphics2\\data\\background\\ir_map\\" + std::to_string(i) + ".exr");
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGB32F, tex.width(), tex.height(), 0, GL_RGB, GL_FLOAT, tex.data());
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Rasterizer::initPrefilteredEnvMapTexture() {
	int level_count = 8;

	glGenTextures(1, &tex_prefilteredEnv_map);
	glBindTexture(GL_TEXTURE_2D, tex_prefilteredEnv_map);

	if (glIsTexture(tex_prefilteredEnv_map)) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level_count - 1);

		for (int i = 0; i < level_count; i++) {
			auto tex = Texture3f("D:\\prg\\cpp\\ComputerGraphics2\\data\\background\\env_map\\" + std::to_string(i) + ".exr");
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGB32F, tex.width(), tex.height(), 0, GL_RGB, GL_FLOAT, tex.data());
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

Rasterizer::Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt) {
	this->camera = std::make_shared<Camera>(width, height, fovY, viewFrom, viewAt);
	this->light = std::make_shared<Directional>(Vector3 {0, 10, 10}, Vector3 {0, 0, 0}, deg2rad(45), 1024, 1024);
}

Rasterizer::~Rasterizer() {
	glDeleteBuffers( 1, &vbo );
	glDeleteVertexArrays( 1, &vao );

	glfwTerminate();
	delete this->window;
}

int Rasterizer::initDevice() {
	glfwSetErrorCallback( glfw_callback_1 );

	if (!glfwInit()) return( EXIT_FAILURE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_SAMPLES, 8 );
	glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GL_TRUE );

	this->window = glfwCreateWindow( camera->getWidth(), camera->getHeight(), "PG2 OpenGL", nullptr, nullptr );
	if (!this->window) {
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glfwSetWindowUserPointer(this->window, reinterpret_cast<void*>(this));
	glfwSetKeyCallback(this->window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback( window, framebuffer_resize_callback_1 );
	glfwMakeContextCurrent( window );

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress )) {
		if (!gladLoadGL()) return EXIT_FAILURE;
	}

	glEnable( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( gl_callback_1, nullptr );

	printf( "OpenGL %s, ", glGetString( GL_VERSION ) );
	printf( "%s", glGetString( GL_RENDERER ) );
	printf( " (%s)\n", glGetString( GL_VENDOR ) );
	printf( "GLSL %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	//check_gl();//wtf

	glEnable( GL_MULTISAMPLE );

	// map from the range of NDC coordinates <-1.0, 1.0>^2 to <0, width> x <0, height>
	glViewport(0, 0, camera->getWidth(), camera->getHeight());
	// GL_LOWER_LEFT (OpenGL) or GL_UPPER_LEFT (DirectX, Windows) and GL_NEGATIVE_ONE_TO_ONE or GL_ZERO_TO_ONE
	glClipControl( GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE );
}

void Rasterizer::initPrograms() {	///řeší vytvoření vertex a fragment shader
	this->mainShader = std::make_shared<ShaderProgram>("basic_shader.vert", "basic_shader.frag");
	this->shadowMappingShader = std::make_shared<ShaderProgram>("shadowMap_shader.vert", "shadowMap_shader.frag");
}

inline float f(float x) {
	return -999.999f * x + 1000.0f;
}

void Rasterizer::loadScene(std::string file_name, std::string background_file) {
	this->scene = std::make_shared<Scene>(file_name, background_file);
	
	/*int width = 1024;
	for (int i = 0; i < 8; i++, width /= 2) {
		float alpha = f(float(i) / 7.0f);
		scene->getPrefilteredEnvMap(alpha, width, width / 2).Save("D:\\prg\\cpp\\ComputerGraphics2\\data\\background\\env_map\\" + std::to_string(i) + ".exr");
		scene->getIrradianceMap(alpha, width, width / 2).Save("D:\\prg\\cpp\\ComputerGraphics2\\data\\background\\ir_map\\" + std::to_string(i) + ".exr");
	}*/
}

void Rasterizer::initBuffers() {	
	this->vao = 0; ///VAO BUFFER = popisuje konfiguraci bufferů, které budou v rámci objektu vystupovat
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao ); //timhle se ten buffer nabinduje k vao

	this->vbo = 0;	//vertex buffer
	glGenBuffers( 1, &vbo ); // generate vertex buffer object (one of OpenGL objects) and get the unique ID corresponding to that buffer
	glBindBuffer( GL_ARRAY_BUFFER, vbo ); // bind the newly created buffer to the GL_ARRAY_BUFFER target

	auto vertices = this->scene->getVerticies();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyVertex), vertices.data(), GL_STATIC_DRAW);
	
	// vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, scene->getVertexStride(), 0);	//popiseme, jaka je struktura toho vertex bufferu = na nultem indexu jsou veci velikosti 3 (x, y, z) typu float a nechceme je normalizovat, stride je pocet bytu, ktere lezi mezi dvema nasledujicimi zaznamy | posledni je offset od zacatku pole
	glEnableVertexAttribArray(0);	

	// vertex position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, scene->getVertexStride(), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, scene->getVertexStride(), (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);
	
	// vertex texture coordinates		
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, scene->getVertexStride(), (void*) (sizeof(float) * 9));
	glEnableVertexAttribArray(3); //kazdy index, ktery popiseme, musime zenablovat
	
	this->initIrradianceMapTexture();
	this->initPrefilteredEnvMapTexture();
	this->initShadowDepthBuffer();
}

int Rasterizer::initShadowDepthBuffer() {
	glGenTextures(1, &tex_shadow_map); // texture to hold the depth values from the light's perspective
	glBindTexture(GL_TEXTURE_2D, tex_shadow_map);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->light->getWidth(), this->light->getHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	const float color[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // areas outside the light's frustum will be lit
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glGenFramebuffers(1, &fbo_shadow_map); // new frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow_map);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_shadow_map, 0); // attach the texture as depth
	glDrawBuffer(GL_NONE); // we dont need any color buffer during the first pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer back

	return 0;
}

void Rasterizer::resize(const int width, const int height) {
	glViewport(0, 0, width, height);
	camera->update(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteRenderbuffers(1, &tex_shadow_map);
	glDeleteFramebuffers(1, &fbo_shadow_map);
	
	initBuffers();
}

void Rasterizer::mainLoop() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable( GL_DEPTH_TEST ); // zrusi pouziti z-bufferu, vykresleni se provede bez ohledu na poradi fragmentu z hlediska jejich pseudohloubky
	glEnable( GL_CULL_FACE ); // zrusi zahazovani opacne orientovanych ploch

	while (!glfwWindowShouldClose(this->window)) {		
		#pragma region ---first pass ---

		// set the shadow shaderprogram and the viewport to match the size of the depth map
		glUseProgram(this->shadowMappingShader->program);
		glViewport(0, 0, light->getWidth(), light->getHeight());
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow_map);
		glClear(GL_DEPTH_BUFFER_BIT);

		// set up the light source through the MLP matrix
		SetMatrix4x4(shadowMappingShader->program, (GLfloat*) light->getMVP().data(), "mlp");

		// draw the scene
		glBindVertexArray(this->vao);
		glDrawArrays(GL_TRIANGLES, 0, scene->getVerticies().size());
		glBindVertexArray(0);

		// set back the main shaderprogram and the viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, camera->getWidth(), camera->getHeight());
		glUseProgram(mainShader->program);

		#pragma endregion
		
		glClearColor( 0.2f, 0.3f, 0.3f, 1.0f ); // state setting function
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); // state using function

		glBindVertexArray( vao );

		SetMatrix4x4(mainShader->program, (GLfloat*) camera->getMVP().data(), "mvp");
		SetMatrix4x4(mainShader->program, (GLfloat*) camera->getMV().data(), "mv");
		SetMatrix4x4(mainShader->program, (GLfloat*) camera->getMVn().data(), "mvn");
		SetMatrix4x4(mainShader->program, (GLfloat*) light->getMVP().data(), "mlp");

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_irradiance_map);
		SetSampler(mainShader->program, 1, "irradiance_map");

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex_prefilteredEnv_map);
		SetSampler(mainShader->program, 1, "prefilteredEnv_map");

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, tex_shadow_map);

		SetSampler(mainShader->program, 3, "shadow_map");

		glDrawArrays( GL_TRIANGLES, 0, this->scene->getVerticies().size() );
		//glDrawArrays( GL_POINTS, 0, 6 );
		//glDrawArrays( GL_LINE_LOOP, 0, this->scene->getVerticies().size() );
		//glDrawElements( GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0 ); // optional - render from an index buffer

		glfwSwapBuffers( window );
		glfwPollEvents();
	}
}
