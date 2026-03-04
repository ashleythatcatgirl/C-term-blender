#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Include/glad.c"
#include "main.h"
#include "verticies.c"

#define STB_IMAGE_IMPLEMENTATION

#include "Include/stb_image.h"
#include "Include/cglm/cglm.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

const unsigned int INIT_WIDTH = 960;
const unsigned int INIT_HEIGHT = 640;

int main(int argc, char **argv) {
	struct Window window;
	window.width = INIT_WIDTH;
	window.height = INIT_HEIGHT;
	window.frame = NULL;

	struct Input input;
	input.options = 0;

	struct Textures textures;
	textures.count = 0;
	textures.texture = NULL;

	struct Objects objects;
	objects.count = 0;
	objects.object = NULL;

	struct Model model;
	model.vCount = 36;
	model.verticies = verticies;
	model.indices = indices;

	struct Transforms transforms;
	transforms.modelLoc = 0;
	transforms.viewLoc = 0;
	transforms.projectionLoc = 0;
	
	ParseArgs(argc, argv, &input);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	printf("Window initializiation..\n");
	window.frame = glfwCreateWindow(window.width, window.height, "C-term blender", NULL, NULL);

	if (window.frame == NULL) {
		printf("Window creation fail\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window.frame);
	glfwSetFramebufferSizeCallback(window.frame, framebuffer_size_callback);  

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("GLAD fail\n");
		glfwTerminate();
		return -1;
	}  


	printf("Compiling shaders..\n");

	const char *vertexShaderSource = GetShaderContent("Shaders/Vertex/normal1.glsl");
	if (vertexShaderSource == 0) return -1;
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
	if (fragmentShaderSource == 0) return -1;
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

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &model.VAO);
	glGenBuffers(1, &model.VBO);
	glGenBuffers(1, &model.EBO);

	glBindVertexArray(model.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), model.verticies, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(model.indices), model.indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	/*switch (LoadObjects(&objects)) {
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
	*/
	
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
	

	glUseProgram(shaderProgram);

	printf("transformations.. \n");
	printf("Loading successful, press enter to continue..");
	getchar();

	int width, height;
	printf("Opened window, press ESC to exit\n");
	RenderLoop(&window, shaderProgram, &input, &model, &textures, &transforms);

	printf("Closed window\n");
	printf("Exiting program...\n");

    	glDeleteProgram(shaderProgram);

	glfwTerminate();

	FreeMemory(&objects, &textures);

	return 0;
}

