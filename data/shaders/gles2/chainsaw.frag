#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

uniform highp int screenwidth;
uniform highp int screenheight;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	mediump float StepX = 4.0 / float(screenwidth);
	mediump float StepY = 4.0 / float(screenheight);
	const mediump float chainsawHandleStart = 236.0 / 512.0;
	const mediump float chainsawBladeEnd = 320.0 / 512.0;
	mediump float gradient = clamp( 2.0 * (frag_texCoord.x - chainsawHandleStart) / (chainsawBladeEnd - chainsawHandleStart), 0.0, 1.0);

	lowp vec4 t = texture2D(texture, frag_texCoord);
	mediump float nearbyAlpha =
		texture2D(texture, frag_texCoord + vec2(0.0, -StepY)).a +
		texture2D(texture, frag_texCoord + vec2(StepX, StepY)).a +
		texture2D(texture, frag_texCoord + vec2(-StepX, StepY)).a;
	nearbyAlpha = min(1.0, nearbyAlpha) * intensity * gradient;
	nearbyAlpha = min(1.0 - t.a, nearbyAlpha);

	gl_FragColor = vec4(min(1.0, t.r + nearbyAlpha), min(1.0, t.g + nearbyAlpha), t.b, t.a + nearbyAlpha) * frag_color;
}
