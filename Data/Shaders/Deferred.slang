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
    uint TileHeight; //

    uint NumTilesX;
    uint BinsArray;
    uint TilesArray;
    uint ShowTileHeatmap; //

    uint SpotLightArray;
    uint SpotLightCount;
    uint2 Pad; //
};

DEFINE_SRV_ARRAY(uint);
DEFINE_SRV_ARRAY(TileData);
DEFINE_SRV_ARRAY(PointLight);
DEFINE_SRV_ARRAY(SpotLight);
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

float3 EvaluatePointLight(PointLight light, float3 V, float4 worldPosition, float3 N, float3 materialColor, float metallic, float roughness)
{
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

    // Apply attenuation to final contribution
    float3 distributedEnergy = ((diffuse + specular) * NdotL);
    float3 incomingRadiance = attenuation * light.Color * light.Intensity;
    return (distributedEnergy * incomingRadiance * (1.0 - Push.ShowTileHeatmap));
}

float3 EvaluateSpotLight(SpotLight light, float3 V, float4 worldPosition, float3 N, float3 materialColor, float metallic, float roughness)
{
    float3 lightVector = light.Position - worldPosition.xyz;
    float distance = length(lightVector);
    float3 L = lightVector / distance;

    // Angle attenuation (spotlight cone)
    float3 toFragment = -L;
    float spotFactor = dot(toFragment, normalize(light.Forward));

    float cosAngle = cos(light.Angle);

    // Smooth fade at edge of cone
    float falloff = saturate((spotFactor - cosAngle) / (1.0 - cosAngle));

    if (falloff <= 0.0001)
        return float3(0, 0, 0); // Outside the cone

    // Distance attenuation
    float attenuation = 1.0 / (distance * distance + 1.0);
    attenuation *= saturate(1.0 - distance / light.Size);

    // Combine angle falloff and distance attenuation
    attenuation *= falloff;

    // Cook-Torrance lighting (same as point light)
    float3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0001);
    float NdotV = max(dot(N, V), 0.0001);
    float HdotV = max(dot(H, V), 0.0001);

    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), materialColor, metallic);
    float3 F = FresnelSchlick(HdotV, F0);

    float3 numerator = D * G * F;
    float denominator = 4.0 * NdotL * NdotV + 0.0001;
    float3 specular = numerator / denominator;

    float3 kD = (1.0 - F) * (1.0 - metallic);
    float3 diffuse = kD * materialColor / PI;

    float3 distributedEnergy = (diffuse + specular) * NdotL;
    float3 incomingRadiance = attenuation * light.Color * light.Intensity;
    return (distributedEnergy * incomingRadiance * (1.0 - Push.ShowTileHeatmap));
}

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    // Resources
    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.Depth);
    Texture2D<float4> normal = BindlessTexture2DFloat4.Load(Push.Normal);
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    Camera camera = BindlessCBV_Camera.Load(Push.CameraDataSRV);

    // Load depth & reconstruct world position
    float viewDepth = depth.Load(tid);
    float2 uv = TexelToUV(tid.xy, float2(Push.Width, Push.Height));
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);

    float3 N = normalize(normal.Load(tid).rgb);

    // Extract height and offset so everything is positive
    float heightOffset = 30.0;
    float height = worldPosition.y + heightOffset;

    // Height thresholds
    float lowHeight = 0.0;    // green start
    float midHeight = 13.3f;  // dirt start
    float highHeight = 25.6f; // rock start
    float snowHeight = 40.0f; // snow start

    // Terrain colors
    float3 green = float3(0.1, 0.6, 0.1); // grass
    float3 brown = float3(0.4, 0.3, 0.1); // dirt
    float3 gray = float3(0.5, 0.5, 0.5);  // rock
    float3 white = float3(1.0, 1.0, 1.0); // snow

    float3 baseColor;

    // Multi-layered color blending
    if (height < midHeight) {
        float t = saturate((height - lowHeight) / (midHeight - lowHeight));
        baseColor = lerp(green, brown, t);
    } else if (height < highHeight) {
        float t = saturate((height - midHeight) / (highHeight - midHeight));
        baseColor = lerp(brown, gray, t);
    } else if (height < snowHeight) {
        float t = saturate((height - highHeight) / (snowHeight - highHeight));
        baseColor = lerp(gray, white, t);
    } else {
        baseColor = white;
    }

    // Lighting
    float3 lightDir = normalize(float3(0.3, 1.0, 0.2)); // from above at an angle
    float NdotL = max(dot(N, lightDir), 0.0);
    float ambient = 0.1;
    float3 finalColor = (NdotL * 0.9) * baseColor;

    output[tid.xy] = float4(finalColor, 1.0);
}
