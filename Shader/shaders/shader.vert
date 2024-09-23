#version 430 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoords;

layout(std430, binding = 3) buffer texShift
{
	vec2 tex_shift;
};

out vec2 out_texCoords;

void main() {
	gl_Position = vec4(pos + tex_shift, 0.0, 1.0);

	out_texCoords = texCoords;
}