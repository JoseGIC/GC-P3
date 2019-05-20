#include "BOX.h"
#include "auxiliar.h"

#include <windows.h>

#include <gl/glew.h>
#include <gl/gl.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
static glm::mat4 proj = glm::mat4(1.0f);
static glm::mat4 view = glm::mat4(1.0f);
static glm::mat4 model1 = glm::mat4(1.0f);
static glm::mat4 model2 = glm::mat4(1.0f);

//Variables auxiliares
static float farPlane = 100.0f;
static float nearPlane = 0.1f;

static glm::vec3 mouseButtons = glm::vec3(false);
static glm::vec2 mousePosition = glm::vec2(0);
static glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 5.0f);
static glm::vec3 lightIntensity = glm::vec3(1.0f);

static bool paused = false;


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
//Por definir
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uLightPosition;
int uLightIntensity;
int uView;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

int uColorTex;
int uEmiTex;

//VAO
unsigned int vao;
//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int colorTexId;
unsigned int emiTexId;

int w = 500, h = 500;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar


//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);


//Funciones auxiliares
void orbitalCamera(int angleX, int angleY);
void firstPersonCamera(int angleX, int angleY);

void setViewMat(glm::mat4 viewMat);
void setProjMat(glm::mat4 projMat);

void setCameraPosition(glm::vec3 cameraPosition);

glm::mat4 getViewMat();
glm::mat4 getProjMat();
glm::vec3 getCameraPosition();
glm::vec3 getCameraRight();
glm::vec3 getCameraUp();
glm::vec3 getCameraBack();


//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	glutMainLoop();

	destroy();


	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Prácticas OGL");


	glutInitWindowSize(w, h);
	glutInitWindowPosition(0, 0);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;
	
	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	
	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 8.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraBack = glm::normalize(cameraPosition - cameraTarget);//coordenadas view
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); //coordenadas world
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraBack)); //coordenadas view
	glm::vec3 cameraUp = glm::cross(cameraBack, cameraRight); //coordenadas view
	glm::mat4 viewMat = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
	setViewMat(viewMat);

	
	float f = 1.0f / tan(3.141592f / 6.0f);

	glm::mat4 projMat = glm::mat4(1.0f);
	projMat[0].x = f;
	projMat[1].y = f;
	projMat[2].z = (farPlane + nearPlane) / (nearPlane - farPlane);
	projMat[2].w = -1.0f;
	projMat[3].z = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
	projMat[3].w = 0.0f;

	setViewMat(viewMat);
	setProjMat(projMat);
}

void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
}

void initShader(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);


	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");

	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uLightPosition = glGetUniformLocation(program, "lightPosition");
	uLightIntensity = glGetUniformLocation(program, "lightIntensity");
	uView = glGetUniformLocation(program, "view");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");


	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");


}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexPos, GL_STATIC_DRAW);


		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}
	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}
	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2, cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}


	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex, GL_STATIC_DRAW);

	model1 = glm::mat4(1.0f);
	model2 = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char *fileName, GLenum type)
{ 
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}



	return shader; 
}

unsigned int loadTex(const char *fileName)
{ 
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	glGenerateMipmap(GL_TEXTURE_2D);

	delete[] map;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);


	
	
	return texId; 
}

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w, h);
	glUseProgram(program);

	if (uView != -1) 
		glUniformMatrix4fv(uView, 1, GL_FALSE, &(view[0][0]));
	if (uLightIntensity != -1) 
		glUniform3fv(uLightIntensity, 1, &(lightIntensity.x));
	if (uLightPosition != -1) 
		glUniform3fv(uLightPosition, 1, &(lightPosition.x));

	if (uColorTex != -1)
	{
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)
	{
		glUniform1i(uEmiTex, 1);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, emiTexId);



	// Cubo 1
	glm::mat4 modelView = view * model1;
	glm::mat4 modelViewProj = proj * modelView;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, 0);


	// Cubo 2
	modelView = view * model2;
	modelViewProj = proj * modelView;
	normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, 0);


	
	glutSwapBuffers();
}




//=======FUNCIONES AUXILIARES=======//

void setViewMat(glm::mat4 viewMat)
{
	view = viewMat;
}

void setProjMat(glm::mat4 projMat)
{
	proj = projMat;
}

glm::mat4 getViewMat()
{
	return view;
}

glm::mat4 getProjMat()
{
	return proj;
}

void setCameraPosition(glm::vec3 cameraPosition)
{
	view[3].x = cameraPosition.x;
	view[3].y = cameraPosition.y;
	view[3].z = cameraPosition.z;
}

glm::vec3 getCameraPosition()
{
	return glm::vec3(view[3].x, view[3].y, view[3].z);
}

glm::vec3 getCameraRight()
{
	return glm::vec3(view[0].x, view[1].x, view[2].x);
}

glm::vec3 getCameraUp()
{
	return glm::vec3(view[0].y, view[1].y, view[2].y);
}

glm::vec3 getCameraBack()
{
	return glm::vec3(view[0].z, view[1].z, view[2].z);
}

//==================================//





void resizeFunc(int width, int height)
{	
	w = width;
	h = height;

	float temp = tan((3.141592f * 30.0f) / 180.0f);
	float aspectRat = (float)width / (float)height;

	glm::mat4 projMat = glm::mat4(0.0f);
	projMat[0].x = 1 / (aspectRat * temp);
	projMat[1].y = 1 / temp;
	projMat[2].z = -(farPlane + nearPlane) / (farPlane - nearPlane);
	projMat[2].w = -1.0f;
	projMat[3].z = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);
	projMat[3].w = 0.0f;

	setProjMat(projMat);
	glutPostRedisplay();
}


