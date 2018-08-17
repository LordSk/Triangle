$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_albedo, 0);

void main()
{
    vec4 color = texture2D(s_albedo,  v_texcoord0);
	gl_FragColor = color;
}
