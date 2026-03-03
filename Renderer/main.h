
#include <GLFW/glfw3.h>

typedef struct Input {
	int options;
	// wireframe
	// whatver
} Input;

typedef struct Object {
	int vCount;
	float *vArray;
	float *iArray;
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;
	char *name;
} Object;

typedef struct Objects {
	int count;
	Object *object;
} Objects ;

typedef struct Texture {
	unsigned int memory;
	char name[64];
} Texture ;

typedef struct Textures {
	int count;
	Texture *texture;
} Textures;


int ParseArgs(int argc, char **argv, struct Input *input);

char* GetShaderContent(const char* fileName);
int LoadObjects(Objects *objects);
int ParseOBJFile(Object *object);
int LoadTextures(Textures *textures, unsigned int shaderProgram);

int RenderLoop(GLFWwindow *window, unsigned int shaderProgram, Input *input, Objects *objects, Textures *textures);
void processInput(GLFWwindow *window);


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void FreeMemory(Objects *objects, Textures *textures);
