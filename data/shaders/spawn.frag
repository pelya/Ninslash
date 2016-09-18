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
	mediump float f = max(0, rand(vec2(gl_FragCoord.y, gl_FragCoord.x+rnd)) - rand(vec2(0, gl_FragCoord.x+rnd))*intensity);

	lowp vec4 texcolor = texture(texunit, frag_texCoord);
	lowp vec4 color = vec4(0, f, 0, texcolor.w * frag_color.w * f);

	lowp vec4 c1 = texcolor * frag_color;

	//c1.w *= 1.0f-intensity;
	//c1.w -= gl_FragCoord.y*0.1f;
	
	out_color = c1*(1-intensity*2) + color*intensity*2;
}
