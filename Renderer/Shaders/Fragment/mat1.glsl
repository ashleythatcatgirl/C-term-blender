#version 450 core
out vec4 FragColor;

in vec3 verticiesCords;
in vec2 textureCords;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

void main() {
	FragColor = vec4(texture(texture2, textureCords));
}
