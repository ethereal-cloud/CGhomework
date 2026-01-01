#version 330 core
in vec3 vNormal;
out vec4 FragColor;

void main()
{
    vec3 n = normalize(vNormal);
    float t = clamp(n.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 topColor = vec3(1.0, 1.0, 1.0);
    vec3 bottomColor = vec3(0.5, 0.5, 0.5);
    vec3 color = mix(bottomColor, topColor, t);
    FragColor = vec4(color, 1.0);
}
