//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 22:24:50
//

module Math;

static public const float PI = 3.14159265359;

public float4 GetWorldFromDepth(float2 uv, float depth, float4x4 invViewProj)
{
    // Convert UV to NDC
    float2 ndc = uv * 2.0 - 1.0;
    float4 clipPos = float4(ndc, depth, 1.0);
    clipPos.y *= -1.0; // Flip Y if needed (API-dependent)

    // Unproject from clip space to world space
    float4 worldPos = mul(invViewProj, clipPos);
    worldPos /= worldPos.w;

    return worldPos;
}
