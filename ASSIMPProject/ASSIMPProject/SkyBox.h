#ifndef SKYBOX
#define SKYBOX

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "Camera.h"
#include "Shader.h"

using namespace std;

class SkyBox
{
public:
	static SkyBox* Instance()
	{
		static SkyBox inst;
		return &inst;
	}

	void init(const char* folder);
	void draw();
	void update(glm::mat4 VP_matr);

private:
	SkyBox();
	~SkyBox();

	GLuint VAO_skybox;
	GLuint VBO_vertices_textures;

	GLuint cube_texture_id;
	GLuint createCubeTexture(const char* folder);
	//GLuint skybox_shaders;

	glm::mat4 VP_matrix; // WITHOUT model for skybox

	Shader *skyBoxShader;
};

#endif