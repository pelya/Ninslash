uniform highp float screenWidthDiv; // = 2.0 / screenWidth
uniform highp float screenHeightDiv; //  = 2.0 / screenHeight
uniform mediump float rnd;
uniform mediump float intensity;

layout(location = 0) in highp vec2 in_position;
layout(location = 1) in highp vec2 in_texCoord;
layout(location = 2) in lowp vec4 in_color;

layout(location = 0) out highp vec2 out_texCoord;
layout(location = 1) out lowp vec2 out_color;

void main(void)
{
	gl_Position = vec4(
		in_position.x * screenWidthDiv - 1.0f,
		1.0f - in_position.y * screenHeightDiv,
		0, 1);
	out_texCoord = in_texCoord;
	out_color = in_color;
}

float rand(vec2 co){
	return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
	gl_Position = vec4(
		in_position.x * screenWidthDiv - 1.0f,
		1.0f - in_position.y * screenHeightDiv,
		0, 1);

	mediump float f = rand(vec2(0, gl_TexCoord[0].y + rnd));
	gl_Position.x += f * 0.015f;

	out_texCoord = in_texCoord;
	out_color = in_color;
}
