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
	mediump float f = rand(vec2(0, gl_FragCoord.x+rnd));

	lowp vec4 texcolor = texture(texunit, frag_texCoord);
	lowp vec4 color = vec4(1.0f, 1.0f-f*0.5f, 1.0f-f, texcolor.w);
	out_color = texcolor * frag_color + color * intensity * (1+rnd);
}
