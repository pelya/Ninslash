#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	//vec4 color = vec4(1.0, 1.0-f*0.5, 1.0-f, texture2D(texture, gl_TexCoord[0].st).w);
	float a = texture2D(texture, frag_texCoord.st).r;
	
	float r = 0.7;
	
	float Step = 0.5 / 1600.0;
	
	float SumRed = (texture2D(texture, frag_texCoord.st + vec2(-Step, -Step)).r + texture2D(texture, frag_texCoord.st + vec2(+Step, +Step)).r) / 2.0;
	
	r = SumRed * 0.7;
	
	
	//r -= (1.0-texture2D(texture, gl_TexCoord[0].st + vec2(-0.001, -0.001)).r)*0.4;
	
	
	
	if (a < 0.7)
		a = 0.0;
	//else
	//	a = 1.0;
	
	gl_FragColor = vec4(r * frag_color.r, 0.0, 0.0, a * frag_color.w);
}
