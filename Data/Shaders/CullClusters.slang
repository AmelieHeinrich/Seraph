//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 18:12:10
//

#include "Common/Bindless.slang"

import Common.Camera;
import Common.Lights;
import Common.Math;

struct FrustumPlane
{
    float3 Normal;
    float Distance;
};

struct PushConstants
{
    uint LightIndex;
    uint CameraIndex;
    uint ClusterArray;
    uint BinsArray;

    uint ClusterWidth;
    uint ClusterHeight;
    uint NumClustersX;
    uint NumClustersY;

    uint Width;
    uint Height;
    uint PointLightCount;
    uint SpotLightCount;

    uint SpotLightArray;
    uint NumClustersZ;
    uint2 Pad;
};

DEFINE_UAV_ARRAY(uint);
DEFINE_UAV_ARRAY(ClusterData);
DEFINE_SRV_ARRAY(PointLight);
DEFINE_SRV_ARRAY(SpotLight);
DEFINE_CBV_ARRAY(Camera);
PUSH_CONSTANTS(PushConstants, Push);

float2 PixelToNDC(int x, int y)
{
    float ndcX = (2.0f * x) / Push.Width - 1.0f;
    float ndcY = 1.0f - (2.0f * y) / Push.Height;
    return float2(ndcX, ndcY);
}

float4 NDCToView(float2 ndc, float z, column_major float4x4 invProj)
{
    float4 clip = float4(ndc, z, 1.0f);
    float4 view = mul(invProj, clip);
    return view / view.w;
}

FrustumPlane MakePlane(float3 a, float3 b, float3 c)
{
    FrustumPlane output = {};
    output.Normal = normalize(cross(b - a, c - a));
    output.Distance = -dot(output.Normal, a);
    return output;
}

bool SphereFrustumTest(float3 center, float radius, FrustumPlane plane)
{
    float distance = dot(plane.Normal, center) + plane.Distance;
    if (distance < -radius) return false;
    return true;
}

float4 ConeBoundingSphere(float3 origin, float3 forward, float size, float angle)
{
    float4 boundingSphere;
    if (angle > PI / 4.0f) {
        boundingSphere.xyz = origin + cos(angle) * size * forward;
        boundingSphere.w = sin(angle) * size;
    } else {
        boundingSphere.xyz = origin + size / (2.0f * cos(angle)) * forward;
        boundingSphere.w = size / (2.0f * cos(angle));
    }
    return boundingSphere;
}

bool TestConeVsSphere(float3 origin, float3 forward, float size, float angle, float4 testSphere)
{
    const float3 V = testSphere.xyz - origin;
    const float VlenSq = dot(V, V);
    const float V1len = dot(V, forward);
    const float distanceClosestPoint = cos(angle) * sqrt(VlenSq - V1len * V1len) - V1len * sin(angle);

    const bool angleCull = distanceClosestPoint > testSphere.w;
    const bool frontCull = V1len > testSphere.w + size;
    const bool backCull = V1len < -testSphere.w;
    return !(angleCull || frontCull || backCull);
}

groupshared uint gSharedBin[MAX_LIGHTS_PER_CLUSTER];
groupshared uint gSharedCount;

