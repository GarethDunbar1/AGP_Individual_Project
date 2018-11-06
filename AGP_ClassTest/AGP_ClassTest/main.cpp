//Advanced Graphics Programming Class Test by B00324366
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include "md2model.h"

using namespace std;

// Globals
// Real programs don't use globals :-D

GLuint meshIndexCount = 0;
GLuint toonIndexCount = 0;
GLuint md2VertCount = 0;
GLuint meshObjects[3];


//shader programs
GLuint invertfbotexProgram;
GLuint phongShaderProgram;
GLuint toonShaderProgram;
GLuint gouraudShaderProgram;
GLuint textureProgram;
GLuint skyboxProgram;
GLuint enviromentMapProgram;
GLuint refractionProgram;
GLuint greyscalefbotexProgram;
 

GLfloat r = 0.0f;

glm::vec3 eye(-2.0f, 1.0f, 8.0f);
glm::vec3 at(0.0f, 1.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

stack<glm::mat4> mvStack;

// TEXTURE STUFF
GLuint textures[3];
GLuint skybox[5];
GLuint labels[5];

int controlShaders = 1;

rt3d::lightStruct light0 = {
	{ 0.4f, 0.4f, 0.4f, 1.0f }, // ambient
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // diffuse
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // specular
	{ -5.0f, 2.0f, 2.0f, 1.0f }  // position
};
glm::vec4 lightPos(-5.0f, 2.0f, 2.0f, 1.0f); //light position

rt3d::materialStruct material0 = {
	{ 0.2f, 0.4f, 0.2f, 1.0f }, // ambient
	{ 0.5f, 1.0f, 0.5f, 1.0f }, // diffuse
	{ 0.0f, 0.1f, 0.0f, 1.0f }, // specular
	2.0f  // shininess
};
rt3d::materialStruct material1 = {
	{ 0.4f, 0.4f, 1.0f, 1.0f }, // ambient
	{ 0.8f, 0.8f, 1.0f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	1.0f  // shininess
};

// light attenuation
float attConstant = 1.0f;
float attLinear = 0.0f;
float attQuadratic = 0.0f;


float theta = 0.0f;

// md2 stuff
md2model tmpModel;
int currentAnim = 0;

void DrawScene(glm::vec4 tmp, glm::mat4 projection);

GLuint initQuadVAO()
{
	// Configure VAO/VBO
	GLfloat vertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	// screen quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	return quadVAO;
}

// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

													   // Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

// A simple texture loading function
// lots of room for improvement - and better error checking!
GLuint loadBitmap(char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

							  // load file - using core SDL library
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	SDL_PixelFormat *format = tmpSurface->format;

	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}


// A simple cubemap loading function
// lots of room for improvement - and better error checking!
GLuint loadCubeMap(const char *fname[6], GLuint *texID)
{
	glGenTextures(1, texID); // generate texture ID
	GLenum sides[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y };
	SDL_Surface *tmpSurface;

	glBindTexture(GL_TEXTURE_CUBE_MAP, *texID); // bind texture and set parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLuint externalFormat;
	for (int i = 0;i<6;i++)
	{
		// load file - using core SDL library
		tmpSurface = SDL_LoadBMP(fname[i]);
		if (!tmpSurface)
		{
			std::cout << "Error loading bitmap" << std::endl;
			return *texID;
		}

		// skybox textures should not have alpha (assuming this is true!)
		SDL_PixelFormat *format = tmpSurface->format;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;

		glTexImage2D(sides[i], 0, GL_RGB, tmpSurface->w, tmpSurface->h, 0,
			externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
		// texture loaded, free the temporary buffer
		SDL_FreeSurface(tmpSurface);
	}
	return *texID;	// return value of texure ID, redundant really
}


void init(void) {
	// invert fbo shader
	invertfbotexProgram = rt3d::initShaders("invert-fbo.vert", "invert-fbo.frag");

	//greyscale fbo shader 
	greyscalefbotexProgram = rt3d::initShaders("greyscale-fbo.vert", "greyscale-fbo.frag");

	//phong shader
	phongShaderProgram = rt3d::initShaders("phong-tex.vert", "phong-tex.frag");
	rt3d::setLight(phongShaderProgram, light0);
	rt3d::setMaterial(phongShaderProgram, material0);
	// set light attenuation shader uniforms
	GLuint uniformIndex = glGetUniformLocation(phongShaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(phongShaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(phongShaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);

	//toon shader
	toonShaderProgram = rt3d::initShaders("toon.vert", "toon.frag");
	rt3d::setLight(toonShaderProgram, light0);
	rt3d::setMaterial(toonShaderProgram, material0);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);

	//gouraud shader
	gouraudShaderProgram = rt3d::initShaders("gouraud.vert", "simple.frag");
	rt3d::setLight(gouraudShaderProgram, light0);
	rt3d::setMaterial(gouraudShaderProgram, material0);
	// set light attenuation shader uniforms
	uniformIndex = glGetUniformLocation(gouraudShaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(gouraudShaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(gouraudShaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);

	//enviroment mapping (reflection)
	enviromentMapProgram = rt3d::initShaders("phong-enviromentMap.vert", "phong-enviromentMap.frag");
	rt3d::setLight(enviromentMapProgram, light0);
	rt3d::setMaterial(enviromentMapProgram, material0);
	// set light attenuation shader uniforms
	uniformIndex = glGetUniformLocation(enviromentMapProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(enviromentMapProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(enviromentMapProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);
	
	//refraction
	refractionProgram = rt3d::initShaders("phong-refraction.vert", "phong-refraction.frag");
	rt3d::setLight(refractionProgram, light0);
	rt3d::setMaterial(refractionProgram, material0);
	// set light attenuation shader uniforms
	uniformIndex = glGetUniformLocation(refractionProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(refractionProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(refractionProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);
	

	textureProgram = rt3d::initShaders("textured.vert", "textured.frag");
	skyboxProgram = rt3d::initShaders("cubeMap.vert", "cubeMap.frag");

	const char *cubeTexFiles[6] = {
		"Town-skybox/Town_bk.bmp", "Town-skybox/Town_ft.bmp", "Town-skybox/Town_rt.bmp", "Town-skybox/Town_lf.bmp", "Town-skybox/Town_up.bmp", "Town-skybox/Town_dn.bmp"
	};
	loadCubeMap(cubeTexFiles, &skybox[0]);

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("cube.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();
	textures[0] = loadBitmap("fabric.bmp");
	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());


	verts.clear(); norms.clear();tex_coords.clear();indices.clear();
	rt3d::loadObj("bunny-5000.obj", verts, norms, tex_coords, indices);
	toonIndexCount = indices.size();
	meshObjects[2] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), nullptr, toonIndexCount, indices.data());


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Binding tex handles to tex units to samplers under programmer control
	// set cubemap sampler to texture unit 1, arbitrarily
	uniformIndex = glGetUniformLocation(enviromentMapProgram, "cubeMap");
	glUniform1i(uniformIndex, 1);
	// set tex sampler to texture unit 0, arbitrarily
	uniformIndex = glGetUniformLocation(enviromentMapProgram, "texMap");
	glUniform1i(uniformIndex, 0);

}

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::sin(r), pos.y, pos.z - d*std::cos(r));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(r), pos.y, pos.z + d*std::sin(r));
}

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	//Camera Controls 
	if (keys[SDL_SCANCODE_W]) eye = moveForward(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_S]) eye = moveForward(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_A]) eye = moveRight(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_D]) eye = moveRight(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1;

	//Light Controls
	if (keys[SDL_SCANCODE_UP]) lightPos[2] -= 0.1; //move light forwards
	if (keys[SDL_SCANCODE_LEFT]) lightPos[0] -= 0.1; //move light left
	if (keys[SDL_SCANCODE_DOWN]) lightPos[2] += 0.1; //move light backwards
	if (keys[SDL_SCANCODE_RIGHT]) lightPos[0] += 0.1; //move light right
	if (keys[SDL_SCANCODE_LSHIFT]) lightPos[1] += 0.1; //move light up
	if (keys[SDL_SCANCODE_RSHIFT]) lightPos[1] -= 0.1; //move light down

	//Camera Rotation
	if (keys[SDL_SCANCODE_COMMA]) r -= 0.09f;
	if (keys[SDL_SCANCODE_PERIOD]) r += 0.09f;

	//Shader Controls
	if (keys[SDL_SCANCODE_1]) controlShaders = 1;
	if (keys[SDL_SCANCODE_2]) controlShaders = 2;
	if (keys[SDL_SCANCODE_3]) controlShaders = 3;
	if (keys[SDL_SCANCODE_4]) controlShaders = 4;
	if (keys[SDL_SCANCODE_5]) controlShaders = 5;
}

void drawToonBunny(glm::vec4 tmp, glm::mat4 projection) {
	glUseProgram(toonShaderProgram);
	rt3d::setLightPos(toonShaderProgram, glm::value_ptr(tmp));
	rt3d::setUniformMatrix4fv(toonShaderProgram, "projection", glm::value_ptr(projection));
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-4.0f, 0.1f, -2.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0, 20.0, 20.0));
	rt3d::setUniformMatrix4fv(toonShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(toonShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);
	mvStack.pop();
}

void drawPhongBunny(glm::vec4 tmp, glm::mat4 projection) {
	glUseProgram(phongShaderProgram);
	rt3d::setLightPos(phongShaderProgram, glm::value_ptr(tmp));
	rt3d::setUniformMatrix4fv(phongShaderProgram, "projection", glm::value_ptr(projection));
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-4.0f, 0.1f, -2.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0, 20.0, 20.0));
	rt3d::setUniformMatrix4fv(phongShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(phongShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);
	mvStack.pop();
}

