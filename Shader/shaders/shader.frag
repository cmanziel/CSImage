#version 430 core

layout(location = 0) out vec4 color;
in vec2 out_texCoords;

// sampler for the texture
uniform sampler2D tex;

void main() {
	color = texture(tex, out_texCoords);
}