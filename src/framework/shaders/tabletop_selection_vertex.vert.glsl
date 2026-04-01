#version 430 core

layout (location = 0) in vec3 in_vert;
layout (location = 1) in vec3 aPos;
layout (location = 2) in vec2 aDim;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 worldPos;

void main()
{
	vec4 p = vec4(in_vert.x * aDim.x + aPos.x, 
				  in_vert.y * aDim.y + aPos.y, 
				  in_vert.z + aPos.z, 1.0);

	gl_Position = u_projection * u_view * p;
	worldPos = vec2(p.x, p.y);

}
