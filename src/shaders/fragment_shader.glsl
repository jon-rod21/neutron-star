#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

void main()
{
    // Star properties
    vec3 starColor = vec3(0.6, 0.8, 1.0); // Blueish white
    float emissiveStrength = 2.0;
    
    // Simple lighting (Phong)
    vec3 lightPos = vec3(0.0, 0.0, 0.0); // Light at center
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 lighting = (ambient + diffuse + specular) * starColor;
    
    // EMISSION TEST - Polar pattern
    // This makes poles (top/bottom) brighter
    vec3 normalizedPos = normalize(FragPos);
    float poleFactor = abs(normalizedPos.y); // 1.0 at poles, 0.0 at equator
    
    // Add some horizontal stripes for extra visibility
    float stripePattern = sin(FragPos.y * 20.0) * 0.5 + 0.5;
    float emissionPattern = poleFactor * 0.7 + stripePattern * 0.3;
    
    vec3 emission = starColor * emissiveStrength * emissionPattern;
    
    // Final color
    vec3 result = lighting + emission;
    FragColor = vec4(result, 1.0);
}
