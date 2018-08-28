$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_combine, 0);
SAMPLER2D(s_depth, 1);

uniform vec4 u_exposure;

void main()
{
    vec3 hdrColor = texture2D(s_combine, v_texcoord0).rgb;
    float depth = texture2D(s_depth, v_texcoord0).r;
    
    float exposure = u_exposure.x;
    vec3 mapped = vec3_splat(1.0) - exp(-hdrColor * exposure);
    gl_FragColor = vec4(mapped, 1.0);
    gl_FragDepth = depth;
}
