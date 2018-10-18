// Std. Includes
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
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode );
void MouseCallback( GLFWwindow *window, double xPos, double yPos );
void DoMovement( );

// Camera
Camera camera( glm::vec3( 15.0f, 5.0f, 10.0f ) );
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//glfw init stuff
GLFWwindow *window;

// Setup and compile our shaders
Shader *shader;// ("res/shaders/modelLoading.vs", "res/shaders/modelLoading.frag");

// Load models
Model *floorModel;// ("res/CubeObject/okcube.obj");
Model *wallModel;// ("res/CubeObject/okcube.obj");
Model *characterModel;// ("res/models/nanosuit.obj");

//matrices
glm::mat4 projection;
glm::mat4 model(1.0f);

//stack
stack<glm::mat4> mStack;

//light stuff
struct lightStruct
{

	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	GLfloat ambient[3];
	GLfloat diffuse[3];
	GLfloat specular[3];

};

struct dirLightStruct
{
	GLfloat direction[3];
	GLfloat ambient[3];
	GLfloat diffuse[3];
	GLfloat specular[3];
};

dirLightStruct dirLight0
{
	{ -0.2f, -1.0f, -0.3f },
	{ 0.3f, 0.3f, 0.3f },
	{ 0.4f, 0.4f, 0.4f },
	{ 0.9f, 0.9f, 0.9f }
};
lightStruct light0 {

	glm::cos(glm::radians(12.5f)),
	glm::cos(glm::radians(17.5f)),

	1.0f, 0.09f, 0.032f,

	{0.1f, 0.1f, 0.1f},
	{0.8f, 0.8f, 0.8f},
	{1.0f, 1.0f, 1.0f}

};

// Positions of the point lights
glm::vec3 pointLightPositions[] = { 
	glm::vec3(0.0f,  1.0f,  0.0f),
	glm::vec3(29.0f, 1.0f, -17.0f),
	glm::vec3(0.0f,  1.0f, 12.0f),
	glm::vec3(0.0f, 1.0f, -17.0f)
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

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
	 shader = new Shader("res/shaders/modelLoading.vs", "res/shaders/modelLoading.frag");

	// Load models
	 floorModel = new Model("res/FloorObject/floor.obj");
	 wallModel = new Model("res/CubeObject/okcube.obj");
	 characterModel = new Model("res/models/nanosuit.obj");

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

void setLight( lightStruct light)
{
	GLint lightPosLoc = glGetUniformLocation(shader->Program, "light.position");
	GLint lightSpotdirLoc = glGetUniformLocation(shader->Program, "light.direction");
	GLint lightSpotCutOffLoc = glGetUniformLocation(shader->Program, "light.cutOff");
	GLint lightSpotOuterCutOffLoc = glGetUniformLocation(shader->Program, "light.outerCutOff");

	glUniform3f(lightPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);//light.position[0], light.position[1], light.position[2]);
	glUniform3f(lightSpotdirLoc, camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
	glUniform1f(lightSpotCutOffLoc, light.cutOff);
	glUniform1f(lightSpotOuterCutOffLoc, light.outerCutOff);
	//glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);


	glUniform3f(glGetUniformLocation(shader->Program, "light.ambient"), light.ambient[0], light.ambient[1], light.ambient[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "light.diffuse"), light.diffuse[0], light.diffuse[1], light.diffuse[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "light.specular"), light.specular[0], light.specular[1], light.specular[2]);
	glUniform1f(glGetUniformLocation(shader->Program, "light.constant"), light.constant);
	glUniform1f(glGetUniformLocation(shader->Program, "light.linear"), light.linear);
	glUniform1f(glGetUniformLocation(shader->Program, "light.quadratic"), light.quadratic);

}

void setDirLight(dirLightStruct dirLight)
{
	glUniform3f(glGetUniformLocation(shader->Program, "dirLight.direction"), dirLight.direction[0], dirLight.direction[1], dirLight.direction[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "dirLight.ambient"), dirLight.ambient[0], dirLight.ambient[1], dirLight.ambient[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "dirLight.diffuse"), dirLight.diffuse[0], dirLight.diffuse[1], dirLight.diffuse[2]);
	glUniform3f(glGetUniformLocation(shader->Program, "dirLight.specular"), dirLight.specular[0], dirLight.specular[1], dirLight.specular[2]);
}

void draw()
{
	// floor
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
		mStack.top() = glm::translate(mStack.top(), glm::vec3(30.0f, 1.0f, 11.25f + (-i *2.0f))); 
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
}
float angle = 0.0f;
void update()
{

	glm::vec3 centrePoint(15.0f, 1.0f, -2.0f);

	float radius1 = 3.5f;
	float radius2 = 7.0f;
	float radius3 = 10.5f;
	float radius4 = 14.0f;

	pointLightPositions[0].x = centrePoint.x + glm::cos(glm::radians(angle)) * radius1;
	pointLightPositions[0].z = centrePoint.z + glm::sin(glm::radians(angle)) * radius1;

	pointLightPositions[1].x = centrePoint.x + glm::cos(glm::radians(angle)) * radius2;
	pointLightPositions[1].z = centrePoint.z + glm::sin(glm::radians(angle)) * radius2;

	pointLightPositions[2].x = centrePoint.x + glm::cos(glm::radians(angle)) * radius3;
	pointLightPositions[2].z = centrePoint.z + glm::sin(glm::radians(angle)) * radius3;

	pointLightPositions[3].x = centrePoint.x + glm::cos(glm::radians(angle)) * radius4;
	pointLightPositions[3].z = centrePoint.z + glm::sin(glm::radians(angle)) * radius4;

	angle += 0.5;



}
int main()
{
	initGlfw();

	initGlew();

	init();

    // Game loop
    while(!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime( );
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check and call events
        glfwPollEvents( );
        DoMovement( );
        
        // Clear the colorbuffer
        glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		shader->Use( );
		
		setMaterial(0,1, 32.0f);

		setLight(light0);

		//set view position for shader
		GLint viewPosLoc = glGetUniformLocation(shader->Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		 setDirLight(dirLight0);

		 // Point light 1
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[0].ambient"), 0.09f, 0.05f, 0.09f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[0].constant"), 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[0].linear"), 0.09f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[0].quadratic"), 0.032f);

		 // Point light 2
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[1].ambient"), 0.09f, 0.09f, 0.05f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[1].diffuse"), 0.8f, 0.8f, 0.8f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[1].constant"), 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[1].linear"), 0.09f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[1].quadratic"), 0.032f);

		 // Point light 3
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[2].ambient"), 0.05f, 0.09f, 0.09f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[2].diffuse"), 0.8f, 0.8f, 0.8f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[2].constant"), 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[2].linear"), 0.09f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[2].quadratic"), 0.032f);

		 // Point light 4
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[3].ambient"), 0.05f, 0.05f, 0.09f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[3].diffuse"), 0.8f, 0.8f, 0.8f);
		 glUniform3f(glGetUniformLocation(shader->Program, "pointLights[3].specular"), 1.0f, 1.0f, 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[3].constant"), 1.0f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[3].linear"), 0.09f);
		 glUniform1f(glGetUniformLocation(shader->Program, "pointLights[3].quadratic"), 0.032f);



		setMatrices();

		draw();
		update();
		//cout << "x: "<<pointLightPositions[0].x <<" y: " << pointLightPositions[0].y << " z: "<<pointLightPositions[0].z << endl;

        // Swap the buffers
        glfwSwapBuffers( window );
    }
    
    glfwTerminate( );
    return 0;
}


