cbuffer BloomUpConstantBuffer : register(b0)
{
    float4 BloomUpSizeAndInvSize;
    float3 BloomColor;
};
Texture2D BloomSetup_BloomTexture: register(t0);
SamplerState BloomSetup_BloomSampler: register(s0);

Texture2D BloomUpTexture: register(t1);
SamplerState BloomUpSampler: register(s1);

struct VSInput
{
    float3 position    : POSITION;
    float2 uv        : TEXCOORD0;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float4 TexCoordVignette : TEXCOORD0;
    float4 TexCoords[6]        : TEXCOORD1;
};

float2 Circle(float Start, float Points, float Point)
{
	float Rad = (3.141592 * 2.0 * (1.0 / Points)) * (Point + Start);
	return float2(sin(Rad), cos(Rad));
}

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.position = float4(input.position, 1.0f);

    result.TexCoordVignette.xy = input.uv.xy;

	float Start;
	float Scale;

	Start = 2.0/6.0;
	Scale = 0.66/2.0;

	result.TexCoords[0].xy = input.uv.xy + Circle(Start, 6.0, 0.0) * Scale * BloomUpSizeAndInvSize.zw;
	result.TexCoords[1].xy = input.uv.xy + Circle(Start, 6.0, 1.0) * Scale * BloomUpSizeAndInvSize.zw;
	result.TexCoords[2].xy = input.uv.xy + Circle(Start, 6.0, 2.0) * Scale * BloomUpSizeAndInvSize.zw;
	result.TexCoords[3].xy = input.uv.xy + Circle(Start, 6.0, 3.0) * Scale * BloomUpSizeAndInvSize.zw;
	result.TexCoords[4].xy = input.uv.xy + Circle(Start, 6.0, 4.0) * Scale * BloomUpSizeAndInvSize.zw;
	result.TexCoords[5].xy = input.uv.xy + Circle(Start, 6.0, 5.0) * Scale * BloomUpSizeAndInvSize.zw;

    return result;
}

half4 PSMain(PSInput input) : SV_TARGET
{
    half4 color;

    float Scale1 = 1.0/7.0;
	float Scale2 = 1.0/7.0;

	half3 Bloom2 = (
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoordVignette.xy).rgba *Scale1 +
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoords[0].xy).rgba * Scale2 +
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoords[1].xy).rgba * Scale2 +
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoords[2].xy).rgba * Scale2 +
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoords[3].xy).rgba * Scale2 +
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoords[4].xy).rgba * Scale2 +
		BloomSetup_BloomTexture.Sample(BloomSetup_BloomSampler, input.TexCoords[5].xy).rgba * Scale2) * rcp(Scale1 * 1.0 + Scale2 * 6.0);

	color.rgb = BloomUpTexture.Sample(BloomUpSampler, input.TexCoordVignette.xy);

	half Scale3 = 1.0/5.0;
	color.rgb *= Scale3;

	color.rgb += Bloom2 * Scale3 * BloomColor;
	color.a = 1.0f;

    return color;
}