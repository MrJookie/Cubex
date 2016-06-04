#version 330

in vec3 inColor;

out vec4 color;

void main() {
    color = vec4(inColor, 0.2f);
}