[numthreads(256, 1, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID, uint3 gtid: SV_GroupThreadID, uint3 gid: SV_GroupID)
{
    if (gid.x >= Push.NumClustersX || gid.y >= Push.NumClustersY || gid.z >= Push.NumClustersZ)
        return;

    // Map 3D cluster ID to 1D index
    uint cx = gid.x;
    uint cy = gid.y;
    uint cz = gid.z;
    uint clusterIndex = cz * Push.NumClustersX * Push.NumClustersY + cy * Push.NumClustersX + cx;

    // Reset shared count
    if (gtid.x == 0)
        gSharedCount = 0;
    GroupMemoryBarrierWithGroupSync();

    // Load camera/light data
    Camera camera = BindlessCBV_Camera.Load(Push.CameraIndex);
    StructuredBuffer<PointLight> pointLights = BindlessSRV_PointLight.Load(Push.LightIndex);
    StructuredBuffer<SpotLight> spotLights = BindlessSRV_SpotLight.Load(Push.SpotLightArray);
    RWStructuredBuffer<ClusterData> clusters = BindlessUAV_ClusterData.Load(Push.ClusterArray);
    RWStructuredBuffer<uint> bins = BindlessUAV_uint.Load(Push.BinsArray);

    clusters[clusterIndex].Offset = clusterIndex * MAX_LIGHTS_PER_CLUSTER;

    // Compute cluster screen-space bounds
    int x0 = cx * Push.ClusterWidth;
    int y0 = cy * Push.ClusterHeight;
    int x1 = min(x0 + Push.ClusterWidth, Push.Width);
    int y1 = min(y0 + Push.ClusterHeight, Push.Height);

    float logZScale = log(CAMERA_FAR / CAMERA_NEAR) / Push.NumClustersZ;
    float clusterNearZ = CAMERA_NEAR * exp(logZScale * float(cz));
    float clusterFarZ = CAMERA_NEAR * exp(logZScale * float(cz + 1));

    // Convert to NDC
    float4 projNear = mul(camera.Proj, float4(0, 0, -clusterNearZ, 1));
    float ndcNearZ = projNear.z / projNear.w;
    float4 projFar = mul(camera.Proj, float4(0, 0, -clusterFarZ, 1));
    float ndcFarZ = projFar.z / projFar.w;

    float minZ = ndcNearZ;
    float maxZ = ndcFarZ;

    // Reconstruct cluster frustum corners (8 corners total)
    float2 ndcTL = PixelToNDC(x0, y0);
    float2 ndcTR = PixelToNDC(x1, y0);
    float2 ndcBR = PixelToNDC(x1, y1);
    float2 ndcBL = PixelToNDC(x0, y1);

    float3 v0 = NDCToView(ndcTL, minZ, camera.InvProj).rgb;
    float3 v1 = NDCToView(ndcTR, minZ, camera.InvProj).rgb;
    float3 v2 = NDCToView(ndcBR, minZ, camera.InvProj).rgb;
    float3 v3 = NDCToView(ndcBL, minZ, camera.InvProj).rgb;
    float3 v4 = NDCToView(ndcTL, maxZ, camera.InvProj).rgb;
    float3 v5 = NDCToView(ndcTR, maxZ, camera.InvProj).rgb;
    float3 v6 = NDCToView(ndcBR, maxZ, camera.InvProj).rgb;
    float3 v7 = NDCToView(ndcBL, maxZ, camera.InvProj).rgb;

    // Construct cluster frustum planes
    FrustumPlane left = MakePlane(v0, v3, v7);
    FrustumPlane right = MakePlane(v2, v1, v5);
    FrustumPlane top = MakePlane(v1, v0, v4);
    FrustumPlane bottom = MakePlane(v3, v2, v6);
    FrustumPlane near = MakePlane(v0, v1, v2);
    FrustumPlane far = MakePlane(v5, v4, v7);

    // Bounding sphere for cluster (for cone light culling)
    float3 center = (v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7) / 8.0f;
    float radius = 0.0f;
    float3 corners[8] = { v0, v1, v2, v3, v4, v5, v6, v7 };
    for (int i = 0; i < 8; ++i)
        radius = max(radius, length(corners[i] - center));
    float4 clusterBoundingSphere = float4(center, radius);

    // Cull point lights
    uint threadCount = 256;
    for (uint i = gtid.x; i < Push.PointLightCount; i += threadCount)
    {
        PointLight light = pointLights[i];
        float3 viewPos = mul(camera.View, float4(light.Position, 1.0)).xyz;

        bool visible = SphereFrustumTest(viewPos, light.Radius, left)
                    && SphereFrustumTest(viewPos, light.Radius, right)
                    && SphereFrustumTest(viewPos, light.Radius, top)
                    && SphereFrustumTest(viewPos, light.Radius, bottom)
                    && SphereFrustumTest(viewPos, light.Radius, near)
                    && SphereFrustumTest(viewPos, light.Radius, far);

        if (visible) {
            uint idx;
            InterlockedAdd(gSharedCount, 1, idx);
            if (idx < MAX_LIGHTS_PER_CLUSTER)
                gSharedBin[idx] = i;
        }
    }

    // Cull spot lights
    for (uint i = gtid.x; i < Push.SpotLightCount; i += threadCount)
    {
        SpotLight light = spotLights[i];
        float3 viewOrigin = mul(camera.View, float4(light.Position, 1.0)).xyz;
        float3 viewForward = normalize(mul((float3x3)camera.View, light.Forward));
        float4 coneSphere = ConeBoundingSphere(viewOrigin, viewForward, light.Size, light.Angle);

        bool visible = SphereFrustumTest(coneSphere.xyz, coneSphere.w, left)
                    && SphereFrustumTest(coneSphere.xyz, coneSphere.w, right)
                    && SphereFrustumTest(coneSphere.xyz, coneSphere.w, top)
                    && SphereFrustumTest(coneSphere.xyz, coneSphere.w, bottom)
                    && SphereFrustumTest(coneSphere.xyz, coneSphere.w, near)
                    && SphereFrustumTest(coneSphere.xyz, coneSphere.w, far);

        if (visible) {
            uint idx;
            InterlockedAdd(gSharedCount, 1, idx);
            if (idx < MAX_LIGHTS_PER_CLUSTER)
                gSharedBin[idx] = i | (1u << 31); // use MSB to tag as spotlight
        }
    }

    GroupMemoryBarrierWithGroupSync();

    uint lightCount = min(gSharedCount, MAX_LIGHTS_PER_CLUSTER);
    for (uint i = gtid.x; i < lightCount; i += 128)
        bins[clusters[clusterIndex].Offset + i] = gSharedBin[i];

    if (gtid.x == 0)
        clusters[clusterIndex].Count = lightCount;
}
