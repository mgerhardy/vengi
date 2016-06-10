// attributes from the VAOs
$in uvec3 a_pos;
$in uvec2 a_info;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform sampler2D u_texture;
uniform vec4 u_materialcolor[32];
#if cl_deferred == 0
uniform vec3 u_lightpos;
uniform vec3 u_diffuse_color;
uniform float u_fogrange;
uniform float u_viewdistance;
uniform float u_debug_color;
#endif

$out vec3 v_pos;
$out vec4 v_color;
#if cl_deferred == 0
$out vec3 v_lightpos;
$out vec3 v_diffuse_color;
$out vec3 v_fogcolor;
$out float v_fogrange;
$out float v_viewdistance;
$out float v_ambientocclusion;
$out float v_debug_color;
#endif

void main(void) {
	uint a_ao = a_info[0];
	uint a_material = a_info[1];
	vec4 pos4 = u_model * vec4(a_pos, 1.0);
	v_pos = pos4.xyz;

	vec3 materialColor = u_materialcolor[a_material].rgb;
	vec3 colornoise = texture(u_texture, abs(pos4.xz) / 256.0 / 10.0).rgb;
	v_color = vec4(materialColor * colornoise * 1.8, u_materialcolor[a_material].a);
	v_color = clamp(v_color, 0.0, 1.0);

#if cl_deferred == 0
	const float aovalues[] = float[](0.15, 0.6, 0.8, 1.0);
	v_ambientocclusion = aovalues[a_ao];

	v_fogrange = u_fogrange;
	v_viewdistance = u_viewdistance;
	v_lightpos = u_lightpos;
	v_diffuse_color = u_diffuse_color;
	v_debug_color = u_debug_color;

	// use the air color as fog color, too
	v_fogcolor = u_materialcolor[0].rgb;
#endif

	gl_Position = u_projection * u_view * pos4;
}