void drawGouraudBunny(glm::vec4 tmp, glm::mat4 projection) {
	glUseProgram(gouraudShaderProgram);
	rt3d::setLightPos(gouraudShaderProgram, glm::value_ptr(tmp));
	rt3d::setUniformMatrix4fv(gouraudShaderProgram, "projection", glm::value_ptr(projection));
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-4.0f, 0.1f, -2.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0, 20.0, 20.0));
	rt3d::setUniformMatrix4fv(gouraudShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(gouraudShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);
	mvStack.pop();
}

void drawReflectedBunny(glm::vec4 tmp, glm::mat4 projection) {
	// Now bind textures to texture units
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glUseProgram(enviromentMapProgram);
	rt3d::setUniformMatrix4fv(enviromentMapProgram, "projection", glm::value_ptr(projection));
	rt3d::setLightPos(enviromentMapProgram, glm::value_ptr(tmp));
	//glBindTexture(GL_TEXTURE_2D, textures[2]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-4.0f, 0.1f, -2.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 20.0f, 20.0f));
	rt3d::setUniformMatrix4fv(enviromentMapProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(enviromentMapProgram, material1);

	glm::mat4 modelMatrix(1.0);
	mvStack.push(mvStack.top());
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.0f, 0.1f, -2.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	mvStack.top() = mvStack.top() * modelMatrix;
	rt3d::setUniformMatrix4fv(enviromentMapProgram, "modelMatrix", glm::value_ptr(modelMatrix));

	GLuint uniformIndex = glGetUniformLocation(enviromentMapProgram, "cameraPos");
	glUniform3fv(uniformIndex, 1, glm::value_ptr(eye));
	
	rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);

	mvStack.pop();
	mvStack.pop();
}

