#version 100

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	//vec4 color = vec4(1.0f, 1.0f-f*0.5f, 1.0f-f, texture2D(texture, gl_TexCoord[0].st).w);
	float a = texture2D(texture, frag_texCoord.st).r;
	
	float r = 0.7f;
	
	float Step = 0.5f / 1600;
	
	float SumRed = (texture2D(texture, frag_texCoord.st + vec2(-Step, -Step)).r + texture2D(texture, frag_texCoord.st + vec2(+Step, +Step)).r) / 2.0f;
	
	r = SumRed * 0.7f;
	
	
	//r -= (1.0f-texture2D(texture, gl_TexCoord[0].st + vec2(-0.001f, -0.001f)).r)*0.4f;
	
	
	
	if (a < 0.7f)
		a = 0.0f;
	//else
	//	a = 1.0f;
	
	gl_FragColor = vec4(r * frag_color.r, 0, 0, a * frag_color.w);
}
