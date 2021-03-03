#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

void main( void )
{
	//gl_Position = vec4( position.x, -position.y, position.z, 1.0f );
	gl_Position = vec4( position, 1.0f );
}
