#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	const float radius = 2.0;
	lowp vec4 t = texture2D(texture, frag_texCoord);
	mediump float nearbyAlpha =
		texture2D(texture, frag_texCoord + vec2(0.0, radius)).a +
		texture2D(texture, frag_texCoord + vec2(0.0, -radius)).a +
		texture2D(texture, frag_texCoord + vec2(radius, 0.0)).a +
		texture2D(texture, frag_texCoord + vec2(-radius, 0.0)).a;
	nearbyAlpha = max(1.0, nearbyAlpha);
	nearbyAlpha *= intensity;
	nearbyAlpha = min(1.0 - t.a, nearbyAlpha);

	gl_FragColor = vec4(t.r + nearbyAlpha, t.g + nearbyAlpha, t.b, t.a) * frag_color;
	//gl_FragColor = vec4(t.r, t.g, t.b, t.a) * frag_color;
}
