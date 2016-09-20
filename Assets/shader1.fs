#version 330

in vec3 inColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform sampler2D mytexture;

uniform vec3 customColor;

void main() {
	// Ambient
    float ambientStrength = 0.3f;
    vec3 ambient = ambientStrength /* * vec3(texture(mytexture, TexCoords)) */ * lightColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff /* * vec3(texture(mytexture, TexCoords)) */ * lightColor;

    vec3 result = (ambient + diffuse) * inColor;
    color = vec4(result, 1.0f);
    
    //color = vec4(inColor, 1.0f);
}
