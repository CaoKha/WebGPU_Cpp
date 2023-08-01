struct MyUniforms {
  color: vec4f,
  time: f32
}

@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;

struct VertexInput {
	@location(0) position: vec2f,
	@location(1) color: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let ratio = 640.0 / 480.0;
	// Offset the shape (before applying the ratio!)
	var offset = vec2f(-0.6875, -0.463);
  var yTime = uMyUniforms.time * 2.0;
  offset += 0.3 * vec2f(sin(uMyUniforms.time), sin(yTime));
	out.position = vec4f(in.position.x + offset.x, (in.position.y + offset.y) * ratio, 0.0, 1.0);
	out.color = in.color;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
  let color = in.color * uMyUniforms.color.rgb;
	// We apply a gamma-correction to the color
	let corrected_color = pow(color, vec3f(2.2));
	return vec4f(corrected_color, uMyUniforms.color.a);
}