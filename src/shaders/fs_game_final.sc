$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_albedo, 0);
SAMPLER2D(s_lightMap, 5);

void main()
{
    vec3 color = toLinear(texture2D(s_albedo, v_texcoord0).rgb);
    vec4 lightMap = texture2D(s_lightMap, v_texcoord0);
    gl_FragColor = vec4(toGamma(color * lightMap.xyz), 1.0);
}
