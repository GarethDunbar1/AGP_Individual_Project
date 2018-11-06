// Phong reflection model vertex shader - phong.vert
// use with simple.frag
// Using Phong reflection with Gourard shading provided by OpenGL's
// default smooth shading interpolation on fragments
#version 330

struct lightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct materialStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform mat4 modelview;
uniform mat4 projection;
uniform lightStruct light;
uniform materialStruct material;
uniform vec4 lightPosition;

in  vec3 in_Position;
//in  vec3 in_Colour; // colour not used with lighting
in  vec3 in_Normal;
out vec4 ex_Color;

// phong shader program
// multiply each vertex position by the MVP matrix
void main(void) {

	// vertex into eye coordinates
	vec4 vertexPosition = modelview * vec4(in_Position,1.0);
    gl_Position = projection * vertexPosition;

	// Find V - in eye coordinates, eye is at (0,0,0)
	vec3 V = normalize(-vertexPosition.xyz);

	// Vertex normal in eye coordinates
	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	vec3 N = normalize(normalmatrix*in_Normal);
	
	// Ambient intensity
	vec4 ambientI = light.ambient * material.ambient;

	// L - to light source from vertex
	vec3 L = normalize(lightPosition.xyz - vertexPosition.xyz);

	// Diffuse intensity
	vec4 diffuseI = light.diffuse * material.diffuse * max(dot(N,L),0);

	// Specular intensity
	// Calculate R - reflection of light
	vec3 R = normalize(-reflect(L,N));
	vec4 specularI = light.specular * material.specular;
	specularI = specularI * pow(max(dot(R,V),0), material.shininess);
	ex_Color = ambientI + diffuseI + specularI;
	//ex_Color =  specularI;

}