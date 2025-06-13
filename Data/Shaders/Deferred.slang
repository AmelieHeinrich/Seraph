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
    uint2 Pad; //
};
DEFINE_CBV_ARRAY(Camera);
DEFINE_SRV_ARRAY(PointLight);
PUSH_CONSTANTS(PushConstants, Push);

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
    Camera camera = BindlessCBV_Camera.Load(Push.CameraDataSRV);

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
    float3 finalColor = 0;
    for (int i = 0; i < Push.PointLightCount; i++) {
        PointLight light = pointLights[i];

        // Light vector and distance
        float3 lightVector = light.Position - worldPosition.xyz;
        float lightDistance = length(lightVector);
        float3 L = lightVector / lightDistance;
        float attenuation = 1.0 / (lightDistance * lightDistance / (light.Radius * light.Radius));

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
        finalColor += (diffuse + specular) * NdotL * (attenuation * light.Color);
    }

    output[tid.xy] = float4(finalColor, 1.0);
}
