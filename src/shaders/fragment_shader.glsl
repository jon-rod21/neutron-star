#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor; 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
uniform float time;
uniform vec3 viewPos;
uniform vec3 starColor;
uniform float emissiveStrength;

// Hash for randomness
vec3 hash3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453123);
}

// 3D noise
float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = i.x + i.y * 157.0 + 113.0 * i.z;
    
    vec4 v1 = fract(753.5453123 * sin(n + vec4(0.0, 1.0, 157.0, 158.0)));
    vec4 v2 = fract(753.5453123 * sin(n + vec4(113.0, 114.0, 270.0, 271.0)));
    
    vec4 v3 = mix(v1, v2, f.z);
    vec2 v4 = mix(v3.xy, v3.zw, f.y);
    return mix(v4.x, v4.y, f.x);
}

// Fractal Brownian Motion
float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 5; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    vec3 lightPos = vec3(0.0, 0.0, 0.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 lighting = (ambient + diffuse + specular) * starColor;
    
    // Surface noise using FBM
    vec3 noisePos = FragPos * 4.0 + vec3(time * 0.5); 
    float surfaceNoise = fbm(noisePos);

    // Anything below 0.3 becomes pure black. Anything above 0.7 becomes max brightness.
    // The values in between interpolate smoothly.
    surfaceNoise = smoothstep(0.1, 0.9, surfaceNoise);
    // Exponentially deepens the dark spots
    surfaceNoise = pow(surfaceNoise, 1.25);
    
    // Poles brighter, its a pulsar after all
    vec3 normalizedPos = normalize(FragPos);
    float poleFactor = pow(abs(normalizedPos.y), 2.0);
    
    // Emission varying across surface
    float emissionPattern = 0.7 + surfaceNoise * 0.8 + poleFactor * 0.3;
    vec3 baseEmission = starColor * emissiveStrength * emissionPattern;

    /* Rim Lighting */
    // Calculate the angle between the camera view and the surface normal.
    // 1.0 at the absolute edges, 0.0 in the dead center.
    float rimFactor = 1.0 - max(dot(viewDir, norm), 0.0);
    rimFactor = smoothstep(0.6, 1.0, rimFactor); 
    float rimIntensity = 1.0; 
    vec3 rimEmission = starColor * rimFactor * rimIntensity;

    // Combine Emissions
    vec3 totalEmission = baseEmission + rimEmission;

    vec3 result = lighting + totalEmission;
    FragColor = vec4(result, 1.0);
    
    // Check brightness and output to the second buffer if it exceeds a threshold
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