int RenderLoop(Window *window, unsigned int shaderProgram, Input *input, Model *model, Textures *textures, Transforms *transforms) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (input->options == 1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	while(!glfwWindowShouldClose(window->frame)) {	
		glfwGetFramebufferSize(window->frame, &window->width, &window->height);
		processInput(window->frame);

		glClearColor(0.5, 0.5, 0.55, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int tex = 0; tex < textures->count; tex++) {
			glActiveTexture(GL_TEXTURE0 + tex);
			glBindTexture(GL_TEXTURE_2D, textures->texture[tex].memory);
		}

		glUseProgram(shaderProgram);

		glm_mat4_identity(transforms->model);
		glm_rotate(transforms->model, 2 * glfwGetTime(), (vec3){1.0, 1.0, 1.0});
		glm_mat4_identity(transforms->view);
		glm_translate(transforms->view, (vec3){0.0, 0.0, -2.0});
		glm_mat4_identity(transforms->projection);
		glm_perspective(glm_rad(90), (float)window->width/(float)window->height, 0.1, 100.0, transforms->projection);

		transforms->modelLoc = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(transforms->modelLoc, 1, GL_FALSE, (float*)transforms->model);
		transforms->viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(transforms->viewLoc, 1, GL_FALSE, (float*)transforms->view);
		transforms->projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(transforms->projectionLoc, 1, GL_FALSE, (float*)transforms->projection);

		glBindVertexArray(model->VAO);
		glDrawArrays(GL_TRIANGLES, 0, model->vCount);

		/*for (int obj = 0; obj < objects->count; obj++) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects->object[obj].EBO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}*/

		glfwSwapBuffers(window->frame);
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

	content = (char*)malloc(sizeof(char) * size);
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

	objects->object = (Object*)malloc(sizeof(Object) * objects->count);
	if (objects->object == NULL) return -1;
	for (int obj = 0; obj < objects->count; obj++) {
		InitializeObjectData(&objects->object[obj]);
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
		SetObjectData(&objects->object[obj]);
	}

	return 0;
}

char* ReadObjectData(Object *object, char *fileName) {
	char buffer[128];
	char typeBuf[4];
	unsigned int type;

	FILE *fPtr = fopen(fileName, "r");
	if (fPtr == NULL) return 0;

/*
o Plane
v -1.000000 -1.000000 -0.000000
v 1.000000 -1.000000 -0.000000
v -1.000000 1.000000 0.000000
v 1.000000 1.000000 0.000000
vn -0.0000 -0.0000 1.0000
vt 0.000000 0.000000
vt 1.000000 0.000000
vt 1.000000 1.000000
vt 0.000000 1.000000
s 0
f 1/1/1 2/2/1 4/3/1 3/4/1
*/

	/*while (fgets(buffer, 128, fPtr) != NULL) {
		for (int i = 0; buffer[i] != ' ' && i < 3; i++) {
			typeBuf[i] = buffer[i];
			typeBuf[i + 1] = '\0';
		}

		if (strcmp(typeBuf, "v")) {
			sscanf(buffer, "%s %f %f %f", NULL,
			&object->vArray[object->vCount][0], &object->vArray[object->vCount][1], &object->vArray[object->vCount][2]);
			object->vCount++;
		}
		else if (strcmp(typeBuf, "vn")) {
			object->vnCount++;
		}
		else if (strcmp(typeBuf, "vt")) {
			sscanf(buffer, "%s %f %f", NULL,
			&object->vArray[vtOffset * (object->vtCount + 1)], &object->vArray[1]);
			object->vtCount++;
		}
		else if (strcmp(typeBuf, "f")) {
			sscanf(buffer, "%s %d/%d/%d", NULL,
			&object->fArray[object->fCount], &object->fArray[object->fCount], &object->fArray[object->fCount]);
			object->fCount++;
		}
		else continue;
	}*/

	return 0;
}

unsigned int GetType(char buffer) {
	unsigned int type;

	switch (buffer) {
		case 'o':
			type = 1;
			break;
		case 'v':
			type = 2;
			break;
		case 'f':
			type = 5;
			break;
		default:
			type = 0;
			break;
	}
	return type;
}

void InitializeObjectData(Object *object) {
	object->vCount = 0;
	object->vtCount = 0;
	object->vnCount = 0;
	object->fCount = 0;
		
	object->vArray = NULL;
	object->fArray = NULL;

	object->VBO = 0;
	object->VAO = 0;
	object->EBO = 0;
}

void SetObjectData(Object *object) {
	glGenBuffers(1, &object->VBO);
	glGenVertexArrays(1, &object->VAO);
	glGenBuffers(1, &object->EBO);

	glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * object->vCount, object->vArray, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ivec3) * object->fCount, object->fArray, GL_STATIC_DRAW);

	glBindVertexArray(object->VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
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

	textures->texture = (Texture*)malloc(sizeof(Texture) * textures->count);
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
		printf("  ->Texture[%d]: %s\n", i, textures->texture[i].name);
		glGenTextures(1, &textures->texture[i].memory);
		glBindTexture(GL_TEXTURE_2D, textures->texture[i].memory);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		data = stbi_load(textures->texture[i].name, &textureWidth, &textureHeight, &colorChannels, 4);
		if (!data) return -1;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0,  GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}


	printf("\n->Linking texture data..\n");
	glUseProgram(shaderProgram);

	char textureName[64];
	char num[16];

	for (int tex = 0; tex < textures->count; tex++) {
		printf("  ->Texture[%d]\n", tex);
		strcpy(textureName, "texture");
		sprintf(num, "%d", tex); 
		strcat(textureName, num);
		printf("%s\n", textureName);
		glUniform1i(glGetUniformLocation(shaderProgram, textureName), tex);
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
			if (objects->object[obj].fArray != NULL) {
				free (objects->object[obj].fArray);
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
