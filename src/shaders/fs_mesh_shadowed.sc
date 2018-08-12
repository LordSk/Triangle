$input v_position, v_normal, v_shadowcoord, v_color0

#include "common.sh"

#define SHADOW_PACKED_DEPTH 0
#include "mesh_shadow.sh"

uniform vec4 u_lightPos;
uniform vec4 u_lightDir;

void main()
{
	float shadowMapBias = 0.005;
	vec3 color = v_color0.rgb;

	/*vec3 v  = v_view;
	vec3 vd = -normalize(v);
	vec3 n  = v_normal;
	vec3 l  = u_lightPos.xyz;
	vec3 ld = -normalize(l);

	vec2 lc = lit(ld, n, vd, 1.0);*/
    float light = dot(v_normal, -normalize(u_lightDir.xyz));

	vec2 texelSize = vec2_splat(1.0/4096.0);
	float visibility = PCF(s_shadowMap, v_shadowcoord, shadowMapBias, texelSize);

	vec3 ambient = 0.01 * color;
	vec3 brdf = max(light, 0.0) * color * visibility;

	vec3 final = toGamma(abs(ambient + brdf));
	gl_FragColor = vec4(final, 1.0);
}
