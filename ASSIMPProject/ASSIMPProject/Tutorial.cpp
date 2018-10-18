/*

uncomment for the tutorial starter code

*/



/*// Std. Includes
#include <string>
#include <stack>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "SkyBox.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2\include\SOIL2\SOIL2.h"

// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();

// Camera
Camera camera(glm::vec3(15.0f, 5.0f, 10.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
float angle = 0.0f;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//glfw init stuff
GLFWwindow *window;

// Setup and compile our shaders
Shader *shader;

// Load models
Model *floorModel;
Model *wallModel;
Model *characterModel;
Model *grassModel;

//matrices
glm::mat4 projection;
glm::mat4 model(1.0f);

//stack
stack<glm::mat4> mStack;

//light stuff
struct lightStruct
{

	float constant;
	float linear;
	float quadratic;

	GLfloat ambient[3];
	GLfloat diffuse[3];
	GLfloat specular[3];

};


lightStruct light0{


1.0f, 0.09f, 0.032f,

{ 0.1f, 0.1f, 0.1f },
{ 0.8f, 0.8f, 0.8f },
{ 1.0f, 1.0f, 1.0f }

};



int initGlfw()
{
	// Init GLFW
	glfwInit();

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WIDTH, HEIGHT, "Lighting Demonstration", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options - uncomment the following to disable the cursor
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
int initGlew() {
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);
}
void init() {

	// Setup and compile our shaders
	shader = new Shader("multiLightTutorial.vert", "multiLightTutorial.frag");

	// Load models
	grassModel = new Model("res/grassObject/grass.obj");
	floorModel = new Model("res/FloorObject/floor.obj");
	wallModel = new Model("res/CubeObject/okcube.obj");
	characterModel = new Model("res/models/nanosuit.obj");

	SkyBox::Instance()->init("images/skybox_violentday");

	// Draw in wireframe
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
}

void setMaterial(int diffuse, int specular, float shininess)
{
	glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), diffuse);
	glUniform1i(glGetUniformLocation(shader->Program, "material.specular"), specular);
	// Set material properties
	glUniform1f(glGetUniformLocation(shader->Program, "material.shininess"), shininess);
}

void setMatrices()
{
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

void setLight(lightStruct light)
{
	GLint lightPosLoc = glGetUniformLocation(shader->Program, "light.position");
	GLint lightSpotdirLoc = glGetUniformLocation(shader->Program, "light.direction");


	glUniform3f(lightPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);//light.position[0], light.position[1], light.position[2]);
	glUniform3f(lightSpotdirLoc, camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);

	glUniform3f(glGetUniformLocation(shader->Program, "light.ambient"), light.ambient[0], light.ambient[1], light.ambient[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "light.diffuse"), light.diffuse[0], light.diffuse[1], light.diffuse[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "light.specular"), light.specular[0], light.specular[1], light.specular[2]);
	glUniform1f(glGetUniformLocation(shader->Program, "light.constant"), light.constant);
	glUniform1f(glGetUniformLocation(shader->Program, "light.linear"), light.linear);
	glUniform1f(glGetUniformLocation(shader->Program, "light.quadratic"), light.quadratic);

}




void draw()
{

	// dirt floor
	mStack.push(model);
	mStack.top() = glm::translate(mStack.top(), glm::vec3(20.0f, 0.15f, -10.0f));
	mStack.top() = glm::rotate(mStack.top(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(mStack.top()));
	floorModel->Draw(*shader);
	mStack.pop();

	//left walls
	for (int i = 0; i < 15; i++) {
		mStack.push(model); //model = glm::mat4(1.0f);
		mStack.top() = glm::translate(mStack.top(), glm::vec3(0.0f, 1.0f, 11.25f + (-i * 2.0f)));
		mStack.top() = glm::rotate(mStack.top(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		mStack.top() = glm::scale(mStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(mStack.top()));
		wallModel->Draw(*shader);
		mStack.pop();
	}

	//back walls
	for (int i = 0; i < 15; i++) {
		mStack.push(model);
		mStack.top() = glm::translate(mStack.top(), glm::vec3(0.0f + (i * 2.0f), 1.0f, 11.25f));
		mStack.top() = glm::rotate(mStack.top(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		mStack.top() = glm::scale(mStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(mStack.top()));
		wallModel->Draw(*shader);
		mStack.pop();
	}

	//front walls
	for (int i = 0; i < 15; i++) {
		mStack.push(model);
		mStack.top() = glm::translate(mStack.top(), glm::vec3(0.0f + (i * 2.0f), 1.0f, -18.75f));
		mStack.top() = glm::rotate(mStack.top(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		mStack.top() = glm::scale(mStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(mStack.top()));
		wallModel->Draw(*shader);
		mStack.pop();
	}

	//right walls
	for (int i = 0; i < 15; i++) {
		mStack.push(model);
		mStack.top() = glm::translate(mStack.top(), glm::vec3(30.0f, 1.0f, 11.25f + (-i * 2.0f)));
		mStack.top() = glm::rotate(mStack.top(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		mStack.top() = glm::scale(mStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(mStack.top()));
		wallModel->Draw(*shader);
		mStack.pop();
	}

	//characters
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 3; j++) {
			mStack.push(model);
			mStack.top() = glm::translate(mStack.top(), glm::vec3(5.0f + (i *5.0f), 0.2f, 2.5f - (j * 5.0f)));
			mStack.top() = glm::scale(mStack.top(), glm::vec3(0.2f, 0.2f, 0.2f));
			glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(mStack.top()));
			characterModel->Draw(*shader);
			mStack.pop();
		}
	}

	SkyBox::Instance()->draw();
}

void update()
{
	// delete translation from view matrix
	SkyBox::Instance()->update(projection * glm::mat4(glm::mat3(camera.GetViewMatrix())));

}


int main()
{
	initGlfw();

	initGlew();

	init();

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		DoMovement();

		// Clear the colorbuffer
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader->Use();

		setMaterial(0, 1, 32.0f);

		setLight(light0);

		//set view position for shader
		GLint viewPosLoc = glGetUniformLocation(shader->Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		setMatrices();

		draw();
		update();

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


//movement for camera and changes the colour of the torch light
void DoMovement()
{
	


	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}


*/