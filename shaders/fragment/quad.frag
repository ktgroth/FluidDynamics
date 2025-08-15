#version 330 core

in vec2 uv;

out vec4 FragColor;

uniform sampler2D fluidTex;

void main() {
    float dens = texture(fluidTex, uv).b;
    FragColor = vec4(0.0, 0.0, dens, 1.0); // blue fluid
}
