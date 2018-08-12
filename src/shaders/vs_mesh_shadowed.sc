$input a_position, a_normal
$output v_position, v_normal, v_shadowcoord, v_color0

#include "common.sh"

uniform mat4 u_lightMtx;
uniform vec4 u_color;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_position = mul(u_model[0], vec4(a_position, 1.0));

	v_normal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);

	const float shadowMapOffset = 0.001;
	vec3 posOffset = a_position + a_normal * shadowMapOffset;
    vec4 worldPosOff = mul(u_model[0], vec4(posOffset, 1.0));
	v_shadowcoord = mul(u_lightMtx, worldPosOff);
    
    v_color0 = u_color;
}
