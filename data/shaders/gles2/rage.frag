#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	float f = rand(vec2((gl_FragCoord.y/2.0), (gl_FragCoord.x/2.0)+rnd))*rand(vec2((gl_FragCoord.y/2.0), (gl_FragCoord.x/2.0)+rnd));

	lowp vec4 c = texture2D(texture, frag_texCoord.st);
	float c1 = c.x + c.y + c.z;
	c1 *= 3.0;
	
	lowp vec4 color = vec4((1.0 - min(c1, 1.0)) * f, 0.0, 0.0, 0.0);

	float x = sin(frag_texCoord.x*16.0+intensity)*0.5 + sin(frag_texCoord.y*12.0+intensity)*0.5;
	x = clamp(x, 0.0, 0.75);
	x *= rand(vec2(0.0, frag_texCoord.y));
	x *= rnd;
	color += vec4(x, 0.0, 0.0, 0.0);

	lowp float r = c.r;
	lowp float b = c.b;
	c.r = r*(1.0-colorswap) + b*colorswap;
	c.b = b*(1.0-colorswap) + r*colorswap;
	
	gl_FragColor = c * frag_color + color*intensity;
}
