#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texcoord;
layout (location = 4) in int material_index; 

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 mvn;
uniform mat4 mlp;

uniform vec3 cameraPos;

out vec3 v_normal;
out vec3 v_tangent;

out vec3 position_lcs;
out vec3 omega_o_es;
out vec3 omega_o;
out vec2 texCoord;

flat out int mat_index;

void main( void ) {
	v_normal = normalize(normal);
	v_tangent = normalize(tangent);

	vec4 hit_es = mv * vec4(position.xyz, 1.0f);
	vec3 omega_i_es = normalize( hit_es.xyz / hit_es.w );
	//if (dot(unified_normal_es, omega_i_es) > 0.0f) {
	//	unified_normal_es *= -1.0f;
	//}
	
	gl_Position = mvp * vec4(position, 1.0f);

	//SHADOW
	vec4 tmp	= mlp * vec4(position, 1.0f);
	position_lcs = tmp.xyz / tmp.w;

	//CAMERA DIR
	omega_o_es = -omega_i_es;
	omega_o = normalize(position - cameraPos);

	texCoord = texcoord;
	mat_index = material_index;
}
