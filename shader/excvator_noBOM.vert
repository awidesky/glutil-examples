#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal; //TODO : swap with vertexUV, see if snapshot show what's messed up.
layout(location = 2) in vec2 vertexUV;

// Values that stay constant for the whole mesh.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//texture UV coordinate
out vec2 UV;
out vec3 FragPos;
out vec3 Normal;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  projection * view * model * vec4(vertexPosition_modelspace,1);
 
    UV = vertexUV;

    // World space position
    FragPos = vec3(model * vec4(vertexPosition_modelspace, 1.0));

    // Normal transform (non-uniform scale ??)
    Normal = mat3(transpose(inverse(model))) * vertexNormal;
}
