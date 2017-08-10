#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform highp vec4 screenPos;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

float rand(vec2 co)
{
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 hash3(vec2 p)
{
	vec3 q = vec3( dot(p,vec2(127.1,311.7)),
				   dot(p,vec2(269.5,183.3)),
				   dot(p,vec2(419.2,371.9)) );
	return fract(sin(q)*43758.5453);
}

float iqnoise(in vec2 x, float u, float v)
{
	vec2 p = floor(x);
	vec2 f = fract(x);

	float k = 1.0+63.0*pow(1.0-v,4.0);
	
	float va = 0.0;
	float wt = 0.0;
	for( mediump int j=-2; j<=2; j++ )
	for( mediump int i=-2; i<=2; i++ )
	{
		vec2 g = vec2( float(i),float(j) );
		vec3 o = hash3( p + g )*vec3(u,u,1.0);
		vec2 r = g - f + o.xy;
		float d = dot(r,r);
		float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k);
		va += o.z*ww;
		wt += ww;
	}
	
	return va/wt;
}

vec3 color(vec2 pt, float size)
{
	float rInv = min(size, intensity*size/45.0)/length(pt);
	pt = pt * rInv - vec2(rInv+2.*mod(intensity,6000.0),0.0);

	return vec3(iqnoise(0.5*pt, 2.0, 1.0)+0.240*rInv);
}

void main (void)
{
	// screenPos.w = 2.0f / screenheight // screenheight = 2.0f / screenPos.w
	vec2 scale = vec2(35.0, 27.0);
	// SLOW!
	//vec3 c = color(vec2(frag_texCoord.x*4.0/scale.x-1.0/(scale.x*screenPos.z), frag_texCoord.y*4.0/scale.y-1.0/(scale.y*screenPos.w)), 2.0/(50.0*screenPos.z));
	vec3 c = vec3(frag_texCoord.x*4.0/scale.x-1.0/(scale.x*screenPos.z), frag_texCoord.y*4.0/scale.y-1.0/(scale.y*screenPos.w), 2.0/(50.0*screenPos.z));

	float s = 1.0-c.b*1.0;
	
	float a = min(1.0, intensity*0.05);
	s *= a;

	gl_FragColor = vec4(0, s*(0.55+cos(intensity*0.15)*0.2), s, 0.6*a) * frag_color;
}
