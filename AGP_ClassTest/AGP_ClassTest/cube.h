#ifndef CUBE_H
#define CUBE_H

GLuint cubeVertCount = 8;
GLfloat cubeVerts[] = { -0.5, -0.5f, -0.5f,
						-0.5, 0.5f, -0.5f,
						0.5, 0.5f, -0.5f,
						0.5, -0.5f, -0.5f,
						-0.5, -0.5f, 0.5f,
						-0.5, 0.5f, 0.5f,
						0.5, 0.5f, 0.5f,
						0.5, -0.5f, 0.5f };
GLfloat cubeColours[] = { 0.0f, 0.0f, 0.0f,
						 0.0f, 1.0f, 0.0f,
						 1.0f, 1.0f, 0.0f,
						 1.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 1.0f,
						 0.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f,
						 1.0f, 0.0f, 1.0f };
GLfloat cubeTexCoords[] = { 0.0f, 0.0f,
							0.0f, 1.0f,
							1.0f, 1.0f,
							1.0f, 0.0f,
							1.0f, 1.0f, 							
							1.0f, 0.0f,
							0.0f, 0.0f, 
							0.0f, 1.0f  };

GLuint cubeIndexCount = 36;
GLuint cubeIndices[] = {0,1,2, 0,2,3, // back  
						1,0,5, 0,4,5, // left					
						6,3,2, 3,6,7, // right
						1,5,6, 1,6,2, // top
						0,3,4, 3,7,4, // bottom
						6,5,4, 7,6,4 }; // front


#endif