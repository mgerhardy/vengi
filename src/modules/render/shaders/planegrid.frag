// https://dev.to/javiersalcedopuyo/simple-infinite-grid-shader-5fah

$in vec4 v_position;
flat $in vec3 v_camera_pos;
$in vec2 v_coords;

out vec4 o_color;

const float grid_size = 100.0;
const float cell_size = 1.0;
const float half_cell_size = cell_size * 0.5;
const float cell_line_thickness = 0.005;

const float subcell_size = 0.1;
const float half_subcell_size = subcell_size * 0.5;
const float subcell_line_thickness = 0.001;

const vec4 cell_color = vec4(0.5, 0.5, 0.5, 1.0);
const vec4 subcell_color = vec4(0.75, 0.75, 0.75, 1.0);

const float height_to_fade_distance_ratio = 25.0;
const float min_fade_distance = grid_size * 0.05;
const float max_fade_distance = grid_size * 0.5;

// Fragment shader
void main() {
	// Offset coordinates for grid alignment
	vec2 cell_coords = mod(v_coords + half_cell_size, cell_size);
	vec2 subcell_coords = mod(v_coords + half_subcell_size, subcell_size);

	// Distance to edges
	vec2 distance_to_cell = abs(cell_coords - half_cell_size);
	vec2 distance_to_subcell = abs(subcell_coords - half_subcell_size);

	// Line thickness adjustment
	vec2 d = fwidth(v_coords);
	float adjusted_cell_line_thickness = 0.5 * (cell_line_thickness + d.x);
	float adjusted_subcell_line_thickness = 0.5 * (subcell_line_thickness + d.x);

	vec4 color = vec4(0.0);
	if (any(lessThan(distance_to_subcell, vec2(adjusted_subcell_line_thickness)))) {
		color = subcell_color;
	}
	if (any(lessThan(distance_to_cell, vec2(adjusted_cell_line_thickness)))) {
		color = cell_color;
	}

	// Fade out around the camera to hide visual artifacts
	float opacity_falloff;
	{
		float distance_to_camera = length(v_coords - v_camera_pos.xz);
		// Adjust the fade distance relative to the camera height
		float fade_distance = abs(v_camera_pos.y) * height_to_fade_distance_ratio;
		{
			fade_distance = max(fade_distance, min_fade_distance);
			fade_distance = min(fade_distance, max_fade_distance);
		}
		opacity_falloff = smoothstep(1.0, 0.0, distance_to_camera / fade_distance);
	}

	color.a *= opacity_falloff;
	o_color = color;
}
