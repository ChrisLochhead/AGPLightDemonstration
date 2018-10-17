// Std. Includes
#include <string>

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
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
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
Model *ourModel;// ("res/CubeObject/okcube.obj");
Model *wallModel;// ("res/CubeObject/okcube.obj");
Model *characterModel;// ("res/models/nanosuit.obj");

//projection matrix
glm::mat4 projection;

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
	 ourModel = new Model("res/CubeObject/okcube.obj");
	 wallModel = new Model("res/CubeObject/okcube.obj");
	 characterModel = new Model("res/models/nanosuit.obj");

	// Draw in wireframe
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
}
int main()
{
	initGlfw();
	initGlew();
	init();
    // Setup and compile our shaders
    /*Shader shader( "res/shaders/modelLoading.vs", "res/shaders/modelLoading.frag" );
    
    // Load models
	Model ourModel( "res/CubeObject/okcube.obj" );
	Model wallModel("res/CubeObject/okcube.obj");
	Model characterModel("res/models/nanosuit.obj");
    
    // Draw in wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 100.0f );
    */
    // Game loop
    while( !glfwWindowShouldClose( window ) )
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
		glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), 0);
		glUniform1i(glGetUniformLocation(shader->Program, "material.specular"), 1);

		GLint lightPosLoc = glGetUniformLocation(shader->Program, "light.position");
		GLint lightSpotdirLoc = glGetUniformLocation(shader->Program, "light.direction");
		GLint lightSpotCutOffLoc = glGetUniformLocation(shader->Program, "light.cutOff");
		GLint lightSpotOuterCutOffLoc = glGetUniformLocation(shader->Program, "light.outerCutOff");
		GLint viewPosLoc = glGetUniformLocation(shader->Program, "viewPos");
		glUniform3f(lightPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(lightSpotdirLoc, camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform1f(lightSpotCutOffLoc, glm::cos(glm::radians(12.5f)));
		glUniform1f(lightSpotOuterCutOffLoc, glm::cos(glm::radians(17.5f)));
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		// Set lights properties
		glUniform3f(glGetUniformLocation(shader->Program, "light.ambient"), 0.1f, 0.1f, 0.1f);
		// We set the diffuse intensity a bit higher; note that the right lighting conditions differ with each lighting method and environment.
		// Each environment and lighting type requires some tweaking of these variables to get the best out of your environment.
		glUniform3f(glGetUniformLocation(shader->Program, "light.diffuse"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(shader->Program, "light.specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shader->Program, "light.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shader->Program, "light.linear"), 0.09);
		glUniform1f(glGetUniformLocation(shader->Program, "light.quadratic"), 0.032);
		// Set material properties
		glUniform1f(glGetUniformLocation(shader->Program, "material.shininess"), 32.0f);


		glUniform3f(glGetUniformLocation(shader->Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(shader->Program, "dirLight.ambient"), 0.3f, 0.3f, 0.3f);
		glUniform3f(glGetUniformLocation(shader->Program, "dirLight.diffuse"), 0.4f, 0.4f, 0.4f);
		glUniform3f(glGetUniformLocation(shader->Program, "dirLight.specular"), 0.9f, 0.9f, 0.9f);

        glm::mat4 view = camera.GetViewMatrix( );
        glUniformMatrix4fv( glGetUniformLocation( shader->Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
        glUniformMatrix4fv( glGetUniformLocation( shader->Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
        
        // Draw the loaded model
        glm::mat4 model(1.0f);
        model = glm::translate( model, glm::vec3( 0.0f, 0.0f, 0.0f ) ); // Translate it down a bit so it's at the center of the scene
        model = glm::scale( model, glm::vec3( 15.0f, 0.2f, 15.0f ) );	// It's a bit too big for our scene, so scale it down
        glUniformMatrix4fv( glGetUniformLocation( shader->Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
        ourModel->Draw( *shader );

		 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 1.1f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		wallModel->Draw(*shader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 1.1f, -5.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		characterModel->Draw(*shader);

        
        // Swap the buffers
        glfwSwapBuffers( window );
    }
    
    glfwTerminate( );
    return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement( )
{
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



