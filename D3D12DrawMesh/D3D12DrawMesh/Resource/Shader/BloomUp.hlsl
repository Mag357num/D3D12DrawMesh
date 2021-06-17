Texture2D BloomUpSourceATexture: register(t0);
SamplerState BloomUpSourceASampler: register(s0);

Texture2D BloomUpSourceBTexture: register(t1);
SamplerState BloomUpSourceBSampler: register(s1);

cbuffer BloomUpConstantBuffer : register(b0)
{
    float4 BufferASizeAndInvSize;
    float4 BufferBSizeAndInvSize;
    float4 BloomTintA;
    float4 BloomTintB;
    float2 BloomUpScales;
};

struct VSInput
{
    float3 position    : POSITION;
    float2 uv        : TEXCOORD0;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float4 uv[8]        : TEXCOORD0;
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

    float Start;
	float Scale;

	Start = 2.0/7.0;
	Scale = BloomUpScales.x;

	result.uv[0].xy = input.uv.xy + Circle(Start, 7.0, 0.0) * Scale * BufferASizeAndInvSize.zw;
	result.uv[0].zw = input.uv.xy + Circle(Start, 7.0, 1.0) * Scale * BufferASizeAndInvSize.zw;
	result.uv[1].xy = input.uv.xy + Circle(Start, 7.0, 2.0) * Scale * BufferASizeAndInvSize.zw;
	result.uv[1].zw = input.uv.xy + Circle(Start, 7.0, 3.0) * Scale * BufferASizeAndInvSize.zw;
	result.uv[2].xy = input.uv.xy + Circle(Start, 7.0, 4.0) * Scale * BufferASizeAndInvSize.zw;
	result.uv[2].zw = input.uv.xy + Circle(Start, 7.0, 5.0) * Scale * BufferASizeAndInvSize.zw;
	result.uv[3].xy = input.uv.xy + Circle(Start, 7.0, 6.0) * Scale * BufferASizeAndInvSize.zw;

	result.uv[3].zw = input.uv.xy;

	Start = 2.0/7.0;
	Scale = BloomUpScales.y;

	result.uv[4].xy = input.uv.xy + Circle(Start, 7.0, 0.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[4].zw = input.uv.xy + Circle(Start, 7.0, 1.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[5].xy = input.uv.xy + Circle(Start, 7.0, 2.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[5].zw = input.uv.xy + Circle(Start, 7.0, 3.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[6].xy = input.uv.xy + Circle(Start, 7.0, 4.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[6].zw = input.uv.xy + Circle(Start, 7.0, 5.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[7].xy = input.uv.xy + Circle(Start, 7.0, 6.0) * Scale * BufferBSizeAndInvSize.zw;
	result.uv[7].zw = float2(0.0, 0.0);

    return result;
}

half4 PSMain(PSInput input) : SV_TARGET
{
    half4 color;

    half3 A0 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[0].xy).rgb;
	half3 A1 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[0].zw).rgb;
	half3 A2 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[1].xy).rgb;
	half3 A3 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[1].zw).rgb;
	half3 A4 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[2].xy).rgb;
	half3 A5 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[2].zw).rgb;
	half3 A6 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[3].xy).rgb;
	half3 A7 = BloomUpSourceATexture.Sample(BloomUpSourceASampler, input.uv[3].zw).rgb;

	half3 B0 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[3].zw).rgb;
	half3 B1 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[4].xy).rgb;
	half3 B2 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[4].zw).rgb;
	half3 B3 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[5].xy).rgb;
	half3 B4 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[5].zw).rgb;
	half3 B5 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[6].xy).rgb;
	half3 B6 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[6].zw).rgb;
	half3 B7 = BloomUpSourceBTexture.Sample(BloomUpSourceBSampler, input.uv[7].xy).rgb;

	half3 WA = BloomTintA.rgb;

	half3 WB = BloomTintB.rgb;

	color.rgb =
		A0 * WA +
		A1 * WA +
		A2 * WA +
		A3 * WA +
		A4 * WA +
		A5 * WA +
		A6 * WA +
		A7 * WA +
		B0 * WB +
		B1 * WB +
		B2 * WB +
		B3 * WB +
		B4 * WB +
		B5 * WB +
		B6 * WB +
		B7 * WB;
	color.a = 0;

    return color;
}