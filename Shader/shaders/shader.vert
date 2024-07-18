#version 430 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoords;

out vec2 out_texCoords;

void main() {
	gl_Position = vec4(pos, 0.0, 1.0);

	out_texCoords = texCoords;
}