void changeLightColour(lightStruct light, char c, float increment)
{

		if (c == 'r')
		{
			light0.ambient[0] += increment;
			light0.diffuse[0] += increment;
			light0.specular[0] += increment;
		}

		if (c == 'g')
		{
			light0.ambient[1] += increment;
			light0.diffuse[1] += increment;
			light0.specular[1] += increment;
		}

		if (c == 'b')
		{
			light0.ambient[2] += increment;
			light0.diffuse[2] += increment;
			light0.specular[2] += increment;
		}
}

//movement for camera and changes the colour of the torch light
void DoMovement( )
{
    //increase red on torch
	if (keys[GLFW_KEY_1]) if (light0.ambient[0] < 1.0f) changeLightColour(light0, 'r', 0.1);
	//decrease red on torch
	if (keys[GLFW_KEY_2]) if (light0.ambient[0] > 0.0f) changeLightColour(light0, 'r', -0.1);
	//increase green on torch
	if (keys[GLFW_KEY_3]) if (light0.ambient[1] < 1.0f)  changeLightColour(light0, 'g', 0.1);
	//decrease green on torch
	if (keys[GLFW_KEY_4]) if (light0.ambient[1] > 0.0f)  changeLightColour(light0, 'g', -0.1);
	//increase blue on torch
	if (keys[GLFW_KEY_5]) if (light0.ambient[2] < 1.0f)  changeLightColour(light0, 'b', 0.1);
	//decrease blue on torch
	if (keys[GLFW_KEY_6]) if (light0.ambient[2] > 0.0f)  changeLightColour(light0, 'b', -0.1);


	// Camera controls
    if ( keys[GLFW_KEY_W] || keys[GLFW_KEY_UP] )
    {
        camera.ProcessKeyboard( FORWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN] )
    {
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT] )
    {
        camera.ProcessKeyboard( LEFT, deltaTime );
    }
    
    if ( keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT] )
    {
        camera.ProcessKeyboard( RIGHT, deltaTime );
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode )
{
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if ( key >= 0 && key < 1024 )
    {
        if ( action == GLFW_PRESS )
        {
            keys[key] = true;
        }
        else if ( action == GLFW_RELEASE )
        {
            keys[key] = false;
        }
    }
}

void MouseCallback( GLFWwindow *window, double xPos, double yPos )
{
    if ( firstMouse )
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xPos;
    lastY = yPos;
    
    camera.ProcessMouseMovement( xOffset, yOffset );
}



