$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_albedo, 0);
SAMPLER2D(s_emit, 1);
SAMPLER2D(s_lightMap, 5);

void main()
{
    vec3 color = toLinear(texture2D(s_albedo, v_texcoord0).rgb);
    vec4 emit = texture2D(s_emit, v_texcoord0);
    vec4 lightMap = texture2D(s_lightMap, v_texcoord0) + texture2D(s_emit, v_texcoord0);
    vec3 lightAndEmit = lightMap.rgb * (1.0 - emit.a) + emit.rgb * emit.a;
    gl_FragColor = vec4(toGamma(color * lightAndEmit), 1.0);
}
