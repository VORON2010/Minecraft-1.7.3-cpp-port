#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec4 inNormal;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out float fragDist;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec4 color;     // glColor, or white when the vertices carry color
    vec4 fogColor;
    vec4 fogParams; // start, end, density, mode (0 off, 1 linear, 2 exp)
    vec4 misc;      // useTexture, alphaRef (<0 = no alpha test), lighting, ambient
    vec4 light0;    // object-space direction, diffuse
    vec4 light1;
} push;

void main() {
    gl_Position = push.mvp * vec4(inPosition, 1.0);
    fragUV = inUV;
    fragColor = inColor * push.color;

    // Beta's Lighting.turnOn: two directional lights + model ambient,
    // color material on ambient and diffuse
    if (push.misc.z > 0.5) {
        float len = length(inNormal.xyz);
        if (len > 0.01) {
            vec3 n = inNormal.xyz / len;
            float l = push.misc.w
                + push.light0.w * max(dot(n, push.light0.xyz), 0.0)
                + push.light1.w * max(dot(n, push.light1.xyz), 0.0);
            fragColor.rgb *= min(l, 1.0);
        }
    }

    // clip w is eye-space depth for a perspective projection, like GL fog
    fragDist = abs(gl_Position.w);
}
