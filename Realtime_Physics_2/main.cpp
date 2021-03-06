
#include <time.h>
#include <common/Shader.h>
#include <common/Utilities.h>
#include <common/SingleMeshLoader.h>
#include <common/EulerCamera.h>
#include <common/Defines.h>
#include <GLM.h>
#include "glm/ext.hpp"
#include <common/text.h>
#include <common/Particle.h>
#include <common/RigidBody.h>

using namespace std;

const float width = 1200, height = 950;

SingleMesh torch_object, wall_object, skyBox, cube_object, sphere_object;

//Cube Vertices for the Skybox
GLfloat vertices[] = {
	-10.0f, -10.0f, -10.0f,  0.0f,  0.0f, -1.0f,
	10.0f, -10.0f, -10.0f,  0.0f,  0.0f, -1.0f,
	10.0f,  10.0f, -10.0f,  0.0f,  0.0f, -1.0f,
	10.0f,  10.0f, -10.0f,  0.0f,  0.0f, -1.0f,
	-10.0f,  10.0f, -10.0f,  0.0f,  0.0f, -1.0f,
	-10.0f, -10.0f, -10.0f,  0.0f,  0.0f, -1.0f,

	-10.0f, -10.0f,  10.0f,  0.0f,  0.0f,  1.0f,
	10.0f, -10.0f,  10.0f,  0.0f,  0.0f,  1.0f,
	10.0f,  10.0f,  10.0f,  0.0f,  0.0f,  1.0f,
	10.0f,  10.0f,  10.0f,  0.0f,  0.0f,  1.0f,
	-10.0f,  10.0f,  10.0f,  0.0f,  0.0f,  1.0f,
	-10.0f, -10.0f,  10.0f,  0.0f,  0.0f,  1.0f,

	-10.0f,  10.0f,  10.0f, -1.0f,  0.0f,  0.0f,
	-10.0f,  10.0f, -10.0f, -1.0f,  0.0f,  0.0f,
	-10.0f, -10.0f, -10.0f, -1.0f,  0.0f,  0.0f,
	-10.0f, -10.0f, -10.0f, -1.0f,  0.0f,  0.0f,
	-10.0f, -10.0f,  10.0f, -1.0f,  0.0f,  0.0f,
	-10.0f,  10.0f,  10.0f, -1.0f,  0.0f,  0.0f,

	10.0f,  10.0f,  10.0f,  1.0f,  0.0f,  0.0f,
	10.0f,  10.0f, -10.0f,  1.0f,  0.0f,  0.0f,
	10.0f, -10.0f, -10.0f,  1.0f,  0.0f,  0.0f,
	10.0f, -10.0f, -10.0f,  1.0f,  0.0f,  0.0f,
	10.0f, -10.0f,  10.0f,  1.0f,  0.0f,  0.0f,
	10.0f,  10.0f,  10.0f,  1.0f,  0.0f,  0.0f,

	-10.0f, -10.0f, -10.0f,  0.0f, -1.0f,  0.0f,
	10.0f, -10.0f, -10.0f,  0.0f, -1.0f,  0.0f,
	10.0f, -10.0f,  10.0f,  0.0f, -1.0f,  0.0f,
	10.0f, -10.0f,  10.0f,  0.0f, -1.0f,  0.0f,
	-10.0f, -10.0f,  10.0f,  0.0f, -1.0f,  0.0f,
	-10.0f, -10.0f, -10.0f,  0.0f, -1.0f,  0.0f,

	-10.0f,  10.0f, -10.0f,  0.0f,  1.0f,  0.0f,
	10.0f,  10.0f, -10.0f,  0.0f,  1.0f,  0.0f,
	10.0f,  10.0f,  10.0f,  0.0f,  1.0f,  0.0f,
	10.0f,  10.0f,  10.0f,  0.0f,  1.0f,  0.0f,
	-10.0f,  10.0f,  10.0f,  0.0f,  1.0f,  0.0f,
	-10.0f,  10.0f, -10.0f,  0.0f,  1.0f,  0.0f
}; 

/*----------------------------------------------------------------------------
								SHADER VARIABLES
----------------------------------------------------------------------------*/

GLuint noTextureShaderID, textureShaderID, cubeMapShaderID;
GLuint normalisedShaderID;
/*----------------------------------------------------------------------------
							CAMERA VARIABLES
----------------------------------------------------------------------------*/

glm::vec3 startingPos = { 0.0f, 0.0f, -20.0f };
GLfloat pitCam = 0, yawCam = 0, rolCam = 0, frontCam = 0, sideCam = 0, speed = 1;
float rotateY = 50.0f, rotateLight = 0.0f;

EulerCamera cam(startingPos, width, height);
RigidBody body;
/*----------------------------------------------------------------------------
							OTHER VARIABLES
----------------------------------------------------------------------------*/

