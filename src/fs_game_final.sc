$input v_texcoord0

#include "common.sh"

SAMPLER2D(u_sdiffuse, 0);

void main()
{
    vec4 color = toGamma(texture2D(u_sdiffuse,  v_texcoord0));
	gl_FragColor = color;
}
