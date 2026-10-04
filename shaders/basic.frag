#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in float fragDist;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    vec4 color;
    vec4 fogColor;
    vec4 fogParams;
    vec4 misc;
    vec4 light0;
    vec4 light1;
} push;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 c = fragColor;
    if (push.misc.x > 0.5)
        c *= texture(texSampler, fragUV);

    if (push.misc.y >= 0.0 && c.a <= push.misc.y)
        discard;

    if (push.fogParams.w > 0.5) {
        float f;
        if (push.fogParams.w < 1.5)
            f = (push.fogParams.y - fragDist) / (push.fogParams.y - push.fogParams.x);
        else
            f = exp(-push.fogParams.z * fragDist);
        c.rgb = mix(push.fogColor.rgb, c.rgb, clamp(f, 0.0, 1.0));
    }

    outColor = c;
}
