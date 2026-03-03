#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexture;

out vec3 verticiesCords;
out vec3 colorCords;
out vec2 textureCords;

void main() {
	gl_Position = vec4(aPos, 1.0);
	verticiesCords = aPos;
	colorCords = aColor;
	textureCords = aTexture;
}
