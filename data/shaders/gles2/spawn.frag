#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	//intensity = 1.0;
	float f = max(0.0, rand(vec2(gl_FragCoord.y, gl_FragCoord.x+rnd)) - rand(vec2(0.0, gl_FragCoord.x+rnd))*intensity);
	lowp vec4 t = texture2D(texture, frag_texCoord.st);
	mediump vec4 color = vec4(0, f, 0, t.w * frag_color.w * f);
	
	mediump vec4 c1 = t * frag_color;
	
	//c1.w *= 1.0-intensity;
	//c1.w -= gl_FragCoord.y*0.1;
	
	gl_FragColor = c1*(1.0-intensity*2.0) + color*intensity*2.0;
}
