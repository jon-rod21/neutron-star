#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

void main()
{
    //does work
//    vec3 lightPos = vec3(2.0, 2.0, 5.0);
//    vec3 lightColor = vec3(1.0, 1.0, 1.0);
//
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(lightPos - FragPos);
//
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * lightColor;
//
//    vec3 ambient = vec3(0.5, 0.5, 0.5);
//
//    vec3 starColor = vec3(0.7, 0.85, 1.0);
//
//    vec3 result = (ambient + diffuse) * starColor;

    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    //psr j1748, aka B2 sequence star color
    vec3 objectColor = vec3(0.659, 0.773, 1.0);

    //ambient light
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    //diffuse
    vec3 norm = normalize(Normal);
    vec3 lightPos = vec3(2.0, 2.0, 5.0);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;



    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}

