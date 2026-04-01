#pragma once

constexpr int DEFAULT_ARRAY_SIZE = 256;
constexpr float UNIT_SCALING = 100.0f;

constexpr float MIN_ZOOM = 1.0f;
constexpr float MAX_ZOOM = 250.0f;

constexpr float MIN_ZOOM_SPEED = 2.0f;
constexpr float MAX_ZOOM_SPEED = 20.0f;

// faster the closer we are
constexpr float ZOOM_FACTOR = (MAX_ZOOM_SPEED - MIN_ZOOM_SPEED) / ((MAX_ZOOM - MIN_ZOOM) * (MAX_ZOOM - MIN_ZOOM));

constexpr float SCALE_MULTIPLIER = 1.0f;


constexpr const char mainVertexShader[] = R"(#version 430 core

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

)";

constexpr const char mainFragmentShader[] = R"(#version 430 core

uniform sampler2D u_texture;

in vec2 uv_pos; 

out vec4 out_fragColor;

void main() {

	out_fragColor = texture(u_texture, uv_pos);
}

)";

constexpr const char selectionVertexShader[] = R"(#version 430 core

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
)";

constexpr const char selectionFragmentShader[] = R"(#version 430 core

uniform vec2 u_selectionCenter;
uniform float u_selectionRadius;

uniform float u_time;

in vec2 worldPos;

out vec4 out_fragColor;

void main() {
	
	vec2 d = worldPos - u_selectionCenter;
	float dist = d.x*d.x + d.y * d.y;
	float s = abs(dist- (u_selectionRadius + 0.05 * sin(5*u_time)) );

	// for dashed line 
	//int a = int(acos(dot(normalize(d), vec2(0, 1))) * 57.3) + 6;
	//if(s < 0.06 && (a / 12) % 2 == 0 )
	if(s < 0.06)
		out_fragColor = vec4((1-s*7), (1-s*7)*0.4, 0, 1);
	else
		out_fragColor = vec4(0, 0, 0, 0);
}

)";