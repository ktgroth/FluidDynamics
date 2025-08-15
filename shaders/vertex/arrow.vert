#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 cellCenter;
layout(location=2) in vec2 velocity;

uniform mat4 projection;
uniform float arrowScale;

void main(){
    float angle = atan(velocity.y, velocity.x);
    float mag = length(velocity);

    mat2 rot = mat2(cos(angle), -sin(angle),
                    sin(angle),  cos(angle));

    vec2 pos = cellCenter + rot * (aPos * mag * arrowScale);
    gl_Position = projection * vec4(pos, 0.0, 1.0);
}

