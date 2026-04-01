#version 430 core

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

