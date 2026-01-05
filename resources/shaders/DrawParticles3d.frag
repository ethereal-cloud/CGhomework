#version 450

in vec3 vColor;
in float vDepth;

out vec4 FragColor;

void main() {
    vec2 uv = gl_PointCoord - vec2(0.5);
    float r = length(uv);
    float alpha = smoothstep(0.5, 0.45, r);
    float depthFactor = clamp(1.2 - vDepth * 0.25, 0.5, 1.2);
    float core = 1.0 - smoothstep(0.0, 0.25, r);
    vec3 color = vColor * depthFactor;
    color = mix(color, vec3(1.0), core * 0.2);
    FragColor = vec4(color, alpha);
}
