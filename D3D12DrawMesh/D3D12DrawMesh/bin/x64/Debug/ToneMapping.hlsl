struct VSInput
{
    float3 position    : POSITION;
    float2 uv        : TEXCOORD0;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float2 uv        : TEXCOORD0;
};

Texture2D BloomTexture : register(t0);
Texture2D SceneColorTexture : register(t1);
SamplerState SceneColorSampler : register(s0);
// SamplerState BloomSampler : register(s1);

float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.position = float4(input.position, 1.0f);
    result.uv = input.uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // float4 color;
    // float4 SceneColor = SceneColorTexture.Sample(SceneColorSampler, input.uv);
    // half3 LinearColor =  SceneColor.rgb;
    // float3 CombinedBloom = BloomTexture.Sample(BloomSampler, input.uv);
    // LinearColor = LinearColor.rgb + CombinedBloom.rgb;
    // color.rgb = ACESToneMapping(LinearColor, 1.0f);
    // color.a = SceneColor.a;

    // return color;

	float4 color;
    float4 SceneColor = SceneColorTexture.Sample(SceneColorSampler, input.uv);
    half3 LinearColor =  SceneColor.rgb;
    LinearColor = LinearColor.rgb;
    color.rgb = ACESToneMapping(LinearColor, 1.0f);
    color.a = SceneColor.a;

    return color;
}