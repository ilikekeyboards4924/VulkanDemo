#version 450

layout(location = 0) out vec3 fragColor;

vec2 positions1[3] = vec2[](
    vec2(1.0, -5),
    vec2(1, 1),
    vec2(-1, 1)
);

// X positive is RIGHT, Y positive is DOWN
// if the coordinates are not CLOCKWISE, the graphics pipeline will cull them. FRONT_FACING_CLOCKWISE
// also remember that last item in GLSL array CANNOT have a comma after it
vec2 positions[6] = vec2[](
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),

    vec2(1, -1),
    vec2(1, 1),
    vec2(-1, 1)
);

vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),

    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 0.0, 0.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}