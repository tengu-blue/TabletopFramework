#version 430 core

uniform sampler2D u_texture;

in vec2 uv_pos; 

out vec4 out_fragColor;

void main() {

	out_fragColor = texture(u_texture, uv_pos);
}

