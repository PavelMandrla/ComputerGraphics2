#include "pch.h"
#include "glutils.h"

void SetMatrix4x4( const GLuint program, const GLfloat * data, const char * matrix_name ) {	
	const GLint location = glGetUniformLocation( program, matrix_name );

	if ( location == -1 ) {
		printf( "Matrix '%s' not found in active shader.\n", matrix_name );
	} else {
		glUniformMatrix4fv( location, 1, GL_TRUE, data );
	}
}

void SetVec3(const GLuint program, const GLfloat* data, const char* vecName) {
	const GLint location = glGetUniformLocation(program, vecName);

	if (location == -1) {
		printf("Vector '%s' not found in active shader.\n", vecName);
	} else {
		//glUniformMatrix4fv(location, 1, GL_TRUE, data);
		glUniform3fv(location, 1, data);
	}
}

void SetSampler(const GLuint program, GLenum texture_unit, const char* sampler_name) {
	const GLint location = glGetUniformLocation(program, sampler_name);

	if (location == -1) {
		printf("Texture sampler '%s' not found in active shader.\n", sampler_name);
	} else {
		glUniform1i(location, texture_unit);
	}
}

void CreateBindlessTexture(GLuint& texture, GLuint64& handle, const int width, const int height, const GLvoid* data) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// copy data from the host buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	handle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(handle);
}
