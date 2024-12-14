#version 460 core

layout (location = 0) out vec4 FragOut;

in vec3 WorldPos;
in vec2 TexCoord;
in vec3 Normal;

uniform vec3 viewPos;

uniform bool flipNormals;

layout (binding = 0) uniform sampler2D hairTexture;
layout (binding = 1) uniform sampler2D hairOpacityTexture;

const float shineDamper = 50.0;
const float reflectivity = 2;

void main() {

    vec3 hairColor = texture2D(hairTexture, TexCoord).rgb;
    float hairOpacity = texture2D(hairOpacityTexture, TexCoord).r;

    vec3 lightPosition = (vec3(7, 0, 10) * 0.5) + vec3(0, 1, 0);
    //vec3 lightColor = vec3(1, 1, 1);
    vec3 lightColor = vec3(1, 0.98, 0.94);
    float lightRadius = 10;
    float lightStrength = 7;

    vec3 normal = normalize(Normal);
    
    if (flipNormals) {
       normal *= -1;
    }
    
    vec3 darkBrownHair = vec3(0.15, 0.1, 0.05);
    lightColor = mix(lightColor, darkBrownHair, 0.95);

    vec3 lightDir = normalize(lightPosition - WorldPos);
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    float distance = length(lightPosition - WorldPos);
    float attenuation = 1.0 / (distance * distance);  // Inverse square law for point light attenuation
    float spec = max(dot(halfVec, normal), 0.0);
    spec = pow(spec, shineDamper);
    
    float ndotl = clamp(dot(normal, lightDir), 0.0, 1.0);
    vec3 specular = lightColor * lightStrength * spec * attenuation * reflectivity * ndotl;
    
    
    hairColor *= darkBrownHair;
    hairColor *= 0.5;

    float diff = max(dot(normal, lightDir), 0.0);
   // diff = 1.0;
    vec3 diffuse = diff * lightColor * hairColor;

   // hairColor = vec3(1,1,1);\

   
    FragOut = vec4(diffuse + specular, hairOpacity);
    //FragOut.rgb *= ndotl;
    //FragOut = vec4(0,0,0, hairOpacity);
   // FragOut = vec4(hairColor * specular, 1);
}
