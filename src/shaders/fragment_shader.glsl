#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    //psr j1748, aka B2 sequence star color
    vec3 objectColor = vec3(0.659, 0.773, 1.0);

    //ambient light
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;


    //emissive
    float emissiveStrength = 1.5;
    vec3 emissive = objectColor  * emissiveStrength;


    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightPos = vec3(2.0, 2.0, 5.0);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //spec
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (emissive + diffuse + specular);

    //vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}