const char* atlas_image = "../freemono.png";
const char* atlas_meta = "../freemono.meta";

float fontSize = 25.0f;
int textID = -1;
bool torque = false;
bool force = false;
bool gravity = false;
bool force1 = false, force2 = false;

/*----------------------------------------------------------------------------
								FUNCTIONS
----------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/

void init()
{
	if (!init_text_rendering(atlas_image, atlas_meta, width, height)) 
	{
		fprintf(stderr, "ERROR init text rendering\n");
		exit(1);
	}
	cam.setSensitivity(2.0f);
	//*************************//
	//*****Compile Shaders*****//
	//*************************//
	Shader* shaderFactory = Shader::getInstance();
	noTextureShaderID = shaderFactory->CompileShader(NOTEXTURE_VERT, NOTEXTURE_FRAG);
	cubeMapShaderID = shaderFactory->CompileShader(SKY_VERT, SKY_FRAG);
	textureShaderID = shaderFactory->CompileShader(TEXTURE_VERT, TEXTURE_FRAG);
	normalisedShaderID = shaderFactory->CompileShader(NORMAL_VERT, NORMAL_FRAG);
	Shader::resetInstance();
	//*********************//
	//*****Init Objects*****//
	//*********************//
	skyBox.initCubeMap(vertices, 36, "desert");
	torch_object.init(TORCH_MESH, NULL, NULL);
	torch_object.mode = GL_QUADS;
	wall_object.init(WALL_MESH, BRICK_TEXTURE, BRICK_NORMAL);
	cube_object.init(CUBE_MESH, BEAR_TEXTURE, NULL);
	cube_object.mode = GL_QUADS;
	sphere_object.init(SPHERE_MESH, BEAR_TEXTURE, NULL);
	body = RigidBody(glm::vec3(0.0, 0.0, 0.0), 5, glm::vec3(1.0, 1.0, 1.0), cube_object);
}

void display() 
{
	glEnable(GL_DEPTH_TEST);								// enable depth-testing
	glDepthFunc(GL_LESS);									// depth-testing interprets a smaller value as "closer"
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear the color and buffer bits to make a clean slate
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);					//Create a background

	LightStruct lightStruct;
	glm::mat4 model;
	// light properties
	lightStruct.Ls = glm::vec3(1.0f, 1.0f, 1.0f);	//Specular Reflected Light
	lightStruct.Ld = glm::vec3(0.99f, 0.99f, 0.99f);	//Diffuse Surface Reflectance
	lightStruct.La = glm::vec3(1.0f, 1.0f, 1.0f);	//Ambient Reflected Light
	lightStruct.lightLocation = glm::vec3(5 * sin(rotateLight), 10, -5.0f*cos(rotateLight));//light source location
	lightStruct.coneDirection = lightStruct.lightLocation + glm::vec3(0.0f, -1.0f, 0.0f);
	// object colour
	lightStruct.Ks = glm::vec3(0.1f, 0.1f, 0.1f); // specular reflectance
	lightStruct.Kd = BROWN;
	lightStruct.Ka = glm::vec3(0.05f, 0.05f, 0.05f); // ambient reflectance
	lightStruct.specular_exponent = 0.5f; //specular exponent - size of the specular elements

	model = glm::mat4();
	drawCubeMap(cubeMapShaderID, cam, skyBox, lightStruct);

	model = glm::translate(glm::mat4(), lightStruct.lightLocation);
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
	lightStruct.Kd = WHITE;
	drawObject(noTextureShaderID, cam, skyBox, model, false, lightStruct);

	model = glm::translate(glm::mat4(), glm::vec3(0.0f, -6.3f, 0.0f));
	lightStruct.Kd = BROWN;
	drawObject(noTextureShaderID, cam, torch_object, model, false, lightStruct);

	lightStruct.Kd = GREY;
	model = glm::translate(glm::mat4(), glm::vec3(-7.5, -6.5f, -7.5));
	model = glm::rotate(model, 90.0f, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 root = model;
	lightStruct.specular_exponent = 50.0f;
	lightStruct.Ks = glm::vec3(1.0, 1.0, 1.0);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			model = glm::translate(glm::mat4(), glm::vec3(16.0f*i, 0.0, 0.0));
			model = glm::translate(model, glm::vec3(0.0, 0.0, 32.0f*j));
			model = model * root;
			drawObject(normalisedShaderID, cam, wall_object, model, false, lightStruct);
		}
	}

	lightStruct.Kd = BLUE;
	lightStruct.specular_exponent = 0.5f;
	drawObject(noTextureShaderID, cam, body.mesh, glm::mat4(), false, lightStruct);
	draw_texts();
	glutSwapBuffers();
}

