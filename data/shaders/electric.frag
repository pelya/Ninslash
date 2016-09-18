#version 300 es
uniform sampler2D texunit;
uniform mediump float rnd;
uniform lowp float intensity;

in highp vec2 frag_texCoord;
in lowp vec4 frag_color;

layout(location = 0) out lowp vec4 out_color;

float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main (void)
{
	mediump float f = rand(vec2(0, frag_texCoord.y)) * rand(vec2(0, frag_texCoord.y+rnd)); // * max(0.2f, 1.0f - cs*0.5);

	lowp vec4 color = vec4(-f*0.5f, f*0.5f, f, 0);
	out_color = texture(texunit, frag_texCoord) * frag_color + color * intensity;
}