void idleFunc()
{
	static float angle1 = 0.0f;
	static float angle2 = 0.0f;
	static float angle3 = 0.0f;

	if (!paused)
	{
		//Cubo 1
		glm::mat4 modelMat1(1.0f);
		angle1 = (angle1 > 3.141592f * 2.0f) ? 0.0f : angle1 + 0.01f;
		model1 = modelMat1;modelMat1 = glm::rotate(modelMat1, angle1, glm::vec3(1.0f, 1.0f, 0.0f));
		model1 = modelMat1;

		//Cubo 2
		glm::mat4 modelMat2(1.0);
		angle2 = (angle2 < 2.0f * 3.141592f) ? angle2 + 0.01f : 0.0f;
		angle3 = (angle3 < 2.0f * 3.141592f) ? angle3 + 0.03f : 0.0f;

		modelMat2 = glm::rotate(modelMat2, angle2, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMat2 = glm::translate(modelMat2, glm::vec3(4.0f, 0.0f, 0.0f));
		modelMat2 = glm::rotate(modelMat2, angle3, glm::vec3(0.0f, 1.0f, 0.0f));
		model2 = modelMat2;
	}

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	float amount = 0.1f;

	glm::vec3 translationVec = glm::vec3(0.0f);

	switch (key)
	{
	case 'p':
		//Pausa movimiento de los cubos
		paused = !paused;
		break;


	case 'w':
		//Camara hacia delante
		translationVec = getCameraBack();
		break;
	case 's':
		//Camara hacia detras
		translationVec = -getCameraBack();
		break;
	case 'a':
		//Camara hacia izquierda
		translationVec = getCameraRight();
		break;
	case 'd':
		//Camara hacia derecha
		translationVec = -getCameraRight();
		break;


	case 'i':
		//Luz hacia delante
		lightPosition += glm::vec3(0.0f, 0.0f, -1.0f);
		break;
	case 'k':
		//Luz hacia atras
		lightPosition += glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	case 'l':
		//Luz hacia derecha
		lightPosition += glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'j':
		//Luz hacia izquierda
		lightPosition += glm::vec3(-1.0f, 0.0f, 0.0f);
		break;
	case 'm':
		//Luz hacia arriba
		lightPosition += glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case 'n':
		//Luz hacia abajo
		lightPosition += glm::vec3(0.0f, -1.0f, 0.0f);
		break;
	

	case '+':
		//Luz mas intensa
		if (lightIntensity.x < 1.0f)
			lightIntensity += glm::vec3(0.1f, 0.1f, 0.1f);
		break;
	case '-':
		//Luz menos intensa
		if (lightIntensity.x > 0.0f)
			lightIntensity -= glm::vec3(0.1f, 0.1f, 0.1f);
		break;
	}

	std::cout << "Se ha pulsado la tecla " << key << std::endl << std::endl;
	glm::mat4 view = glm::translate(getViewMat(), translationVec * amount);
	setViewMat(view);
	glutPostRedisplay();
}

void mouseFunc(int button, int state, int x, int y)
{
	if (state == 0)
	{
		std::cout << "Se ha pulsado el botón ";
	}
	else
	{
		mouseButtons = glm::vec3(false, false, false);
		std::cout << "Se ha soltado el botón ";
	}

	if (button == 0)
	{
		mouseButtons[0] = true;
		mousePosition.x = x;
		mousePosition.y = y;
		std::cout << "de la izquierda del ratón " << std::endl;
	}

	if (button == 1)
	{
		mouseButtons[1] = true;
		mousePosition.x = x;
		mousePosition.y = y;
		std::cout << "central del ratón " << std::endl;
	}

	if (button == 2)
	{
		mouseButtons[2] = true;
		mousePosition.x = x;
		mousePosition.y = y;
		std::cout << "de la derecha del ratón " << std::endl;
	}

	std::cout << "en la posición " << x << " " << y << std::endl << std::endl;
}

void mouseMotionFunc(int x, int y)
{
	if (mouseButtons == glm::vec3(false, false, false))
		return;

	std::cout << "Se mueve el raton en la posición " << x << " " << y << std::endl << std::endl;

	int dx = x - mousePosition.x;
	int dy = y - mousePosition.y;

	if (mouseButtons[0] == true)
	{
		orbitalCamera(dx, dy);
	}
	else if (mouseButtons[2] == true)
	{
		firstPersonCamera(dx, dy);
	}

	mousePosition.x = x;
	mousePosition.y = y;
}

void orbitalCamera(int dx, int dy)
{
	std::cout << "Orbital camera" << std::endl;

	float rotation = 0.4f;
	float angleX = glm::radians(dx * rotation);
	float angleY = glm::radians(dy * rotation);

	glm::mat4 view = glm::rotate(getViewMat(), angleX, getCameraUp());
	view = glm::rotate(view, angleY, getCameraRight());

	setViewMat(view);
}

void firstPersonCamera(int dx, int dy)
{
	std::cout << "First person shooter camera" << std::endl;

	float rotation = 0.2f;
	float angleX = glm::radians(dx * rotation);
	float angleY = glm::radians(dy * rotation);

	glm::mat4 matPitch = glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 matYaw = glm::rotate(glm::mat4(1.0f), angleX, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 rotationMat = matPitch * matYaw;
	glm::mat4 view = rotationMat * getViewMat();

	setViewMat(view);
}