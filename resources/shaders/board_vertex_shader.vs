#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

out vec2 outTexCoords;
out vec3 outNormCoords;
out vec3 outFragCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    outTexCoords = aTex;
    outNormCoords = mat3(transpose(inverse(model))) * aNorm;
    outFragCoords = vec3(model * vec4(aPos, 1.0));
}