void drawRefractedBunny(glm::vec4 tmp, glm::mat4 projection) {
	// Now bind textures to texture units
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glUseProgram(refractionProgram);
	rt3d::setUniformMatrix4fv(refractionProgram, "projection", glm::value_ptr(projection));
	rt3d::setLightPos(refractionProgram, glm::value_ptr(tmp));
	//glBindTexture(GL_TEXTURE_2D, textures[2]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-4.0f, 0.1f, -2.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 20.0f, 20.0f));
	rt3d::setUniformMatrix4fv(refractionProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(refractionProgram, material1);

	glm::mat4 modelMatrix(1.0);
	mvStack.push(mvStack.top());
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.0f, 0.1f, -2.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	mvStack.top() = mvStack.top() * modelMatrix;
	rt3d::setUniformMatrix4fv(refractionProgram, "modelMatrix", glm::value_ptr(modelMatrix));

	GLuint uniformIndex = glGetUniformLocation(refractionProgram, "cameraPos");
	glUniform3fv(uniformIndex, 1, glm::value_ptr(eye));

	rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);

	mvStack.pop();
	mvStack.pop();
}

void draw(SDL_Window * window) {
	// ==========================================================================================

	// fbo
	unsigned int fbo;
	glGenFramebuffers(1, &fbo); //Creates the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);//binds the fbo as the active framebuffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) //checks the status of the framebuffer to see if we have completed the requirments.
		std::cout << "framebuffer not complete" << std::endl;

	// creates the texture for the framebuffer
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// attaches the texture to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	// first pass
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// ===================================================================================================

	// clear the screen
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection(1.0);
	projection = glm::perspective(float(1.04f), 800.0f / 600.0f, 1.0f, 150.0f);


	GLfloat scale(1.0f); // just to allow easy scaling of complete scene

	glm::mat4 modelview(1.0); // set base position for scene
	mvStack.push(modelview);

	at = moveForward(eye, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, at, up);

	// skybox as single cube using cube map
	glUseProgram(skyboxProgram);
	rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));

	glDepthMask(GL_FALSE); // make sure writing to update depth test is off
	glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
	mvStack.push(glm::mat4(mvRotOnlyMat3));

	glCullFace(GL_FRONT); // drawing inside of cube!
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox[0]);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.5f, 1.5f, 1.5f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();
	glCullFace(GL_BACK); // drawing inside of cube!

	// back to remainder of rendering
	glDepthMask(GL_TRUE); // make sure depth test is on

	glUseProgram(phongShaderProgram);
	rt3d::setUniformMatrix4fv(phongShaderProgram, "projection", glm::value_ptr(projection));

	glm::vec4 tmp = mvStack.top()*lightPos;
	light0.position[0] = tmp.x;
	light0.position[1] = tmp.y;
	light0.position[2] = tmp.z;
	rt3d::setLightPos(phongShaderProgram, glm::value_ptr(tmp));

	// draw a small cube block at lightPos
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.25f, 0.25f, 0.25f));
	rt3d::setUniformMatrix4fv(phongShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(phongShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// draw a cube for ground plane
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.1f, -10.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
	rt3d::setUniformMatrix4fv(phongShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(phongShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// ==========================================================================================
	
	// first pass draw bunny
	DrawScene(tmp, projection);
	
	// second pass
	GLuint quadVAO = initQuadVAO();

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(greyscalefbotexProgram);
	
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	GLuint glu_loc = glGetUniformLocation(greyscalefbotexProgram, "screenTexture");
	glUniform1f(glu_loc, 2);
	
	glBindVertexArray(quadVAO);

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glEnable(GL_DEPTH_TEST);
	// ==========================================================================================
	
	// remember to use at least one pop operation per push...
	mvStack.pop(); // initial matrix
	glDepthMask(GL_TRUE);


	SDL_GL_SwapWindow(window); // swap buffers
}

void DrawScene(glm::vec4 tmp, glm::mat4 projection)
{
	if (controlShaders == 1) drawToonBunny(tmp, projection);
	if (controlShaders == 2) drawPhongBunny(tmp, projection);
	if (controlShaders == 3) drawGouraudBunny(tmp, projection);
	if (controlShaders == 4) drawReflectedBunny(tmp, projection);
	if (controlShaders == 5) drawRefractedBunny(tmp, projection);
}

// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

								  // Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}