#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

void main()
{
    vec3 lightPos = vec3(2.0, 2.0, 5.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambient = vec3(0.5, 0.5, 0.5);

    vec3 starColor = vec3(0.7, 0.85, 1.0);

    vec3 result = (ambient + diffuse) * starColor;
    /*
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(-FragPos);
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim =  pow(rim, 1.0);

    vec3 starColor = vec3(0.6, 0.8, 1.0);

    vec3 emission = vec3(0.8, 0.9, 1.0) * 1.5;

    vec3 rimGlow = vec3(0.5, 0.7, 1.0) * rim * 2.0;

    vec3 result = starColor + emission + rimGlow;
    */
    FragColor = vec4(result, 1.0);
}