void updateScene() {
	static clock_t lastFrame = clock();
	clock_t currFrame = clock();
	float delta = (currFrame - lastFrame) / (float)CLOCKS_PER_SEC;
	if (delta >= 0.03f) 
	{
		lastFrame = currFrame;
		glutPostRedisplay();
		rotateLight = rotateLight + 0.01f;
		if (rotateLight >= 360.0f)
			rotateLight = 0.0f;

		body.force = glm::vec3(0.0, 0.0, 0.0);
		body.torque = glm::vec3(0.0, 0.0, 0.0);
		body.addForce(glm::vec3(50.0f, 0.0f, 0.0f) * (force1 * 1.0f), glm::vec3(0.5, 0.0, 0.5));
		body.addForce(glm::vec3(2.0f, 50.0f, 0.0f) * (force2 * 1.0f), glm::vec3(0.0, 1.0, 0.0));
		if (gravity)
			body.force += glm::vec3(0.0f, -50.0f, 0.0f);
		if (torque)
			body.torque += glm::vec3(50.0, 0.0, 0.0);
		if (force)
			body.force += glm::vec3(0.0, 50.0, 0.0);
		cam.movForward(frontCam*speed);
		cam.movRight(sideCam*speed);
		cam.changeFront(pitCam, yawCam, rolCam);

		body.resolveForce(delta);

		string values = body.updateString();

		update_text(textID, values.c_str());
	}
	
}

#pragma region INPUT FUNCTIONS

void keypress(unsigned char key, int x, int y) 
{
	if (key == (char)27)	//Pressing Escape Ends the game
	{
		exit(0);
	}
	else if (key == 'w' || key == 'W')
		frontCam = 1;
	else if ((key == 's') || (key == 'S'))
		frontCam = -1;
	if ((key == 'a') || (key == 'A'))
		sideCam = -1;
	else if ((key == 'd') || (key == 'D'))
		sideCam = 1;
	if ((key == 't') || (key == 'T'))
		torque = true;
	if ((key == 'f') || (key == 'F'))
		force = true;
//	if ((key == 'g') || key == 'G')
//		gravity = true;
	if (key == '1')
		force1 = true;
	if (key == '2')
		force2 = true;
}

void keypressUp(unsigned char key, int x, int y)
{
	if ((key == 'w') || (key == 'W'))
		frontCam = 0;
	else if ((key == 's') || (key == 'S'))
		frontCam = 0;
	if ((key == 'a') || (key == 'A'))
		sideCam = 0;
	else if ((key == 'd') || (key == 'D'))
		sideCam = 0;
	if ((key == 't') || (key == 'T'))
		torque = false;
	if ((key == 'f') || (key == 'F'))
		force = false;
	if (key == '1')
		force1 = false;
	if (key == '2')
		force2 = false;
	if (key == ' ')
	{
		body.reset(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0));
	}
	else if (key == '\\')
	{
		body.reset(glm::vec3(0.0, 500.0, 0.0), glm::vec3(500.0, 500.0, 500.0));
	}
	if ((key == 'g') || key == 'G')
	{
		gravity = !gravity;
	}
}

void specialKeypress(int key, int x, int y) 
{
	switch (key)
	{
	case (GLUT_KEY_SHIFT_L):
	case (GLUT_KEY_SHIFT_R):
		speed = 4;
		break;
	case (GLUT_KEY_LEFT):
		yawCam = -1;
		break;
	case (GLUT_KEY_RIGHT):
		yawCam = 1;
		break;
	case (GLUT_KEY_UP):
		pitCam = 1;
		break;
	case (GLUT_KEY_DOWN):
		pitCam = -1;
		break;
	}
}

void specialKeypressUp(int key, int x, int y) 
{
	switch (key)
	{
	case (GLUT_KEY_SHIFT_L):
	case (GLUT_KEY_SHIFT_R):
		speed = 1;
		break;
	case (GLUT_KEY_LEFT):
		yawCam = 0;
		break;
	case (GLUT_KEY_RIGHT):
		yawCam = 0;
		break;
	case (GLUT_KEY_UP):
		pitCam = 0;
		break;
	case (GLUT_KEY_DOWN):
		pitCam = 0;
		break;
	}
}

void (mouse)(int x, int y)
{
	
}

#pragma endregion INPUT FUNCTIONS

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("GameApp");
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);


	// Tell glut where the display function is
	glutWarpPointer(width / 2, height / 2);
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);

	// Input Function Initialisers //
	glutKeyboardFunc(keypress);
	glutPassiveMotionFunc(mouse);
	glutSpecialFunc(specialKeypress);
	glutSpecialUpFunc(specialKeypressUp);
	glutKeyboardUpFunc(keypressUp);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	init();
	textID = add_text("",
		-0.95, 0.9, fontSize, 1.0f, 1.0f, 1.0f, 1.0f);
	glutMainLoop();
	return 0;
}