#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform highp vec4 screenPos;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	highp float StepX = screenPos.z;
	highp float StepY = screenPos.w;

	vec4 c = (texture2D(texture, frag_texCoord.st + vec2(-StepX, -StepY)) + texture2D(texture, frag_texCoord.st + vec2(+StepX, +StepY))) / 2.0;

	float a = clamp(c.r + c.g + c.b, 0.0, 1.0);
	
	c *= frag_color;
	
	c.g -= (c.r + c.b) / 1.3;
	c.r -= (c.g + c.b) / 1.3;
	c.b -= (c.g + c.r) / 1.3;
	
	c.r = clamp(c.r, 0.0, 0.5);
	c.g = clamp(c.g, 0.0, 0.5);
	c.b = clamp(c.b, 0.0, 0.1);
	
	a = step(0.7, a);
	c.a = a * frag_color.w;

	gl_FragColor = c;
}
