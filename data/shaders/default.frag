uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

layout(location = 0) in highp vec2 in_texCoord;
layout(location = 1) in lowp vec2 in_color;

layout(location = 0) out lowp vec4 out_color;

void main (void)
{
	out_color = texture(texture, in_texCoord) * in_color;
}
