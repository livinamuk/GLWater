#version 460 core

layout (location = 0) out vec4 FragOut;
layout (location = 1) out vec4 PositionOut;
layout (binding = 0) uniform sampler2D baseColorTexture;
layout (binding = 1) uniform sampler2D hairTexture;

in vec2 TexCoord;
in vec3 Normal;
in vec3 WorldPos;

uniform vec3 viewPos;
uniform int settings;
uniform float viewportWidth;
uniform float viewportHeight;


/////////////////////////
//                     //
//   Direct Lighting   //


const float PI = 3.14159265359;

float D_GGX(float NoH, float roughness) {
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float NoH2 = NoH * NoH;
  float b = (NoH2 * (alpha2 - 1.0) + 1.0);
  return alpha2 / (PI * b * b);
}

float G1_GGX_Schlick(float NdotV, float roughness) {
  //float r = roughness; // original
  float r = 0.5 + 0.5 * roughness; // Disney remapping
  float k = (r * r) / 2.0;
  float denom = NdotV * (1.0 - k) + k;
  return NdotV / denom;
}

float G_Smith(float NoV, float NoL, float roughness) {
  float g1_l = G1_GGX_Schlick(NoL, roughness);
  float g1_v = G1_GGX_Schlick(NoV, roughness);
  return g1_l * g1_v;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 microfacetBRDF(in vec3 L, in vec3 V, in vec3 N, in vec3 baseColor, in float metallicness, in float fresnelReflect, in float roughness, in vec3 WorldPos) {
  vec3 H = normalize(V + L); // half vector
  // all required dot products
  float NoV = clamp(dot(N, V), 0.0, 1.0);
  float NoL = clamp(dot(N, L), 0.0, 1.0);
  float NoH = clamp(dot(N, H), 0.0, 1.0);
  float VoH = clamp(dot(V, H), 0.0, 1.0);
  // F0 for dielectics in range [0.0, 0.16]
  // default FO is (0.16 * 0.5^2) = 0.04
  vec3 f0 = vec3(0.16 * (fresnelReflect * fresnelReflect));
  // f0 = vec3(0.125);
  // in case of metals, baseColor contains F0
  f0 = mix(f0, baseColor, metallicness);
  // specular microfacet (cook-torrance) BRDF
  vec3 F = fresnelSchlick(VoH, f0);
  float D = D_GGX(NoH, roughness);
  float G = G_Smith(NoV, NoL, roughness);
  vec3 spec = (D * G * F) / max(4.0 * NoV * NoL, 0.001);

  // diffuse
  vec3 notSpec = vec3(1.0) - F; // if not specular, use as diffuse
  notSpec *= 1.0 - metallicness; // no diffuse for metals
  vec3 diff = notSpec * baseColor / PI;
  spec *= 1.05;
  vec3 result = diff + spec;

  return result;
}

vec3 GetDirectLighting(vec3 lightPos, vec3 lightColor, float radius, float strength, vec3 Normal, vec3 WorldPos, vec3 baseColor, float roughness, float metallic, vec3 viewPos) {
	
    
    float fresnelReflect = 1.0; // 0.5 is what they used for box, 1.0 for demon

	vec3 viewDir = normalize(viewPos - WorldPos);
	float lightRadiance = strength;
	vec3 lightDir = normalize(lightPos - WorldPos);
	float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));
	// lightAttenuation = clamp(lightAttenuation, 0.0, 0.9); // THIS IS WRONG, but does stop super bright region around light source and doesn't seem to affect anything else...
	float irradiance = max(dot(lightDir, Normal), 0.0) ;
	irradiance *= lightAttenuation * lightRadiance;
	vec3 brdf = microfacetBRDF(lightDir, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness, WorldPos);
	return brdf * irradiance * clamp(lightColor, 0, 1);
}


void main() {

    vec3 baseColor = texture2D(baseColorTexture, TexCoord).rgb;
    vec3 color = baseColor;
    vec3 normal = normalize(Normal);
    float roughness = baseColor.r;
    float metallic = baseColor.b + 0.2;
    
    // Mermaid tail
    if (settings == 1) {
        roughness *= 0.75;
        metallic = 0.1;
        //metallic = 0.8;
    }
    // Mermaid skin
    if (settings == 2) {
        roughness = 0.8;
        metallic = 0.2;
    }
    // Mermaid hair
    if (settings == 3 || settings == 4) {
        roughness = 0.95;
        metallic = 0.2;
        vec3 darkBrownHair = vec3(0.15, 0.1, 0.05);
        baseColor = darkBrownHair * 0.1;
    }
    vec3 lightPosition = (vec3(7, 0, 10) * 0.5) + vec3(0, 0.125, 0);
    //vec3 lightColor = vec3(1, 1, 1);
    vec3 lightColor = vec3(1, 0.98, 0.94);
    float lightRadius = 10;
    float lightStrength = 4;

    const vec3 lightDirection = normalize(lightPosition - WorldPos);
    vec3 ligthting = GetDirectLighting(lightPosition, lightColor, lightRadius, lightStrength, normal, WorldPos, baseColor, roughness, metallic, viewPos);

    
    if (settings == 4) {
        vec2 screenUV = vec2(gl_FragCoord) / vec2(viewportWidth, viewportHeight);
        vec4 hairOverlay = texture2D(hairTexture, screenUV).rgba;
        ligthting = hairOverlay.rgb;

        if (hairOverlay.a < 0.001) {
            //discard;
        }
    }

    FragOut = vec4(ligthting, 1.0);
    //FragOut.rgb = baseColor;
    //FragOut.rgb = WorldPos;
    //FragOut.rgb = normal * 0.5 + 0.5;
    //FragOut.rgb = vec3(diff);
	FragOut.a = 1.0;

    PositionOut = vec4(WorldPos.rgb, 1.0);
}
