Texture2D BloomDownSourceTexture : register(t0);
SamplerState BloomDownSourceSampler : register(s0);

cbuffer BloomDownConstantBuffer : register(b0)
{
    float4 BufferSizeAndInvSize;
    float BloomDownScale;
};

struct VSInput
{
    float3 position    : POSITION;
    float2 uv        : TEXCOORD0;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float4 uv[8]       : TEXCOORD0;
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

    float Start = 2.0/14.0;
	float Scale = BloomDownScale;

	result.uv[0].xy = input.uv.xy;
	result.uv[0].zw = input.uv.xy + Circle(Start, 14.0, 0.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[1].xy = input.uv.xy + Circle(Start, 14.0, 1.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[1].zw = input.uv.xy + Circle(Start, 14.0, 2.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[2].xy = input.uv.xy + Circle(Start, 14.0, 3.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[2].zw = input.uv.xy + Circle(Start, 14.0, 4.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[3].xy = input.uv.xy + Circle(Start, 14.0, 5.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[3].zw = input.uv.xy + Circle(Start, 14.0, 6.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[4].xy = input.uv.xy + Circle(Start, 14.0, 7.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[4].zw = input.uv.xy + Circle(Start, 14.0, 8.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[5].xy = input.uv.xy + Circle(Start, 14.0, 9.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[5].zw = input.uv.xy + Circle(Start, 14.0, 10.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[6].xy = input.uv.xy + Circle(Start, 14.0, 11.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[6].zw = input.uv.xy + Circle(Start, 14.0, 12.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[7].xy = input.uv.xy + Circle(Start, 14.0, 13.0) * Scale * BufferSizeAndInvSize.zw;
	result.uv[7].zw = float2(0.0, 0.0);

    return result;
}

half4 PSMain(PSInput input) : SV_TARGET
{
    half4 color;
    half4 N0 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[0].xy).rgba;
	half4 N1 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[0].zw).rgba;
	half4 N2 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[1].xy).rgba;
	half4 N3 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[1].zw).rgba;
	half4 N4 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[2].xy).rgba;
	half4 N5 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[2].zw).rgba;
	half4 N6 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[3].xy).rgba;
	half4 N7 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[3].zw).rgba;
	half4 N8 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[4].xy).rgba;
	half4 N9 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[4].zw).rgba;
	half4 N10 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[5].xy).rgba;
	half4 N11 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[5].zw).rgba;
	half4 N12 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[6].xy).rgba;
	half4 N13 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[6].zw).rgba;
	half4 N14 = BloomDownSourceTexture.Sample(BloomDownSourceSampler, input.uv[7].xy).rgba;
	float W = 1.0/15.0;
	color.rgb =
		(N0 * W) +
		(N1 * W) +
		(N2 * W) +
		(N3 * W) +
		(N4 * W) +
		(N5 * W) +
		(N6 * W) +
		(N7 * W) +
		(N8 * W) +
		(N9 * W) +
		(N10 * W) +
		(N11 * W) +
		(N12 * W) +
		(N13 * W) +
		(N14 * W);
	color.a = 0;
    
    return color;
}