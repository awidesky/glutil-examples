#version 330 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

layout(location = 0) out vec4 color;

uniform sampler2D myTextureSampler;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform bool isUI = false;

void main(){
    vec4 tex = texture(myTextureSampler, UV);
    if(isUI){
        color = tex;
        return;
    }
    vec3 objectColor = tex.rgb;
    
    vec3 ambient = ambientStrength * objectColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = 0.4 * diff * objectColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    const vec3 lightColor = vec3(2.0);
    vec3 specular = 0.5 * spec * lightColor;

    vec3 result = ambient + diffuse + specular;
    color = vec4(result, tex.a);
}