//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-09 18:38:14
//

module Shadow;

static public const int SHADOW_CASCADE_COUNT = 4;
static public const int SHADOW_MAP_SIZE = 2048;

public float PCFCascade(
    Texture2D<float> ShadowMap,
    SamplerState regularSampler,
    float4 WorldSpacePosition,
    float3 LightDirection,
    float4x4 LightView,
    float4x4 LightProj,
    float bias,
    int kernelSize) // kernelSize can now be used as scale factor
{
    // Transform world-space position into light space
    float4 lightSpacePosition = mul(LightView, WorldSpacePosition);
    float4 ndcPosition = mul(LightProj, lightSpacePosition);
    ndcPosition.xyz /= ndcPosition.w;

    // Compute shadow map UV coordinates
    float2 shadowUV = ndcPosition.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    // Check if outside the light frustum
    if (ndcPosition.z > 1.0)
        return 1.0;

    // Compute texel size for PCF kernel sampling
    float2 texelSize = 1.0 / SHADOW_MAP_SIZE;

    // Poisson disk PCF sampling
    float sampledDepth = ShadowMap.SampleLevel(regularSampler, shadowUV, 0.0);
    return ndcPosition.z - bias > sampledDepth ? 0.0 : 1.;
}

float4 GetCascadeColor(int layer)
{
    switch (layer) {
        case 0:
            return float4(1.0f, 0.0f, 0.0f, 1.0f);
        case 1:
            return float4(0.0f, 1.0f, 0.0f, 1.0f);
        case 2:
            return float4(0.0f, 0.0f, 1.0f, 1.0f);
        case 3:
            return float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    return 0.0f;
}
