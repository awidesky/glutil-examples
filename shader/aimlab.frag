#version 330 core

in vec3 vWorldPos;
in vec3 vViewPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 uCameraPos;

uniform vec3 uLightPos;
uniform vec3 uLightColor;

uniform vec3 uAmbientColor;
uniform vec3 uDiffuseColor;
uniform vec3 uSpecularColor;

uniform float uShininess;

uniform sampler2D uTexture;
uniform bool uUseTexture;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vWorldPos);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 R = reflect(-L, N);

    // Ambient
    vec3 ambient = uAmbientColor * uLightColor;

    // Diffuse (Lambert)
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uDiffuseColor * uLightColor;

    // Specular (Phong)
    float spec = pow(max(dot(V, R), 0.0), uShininess);
    vec3 specular = spec * uSpecularColor * uLightColor;

    vec3 lighting = ambient + diffuse + specular;

    vec3 baseColor = vec3(1.0);
    if (uUseTexture) {
        baseColor = texture(uTexture, vUV).rgb;
    }

    FragColor = vec4(lighting * baseColor, 1.0);
}