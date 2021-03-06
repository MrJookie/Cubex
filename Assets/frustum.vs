#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 inColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform mat4 mvp;

void main() {
    gl_PointSize = 3.0;
    
    gl_Position = projection * view * model * vec4( position, 1.0 );
    //gl_Position = projection * view * vec4( position, 1.0 );
    
    inColor = color;
}
