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
    uint ClusterWidth;
    uint ClusterHeight; //

    uint NumClustersX;
    uint BinsArray;
    uint ClusterArray;
    uint ShowClusterHeatmap; //

    uint SpotLightArray;
    uint SpotLightCount;
    uint NumClustersZ;
    uint NumClustersY;
};

DEFINE_SRV_ARRAY(uint);
DEFINE_SRV_ARRAY(ClusterData);
DEFINE_SRV_ARRAY(PointLight);
DEFINE_SRV_ARRAY(SpotLight);
DEFINE_CBV_ARRAY(Camera);
PUSH_CONSTANTS(PushConstants, Push);

float3 GetClusterColor(uint tileLightCount)
{
    float t = MAX_LIGHTS_PER_CLUSTER > 0 ? clamp(float(tileLightCount) / float(MAX_LIGHTS_PER_CLUSTER), 0.0f, 1.0f) : 0.0f;

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
    return (distributedEnergy * incomingRadiance * (1.0 - Push.ShowClusterHeatmap));
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
    return (distributedEnergy * incomingRadiance * (1.0 - Push.ShowClusterHeatmap));
}

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height)
        return;

    // Resources
    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.Depth);
    Texture2D<float4> normal = BindlessTexture2DFloat4.Load(Push.Normal);
    Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(Push.Albedo);
    Texture2D<float2> pbr = BindlessTexture2DFloat2.Load(Push.PBR);
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    StructuredBuffer<PointLight> pointLights = BindlessSRV_PointLight.Load(Push.PointLightArray);
    StructuredBuffer<SpotLight> spotLights = BindlessSRV_SpotLight.Load(Push.SpotLightArray);
    StructuredBuffer<ClusterData> clusters = BindlessSRV_ClusterData.Load(Push.ClusterArray);
    StructuredBuffer<uint> bins = BindlessSRV_uint.Load(Push.BinsArray);
    Camera camera = BindlessCBV_Camera.Load(Push.CameraDataSRV);

    // Load depth and compute view-space position
    float viewDepth = depth.Load(tid);
    float2 uv = TexelToUV(tid.xy, float2(Push.Width, Push.Height));
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);
    float3 viewPos = mul(camera.View, worldPosition).xyz;

    // Determine cluster indices from screen coordinates and view depth
    uint clusterX = tid.x / Push.ClusterWidth;
    uint clusterY = tid.y / Push.ClusterHeight;

    // Logarithmic depth slicing
    float invLogZScale = 1.0f / (log(CAMERA_FAR / CAMERA_NEAR) / Push.NumClustersZ);
    float z = -viewPos.z;
    uint clusterZ = (uint)clamp(log(z / CAMERA_NEAR) * invLogZScale, 0.0f, float(Push.NumClustersZ - 1));

    uint clusterIndex = clusterZ * Push.NumClustersX * Push.NumClustersY + clusterY * Push.NumClustersX + clusterX;
    ClusterData cluster = clusters[clusterIndex];

    // Load material info
    float3 cameraPosition = camera.Position.xyz;
    float3 materialColor = albedo.Load(tid).rgb;
    float3 N = normalize(normal.Load(tid).rgb);
    float metallic = pbr.Load(tid).r;
    float roughness = pbr.Load(tid).g;

    // Lighting computation
    float3 V = normalize(cameraPosition - worldPosition.xyz);
    float3 finalColor = GetClusterColor(cluster.Count) * Push.ShowClusterHeatmap; // Optional heatmap
    for (int i = 0; i < cluster.Count; ++i) {
        uint encoded = bins[cluster.Offset + i];
        bool isSpot = (encoded & (1u << 31)) != 0;
        uint lightIndex = encoded & 0x7FFFFFFF;

        float3 contrib = float3(0, 0, 0);
        if (isSpot) {
            contrib = EvaluateSpotLight(spotLights[lightIndex], V, worldPosition, N, materialColor, metallic, roughness);
        } else {
            contrib = EvaluatePointLight(pointLights[lightIndex], V, worldPosition, N, materialColor, metallic, roughness);
        }
        finalColor += contrib;
    }

    output[tid.xy] = float4(finalColor, 1.0);
}
