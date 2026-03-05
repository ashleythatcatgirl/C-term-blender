#include <GLFW/glfw3.h>
#include "Include/cglm/cglm.h"

typedef struct Window {
	int width;
	int height;
	GLFWwindow *frame;
} Window;

typedef struct Input {
	unsigned int options;
	// wireframe
	// whatver
} Input;

typedef struct Model {
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	unsigned int vCount;

	float *verticies;
	unsigned int *indices;
} Model;

typedef struct Object {
	unsigned int vOffset;
	unsigned int vtOffset;

	unsigned int vCount;
	unsigned int vtCount;
	unsigned int vnCount;
	unsigned int fCount;

	float *vArray;
	unsigned int *fArray;

	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	char name[64];
} Object;

typedef struct Objects {
	unsigned int count;
	Object *object;
} Objects;

typedef struct Texture {
	unsigned int memory;
	char name[64];
} Texture;

typedef struct Textures {
	unsigned int count;
	Texture *texture;
} Textures;

typedef struct Transforms {
	mat4 model;
	mat4 view;
	mat4 projection;
	
	int modelLoc;
	int viewLoc;
	int projectionLoc;
} Transforms;

typedef struct Camera {
	vec3 position;
	vec3 target;
	vec3 front;
	vec3 up;
	vec3 right;

	vec3 direction;

	float pitch;
	float yaw;

	float moveSpeed;
	float turnSpeed;

	float zoom;

	bool sprint;
} Camera;

int ParseArgs(int argc, char **argv, struct Input *input);

char* GetShaderContent(const char* fileName);

int LoadObjects(Objects *objects);
void InitializeObjectData(Object *object);
void SetObjectData(Object *object);
int ParseOBJFile(Object *object);

int LoadTextures(Textures *textures, unsigned int shaderProgram);

int RenderLoop(Window *window, unsigned int shaderProgram, Input *input, Model *model, Textures *textures, Transforms *transforms, Camera *camera);
void processInput(GLFWwindow *window, Camera *camera, float deltaTime);


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void FreeMemory(Objects *objects, Textures *textures);
