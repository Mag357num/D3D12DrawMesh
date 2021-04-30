Texture2D SceneColorTexture : register(t0);
SamplerState SceneColorSampler : register(s0);

cbuffer BloomSetupConstantBuffer : register(b0)
{
    float4 BufferSizeAndInvSize;
    float  BloomThreshold;
};

half  Luminance(  half3  LinearColor )
{
	return dot( LinearColor,  half3 ( 0.3, 0.59, 0.11 ) );
}

struct VSInput
{
    float3 position    : POSITION;
    float2 uv        : TEXCOORD0;
};

struct PSInput
{
    float4 position    : SV_POSITION;
    float2 uv[4]       : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.position = float4(input.position, 1.0f);
    result.uv[0] = input.uv + BufferSizeAndInvSize.zw * float2(-1, -1);
    result.uv[1] = input.uv + BufferSizeAndInvSize.zw * float2( 1, -1);
    result.uv[2] = input.uv + BufferSizeAndInvSize.zw * float2(-1,  1);
    result.uv[3] = input.uv + BufferSizeAndInvSize.zw * float2( 1,  1);

    return result;
}

half4 PSMain(PSInput input) : SV_TARGET
{
    half4 C0 = SceneColorTexture.Sample(SceneColorSampler, input.uv[0]);
	half4 C1 = SceneColorTexture.Sample(SceneColorSampler, input.uv[1]);
	half4 C2 = SceneColorTexture.Sample(SceneColorSampler, input.uv[2]);
	half4 C3 = SceneColorTexture.Sample(SceneColorSampler, input.uv[3]);

	C0.rgb = C0;
	C1.rgb = C1;
	C2.rgb = C2;
	C3.rgb = C3;

    half3 AverageColor = 0.0f;
	AverageColor.rgb = (C0.rgb * 0.25) + (C1.rgb * 0.25) + (C2.rgb * 0.25) + (C3.rgb * 0.25);

    half TotalLuminance = Luminance(AverageColor.rgb);
    half BloomLuminance = TotalLuminance - BloomThreshold;
    half Amount = saturate(BloomLuminance * 0.5f);

    half4 color;
    color.rgb = AverageColor * Amount;
    color.a = 0.0f;
    return color;
}