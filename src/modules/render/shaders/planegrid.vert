// https://dev.to/javiersalcedopuyo/simple-infinite-grid-shader-5fah

layout(std140) uniform u_uniformblock {
	vec3 camera_pos;
	mat4 view;
	mat4 proj;
};

const vec4 positions[4] = vec4[4](
	vec4(-0.5, 0.0,  0.5, 1.0),
	vec4( 0.5, 0.0,  0.5, 1.0),
	vec4(-0.5, 0.0, -0.5, 1.0),
	vec4( 0.5, 0.0, -0.5, 1.0)
);

$out vec4 v_position;
flat $out vec3 v_camera_pos;
$out vec2 v_coords;

// Vertex shader
void main() {
	vec4 world_pos = positions[gl_VertexID];
	world_pos.xyz *= grid_size;
	world_pos.xz += camera_pos.xz; // Make the quad follows the camera for "infinity"

	v_position = proj * view * world_pos;
	v_camera_pos = camera_pos;
	v_coords = world_pos.xz;

	gl_Position = v_position;
}
