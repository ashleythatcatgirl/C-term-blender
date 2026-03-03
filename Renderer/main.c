#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Include/glad.c"
#include "main.h"

#define STB_IMAGE_IMPLEMENTATION

#include "Include/stb_image.h"
#include <stdio.h>
#include <stdbool.h>
//#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

const unsigned int WINDOW_WIDTH = 960;
const unsigned int WINDOW_HEIGHT = 540;

int main(int argc, char **argv) {
	struct Input input;
	input.options = 0;

	struct Textures textures;
	textures.count = 0;
	textures.texture = NULL;

	struct Objects objects;
	objects.count = 0;
	objects.object = NULL;
	
	ParseArgs(argc, argv, &input);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	printf("Window initializiation..\n");
	///CREATING WINDOW ///
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "C-blender", NULL, NULL);

	if (window == NULL) {
		printf("Window creation fail\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("GLAD fail\n");
		glfwTerminate();
		return -1;
	}  


	printf("Compiling shaders..\n");

	const char *vertexShaderSource = GetShaderContent("Shaders/Vertex/normal.glsl");
	//printf("%s\n", vertexShaderSource);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
        	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	       	printf("Vertex shader fail%s", infoLog);
    	}

	const char *fragmentShaderSource = GetShaderContent("Shaders/Fragment/mat1.glsl");
	//printf("%s\n", fragmentShaderSource);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
        	glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        	printf("Fragment shader fail%s", infoLog);
    	}

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    	if (!success) {
        	glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        	printf("Shader to program linking fail\n %s", infoLog);
    	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	printf("Initializing object vertex data..\n");
	switch (LoadObjects(&objects)) {
		case 0:
			printf("\nObjects loaded\n");
			break;
		case 1:
			printf("\nNo objects found\n");
			break;
		case -1:
			printf("\nObject loading failed\n");
			glfwTerminate();
			return 1;
	}

	printf("Initializing texture data..\n");
	switch (LoadTextures(&textures, shaderProgram)) {
		case 0:
			printf("\nTextures loaded\n");
			break;
		case 1:
			printf("\nNo textures found\n");
			break;
		case -1:
			printf("\nTexture loading failed\n");
			glfwTerminate();
			return 1;
	}

	printf("Loading successful, press enter to continue..");
	getchar();

	/// MAIN LOOP ///
	printf("Opened window, press ESC to exit\n");
	RenderLoop(window, shaderProgram, &input, &objects, &textures);
	
	/// END ///
	printf("Closed window\n");
	printf("Exiting program...\n");

    	glDeleteProgram(shaderProgram);

	glfwTerminate();

	FreeMemory(&objects, &textures);

	return 0;
}

