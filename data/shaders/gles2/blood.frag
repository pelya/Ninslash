#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform highp vec4 screenPos;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	float a = texture2D(texture, frag_texCoord.st).r;
	
	float r = 0.7;
	
	highp float StepX = screenPos.z;
	highp float StepY = screenPos.w;
	
	float SumRed = (texture2D(texture, frag_texCoord.st + vec2(-StepX, -StepY)).r + texture2D(texture, frag_texCoord.st + vec2(+StepX, +StepY)).r) / 2.0;
	
	r = SumRed * 0.7;
	
	a = step(0.7, a);
	
	gl_FragColor = vec4(r * frag_color.r, 0.0, 0.0, a * frag_color.w);
}
