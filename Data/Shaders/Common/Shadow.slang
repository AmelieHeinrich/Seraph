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
    int kernelSize)
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

    // Perform PCF with manual depth comparison
    float shadow = 0.0;
    int sampleCount = 0;

    for (int x = -kernelSize; x <= kernelSize; x++) {
        for (int y = -kernelSize; y <= kernelSize; y++) {
            float2 offsetUV = clamp(shadowUV + (float2(x, y) * texelSize), 0.0, 1.0);

            // Sample the depth from the shadow map
            float sampledDepth = ShadowMap.SampleLevel(regularSampler, offsetUV, 0.0);

            // Manually compare depth with bias
            shadow += ndcPosition.z - bias > sampledDepth ? 0.0 : 1.0;
            sampleCount++;
        }
    }

    shadow /= sampleCount;
    return shadow;
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
