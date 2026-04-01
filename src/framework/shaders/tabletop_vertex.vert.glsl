#version 430 core

layout (location = 0) in vec3 in_vert;
layout (location = 1) in vec2 in_uv;

layout (location = 2) in vec3 aPos;
layout (location = 3) in vec2 aDim;

layout (location = 4) in vec2 aUvPos;
layout (location = 5) in vec2 aUvDim;

out vec2 uv_pos;

uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * 
	vec4(in_vert.x * aDim.x + aPos.x, 
		 in_vert.y * aDim.y + aPos.y, 
		 in_vert.z + aPos.z, 1.0);	
	
	uv_pos = vec2(in_uv.x * aUvDim.x + aUvPos.x, in_uv.y * aUvDim.y + aUvPos.y);

}