int RenderLoop(GLFWwindow *window, unsigned int shaderProgram, Input *input, Objects *objects, Textures *textures) {
	// WIREFRAME
	if (input->options == 1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	while(!glfwWindowShouldClose(window)) {	
		processInput(window);

		glClearColor(0.1, 0.1, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int tex = 0; tex < textures->count; tex++) {
			glActiveTexture(GL_TEXTURE0 + tex);
			glBindTexture(GL_TEXTURE_2D, textures->texture[tex].memory);
		}

		glUseProgram(shaderProgram);

		for (int obj = 0; obj < objects->count; obj++) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects->object[obj].EBO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}

void processInput(GLFWwindow *window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

char* GetShaderContent(const char* fileName) {
	char buffer = 0;
	char* content = 0;
	int size = 1024;

	content = malloc(sizeof(char) * size);
	if (content == NULL) return 0;

	FILE *fPtr = fopen(fileName, "r");
	if (fPtr == NULL) return 0;

	int i = 0;
	for (; (buffer = fgetc(fPtr)) != EOF; i++) {
		if (i >= size) {
			size *= 2;

			char* temp = realloc(content, sizeof(char) * size);
			if (temp == NULL) return 0;

			content = temp;
		}
		
		content[i] = buffer;
	}

	content[i] = '\0';
	
	return content;
}

int LoadObjects(Objects *objects) {
	printf("\n->Reading object data..\n");
	struct dirent *de;
	DIR *dr = opendir("Objects/");
	if (dr == NULL) return 1;
	printf("  ->Found object folder!\n");

	char *end = 0;
	while ((de = readdir(dr)) != NULL) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;

		if ((end = strchr(de->d_name, '.')) == NULL) continue;
		if (strcmp(end, ".obj")) continue;

		objects->count++;
	}
	
	if (objects->count == 0) return 1;

	objects->object = malloc(sizeof(Object) * objects->count);
	if (objects->object == NULL) return -1;
	for (int obj = 0; obj < objects->count; obj++) {
		objects->object[obj].vCount = 0;
		
		objects->object[obj].vArray = NULL;
		objects->object[obj].iArray = NULL;

		objects->object[obj].VBO = 0;
		objects->object[obj].VAO = 0;
		objects->object[obj].EBO = 0;
	}

	dr = opendir("Objects/");

	for (int obj = 0; (de = readdir(dr)) != NULL;) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;

		if ((end = strchr(de->d_name, '.')) == NULL) continue;
		if (strcmp(end, ".obj")) continue;

		printf("    ->Objects[%d]: %s\n", obj, de->d_name);

		//ParseOBJFile();
	}

	closedir(dr);

	printf("\n->Setting object data..\n");
	for (int obj = 0; obj < objects->count; obj++) {
		printf("  ->Object[%d]\n", obj);
		glGenBuffers(1, &objects->object[obj].VBO);
		glGenVertexArrays(1, &objects->object[obj].VAO);
		glGenBuffers(1, &objects->object[obj].EBO);

		glBindBuffer(GL_ARRAY_BUFFER, objects->object[obj].VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(objects->object[obj].vArray), objects->object[obj].vArray, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects->object[obj].EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(objects->object[obj].iArray), objects->object[obj].iArray, GL_STATIC_DRAW);

		glBindVertexArray(objects->object[obj].VAO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	return 0;
}

int ReadObjectData() {

	return 0;
}

int LoadTextures(struct Textures *textures, unsigned int shaderProgram) {
	printf("\n->Reading texture data..\n");
	struct dirent *de;
	DIR *dr = opendir("Textures/");
	if (dr == NULL) return 1;
	printf("  ->Found texture folder!\n");

	char *end = NULL;
	while ((de = readdir(dr)) != NULL) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;

		if ((end = strchr(de->d_name, '.')) == NULL) continue;
		if (strcmp(end, ".png") && strcmp(end,".jpg")) continue;

		textures->count++;
	}

	closedir(dr);

	if (textures->count == 0) return 1;

	textures->texture = malloc(sizeof(Texture) * textures->count);
	if (textures->texture == NULL) return -1;

	dr = opendir("Textures/");

	for (int tex = 0; (de = readdir(dr)) != NULL;) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;

		if ((end = strchr(de->d_name, '.')) == NULL) continue;
		if (strcmp(end, ".png") && strcmp(end,".jpg")) continue;

		strcpy(textures->texture[tex].name, "Textures/");
		strcat(textures->texture[tex].name, de->d_name);
		printf("    ->Texture[%d]: %s\n", tex, textures->texture[tex].name);
		tex++;
	}

	closedir(dr);

	unsigned char *data;
	int textureWidth, textureHeight, colorChannels;

	printf("\n->Setting texture data..\n");
	for (int i = 0; i < textures->count; i++) {
		printf("  ->Texture[%d]\n", i);
		glGenTextures(1, &textures->texture[i].memory);
		glBindTexture(GL_TEXTURE_2D, textures->texture[i].memory);

		data = stbi_load(textures->texture[i].name, &textureWidth, &textureHeight, &colorChannels, 0);
		if (!data) {
			return -1;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	printf("\n->Linking texture data..\n");
	glUseProgram(shaderProgram);

	char *textureName = malloc(sizeof(char) * 64);
	if (textureName == NULL) return -1;
	char *num = malloc(sizeof(char) * 16);
	if (num == NULL) return -1;

	for (int tex = 0; tex < textures->count; tex++) {
		printf("  ->Texture[%d]\n", tex);
		strcpy(textureName, "texture");
		sprintf(num, "%d", tex); 
		strcat(textureName, num);
		glUniform1i(glGetUniformLocation(shaderProgram, textureName), 0);
	}

	return 0;
}

int ParseArgs(int argc, char **argv, struct Input *input) {
	if (argc <= 1) return 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "wireframe") == 0) {
			input->options = 1;
		}
		
		printf("%d\n", input->options);
	}

	return 0;
}

void FreeMemory(Objects *objects, Textures *textures) {
	if (objects->object != NULL) {
		for (int obj = 0; obj < objects->count; obj++) {
			if (objects->object[obj].vArray != NULL) {
				free (objects->object[obj].vArray);
			}

			if (objects->object[obj].iArray != NULL) {
				free (objects->object[obj].iArray);
			}

    			glDeleteBuffers(1, &objects->object[obj].VBO);
			glDeleteVertexArrays(1, &objects->object[obj].VAO);
    			glDeleteBuffers(1, &objects->object[obj].EBO);
		}

		free(objects->object);
	}
	if (textures->texture != NULL) {
		free(textures->texture);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
} 
