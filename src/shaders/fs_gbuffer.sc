$input v_position, v_normal, v_color0, v_texcoord0

#include "common.sh"

SAMPLER2D(s_emit, 0);

void main()
{
	gl_FragData[0] = vec4(v_color0.rgb, 1.0);
    gl_FragData[1] = texture2D(s_emit, v_texcoord0) * vec4(v_color0.rgb, 1.0);
	gl_FragData[2] = vec4(v_position.xyz, 1.0);
	gl_FragData[3] = vec4((v_normal + 1.0) * 0.5, 1.0);
}
