#version 450 core
out vec4 FragColor;

in vec3 verticiesCords;
in vec3 colorCords;
in vec2 textureCords;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
	FragColor = mix(texture(texture1, textureCords), texture(texture2, textureCords), 0.2);
}
