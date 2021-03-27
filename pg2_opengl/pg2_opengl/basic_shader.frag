#version 460 core

in vec3 unified_normal_es;
out vec4 FragColor;

void main( void ) {
	vec3 color  = (unified_normal_es + 1) / 2;
	FragColor = vec4( color.xyz, 1.0f );
}
