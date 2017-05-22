#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

float rand(vec2 co)
{
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt= dot(co.xy ,vec2(a,b));
    float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main (void)
{
	float f = rand(vec2(0.0, frag_texCoord.y/4.0)) * rand(vec2(0.0, frag_texCoord.y/4.0+rnd));// * max(0.2f, 1.0f - cs*0.5);

	lowp vec4 color = vec4(-f*0.5, f*0.5, f, 0.0);
	
	float x = sin(frag_texCoord.x/16.0+intensity)*0.5 + sin(frag_texCoord.y/12.0+intensity)*0.5;
	x = clamp(x, 0.0, 0.75);
	x *= rand(vec2(0.0, frag_texCoord.y));
	x *= rnd;
	color += vec4(x*0.25, x*0.5, x, 0.0);
	
	vec4 c = texture2D(texture, frag_texCoord.st);
	
	lowp float r = c.r;
	lowp float b = c.b;
	c.r = r*(1.0-colorswap) + b*colorswap;
	c.b = b*(1.0-colorswap) + r*colorswap;
	
	gl_FragColor = c * frag_color + color*intensity;
}
