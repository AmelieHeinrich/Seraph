//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:18:07
//

#include "Common/Bindless.slang"

import Common.Lights;
import Common.Math;
import Common.PBR;
import Common.Camera;
import Common.Compute;

struct PushConstants
{
    uint Depth;
    uint Normal;
    uint Albedo;
    uint PBR; //
    
    uint Output;
    uint Width;
    uint Height;
    uint PointLightArray; //
    
    uint PointLightCount;
    uint CameraDataSRV;
    uint TileWidth;
    uint TileHeight;

    uint NumTilesX;
    uint BinsArray;
    uint TilesArray;
    uint Pad;
};

DEFINE_SRV_ARRAY(uint);
DEFINE_SRV_ARRAY(TileData);
DEFINE_SRV_ARRAY(PointLight);
DEFINE_CBV_ARRAY(Camera);
PUSH_CONSTANTS(PushConstants, Push);

float3 GetTileColor(uint tileLightCount)
{
    float t = MAX_LIGHTS_PER_TILE > 0 ? clamp(float(tileLightCount) / float(MAX_LIGHTS_PER_TILE), 0.0f, 1.0f) : 0.0f;

    // Blue → Green → Yellow → Red
    if (t < 0.33f) {
        // Blue to Green
        float localT = t / 0.33f;
        return lerp(float3(0, 0, 1), float3(0, 1, 0), localT);
    }
    else if (t < 0.66f) {
        // Green to Yellow
        float localT = (t - 0.33f) / 0.33f;
        return lerp(float3(0, 1, 0), float3(1, 1, 0), localT);
    }
    else {
        // Yellow to Red
        float localT = (t - 0.66f) / 0.34f;
        return lerp(float3(1, 1, 0), float3(1, 0, 0), localT);
    }
    return float3(0, 0, 1);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    // Resources
    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.Depth);
    Texture2D<float4> normal = BindlessTexture2DFloat4.Load(Push.Normal);
    Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(Push.Albedo);
    Texture2D<float2> pbr = BindlessTexture2DFloat2.Load(Push.PBR);
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    StructuredBuffer<PointLight> pointLights = BindlessSRV_PointLight.Load(Push.PointLightArray);
    StructuredBuffer<TileData> tiles = BindlessSRV_TileData.Load(Push.TilesArray);
    StructuredBuffer<uint> bins = BindlessSRV_uint.Load(Push.BinsArray);
    Camera camera = BindlessCBV_Camera.Load(Push.CameraDataSRV);

    // Get current tile
    uint tileX = tid.x / Push.TileWidth;
    uint tileY = tid.y / Push.TileHeight;
    uint tileIndex = tileY * Push.NumTilesX + tileX;
    TileData tile = tiles[tileIndex];

    // Load Basic data
    float viewDepth = depth.Load(tid);
    float2 uv = TexelToUV(tid.xy, float2(Push.Width, Push.Height));
    float3 cameraPosition = camera.Position.xyz;
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);
    float3 materialColor = albedo.Load(tid).rgb;
    float3 N = normal.Load(tid).rgb;
    float metallic = pbr.Load(tid).r;
    float roughness = pbr.Load(tid).g;

    // Math!
    float3 V = normalize(cameraPosition - worldPosition.xyz);
    float3 finalColor = materialColor * GetTileColor(tile.Count) * 0.05;
    for (int i = 0; i < tile.Count; i++) {
        PointLight light = pointLights[tile.Offset + i];

        // Light vector and distance
        float3 lightVector = light.Position - worldPosition.xyz;
        float distance = length(lightVector);
        float3 L = lightVector / distance;

        float attenuation = 1.0 / (distance * distance + 1.0);  // safe inverse-square
        attenuation *= saturate(1.0 - distance / light.Radius); // fade to 0 at max range

        // Cook-Torrance
        float3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0001);
        float NdotV = max(dot(N, V), 0.0001);
        float HdotV = max(dot(H, V), 0.0001);

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F0 = lerp(float3(0.04, 0.04, 0.04), materialColor, metallic);
        float3 F = FresnelSchlick(HdotV, F0);

        float3 numerator = D * G * F;
        float denominator = 4 * NdotL * NdotV + 0.0001;
        float3 specular = numerator / denominator;

        // Lambertian
        float3 kD = (1.0 - F) * (1.0 - metallic);
        float3 diffuse = kD * materialColor / PI;

        // Constant ambient just in case
        float3 ambient = 0.04;

        // Apply attenuation to final contribution
        float3 distributedEnergy = ((diffuse + specular) * NdotL);
        float3 incomingRadiance = attenuation * light.Color * light.Intensity;
        finalColor += distributedEnergy * incomingRadiance;
    }

    output[tid.xy] = float4(finalColor, 1.0);
}
