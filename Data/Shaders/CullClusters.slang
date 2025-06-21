//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 18:12:10
//

#include "Common/Bindless.slang"

#define LIGHTS_PER_THREAD 128

import Common.Camera;
import Common.Lights;
import Common.Math;

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

float4 MakePlane(float3 a, float3 b, float3 c)
{
    float3 normal = normalize(cross(b - a, c - a));
    return float4(normal, -dot(normal, a));
}

bool SphereFrustumTest(float3 center, float radius, float4 plane)
{
    float distance = dot(plane.xyz, center) + plane.w;
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

bool SphereAABBIntersect(float3 center, float radius, float3 aabbMin, float3 aabbMax)
{
    float3 clamped = clamp(center, aabbMin, aabbMax);
    float distSq = dot(center - clamped, center - clamped);
    return distSq <= radius * radius;
}

groupshared uint gClusterOffset;
groupshared float4 gAABBMin;
groupshared float4 gAABBMax;
groupshared Camera gCamera;
groupshared ClusterData gCluster;

[shader("compute")]
[numthreads(LIGHTS_PER_THREAD, 1, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID, uint3 gtid: SV_GroupThreadID, uint3 gid: SV_GroupID)
{
    if (gid.x >= Push.NumClustersX || gid.y >= Push.NumClustersY || gid.z >= Push.NumClustersZ)
        return;

    uint cx = gid.x;
    uint cy = gid.y;
    uint cz = gid.z;
    uint clusterIndex = cz * Push.NumClustersX * Push.NumClustersY + cy * Push.NumClustersX + cx;

    StructuredBuffer<PointLight> pointLights = BindlessSRV_PointLight.Load(Push.LightIndex);
    StructuredBuffer<SpotLight> spotLights = BindlessSRV_SpotLight.Load(Push.SpotLightArray);
    RWStructuredBuffer<ClusterData> clusters = BindlessUAV_ClusterData.Load(Push.ClusterArray);
    RWStructuredBuffer<uint> bins = BindlessUAV_uint.Load(Push.BinsArray);

    // Initialize planes
    if (gtid.x == 0) {
        gCamera = BindlessCBV_Camera.Load(Push.CameraIndex);
        gClusterOffset = clusterIndex * MAX_LIGHTS_PER_CLUSTER;
        gCluster.Count = 0;
        gCluster.Offset = gClusterOffset;

        int x0 = cx * Push.ClusterWidth;
        int y0 = cy * Push.ClusterHeight;
        int x1 = min(x0 + Push.ClusterWidth, Push.Width);
        int y1 = min(y0 + Push.ClusterHeight, Push.Height);

        float logZScale = log(CAMERA_FAR / CAMERA_NEAR) / Push.NumClustersZ;
        float clusterNearZ = CAMERA_NEAR * exp(logZScale * float(cz));
        float clusterFarZ = CAMERA_NEAR * exp(logZScale * float(cz + 1));

        float4 projNear = mul(gCamera.Proj, float4(0, 0, -clusterNearZ, 1));
        float ndcNearZ = projNear.z / projNear.w;
        float4 projFar = mul(gCamera.Proj, float4(0, 0, -clusterFarZ, 1));
        float ndcFarZ = projFar.z / projFar.w;

        float minZ = ndcNearZ;
        float maxZ = ndcFarZ;

        float2 ndcTL = PixelToNDC(x0, y0);
        float2 ndcTR = PixelToNDC(x1, y0);
        float2 ndcBR = PixelToNDC(x1, y1);
        float2 ndcBL = PixelToNDC(x0, y1);

        float4 v0 = NDCToView(ndcTL, minZ, gCamera.InvProj);
        float4 v1 = NDCToView(ndcTR, minZ, gCamera.InvProj);
        float4 v2 = NDCToView(ndcBR, minZ, gCamera.InvProj);
        float4 v3 = NDCToView(ndcBL, minZ, gCamera.InvProj);
        float4 v4 = NDCToView(ndcTL, maxZ, gCamera.InvProj);
        float4 v5 = NDCToView(ndcTR, maxZ, gCamera.InvProj);
        float4 v6 = NDCToView(ndcBR, maxZ, gCamera.InvProj);
        float4 v7 = NDCToView(ndcBL, maxZ, gCamera.InvProj);

        float3 minCorner = min(min(min(v0.xyz, v1.xyz), min(v2.xyz, v3.xyz)),
                               min(min(v4.xyz, v5.xyz), min(v6.xyz, v7.xyz)));
        float3 maxCorner = max(max(max(v0.xyz, v1.xyz), max(v2.xyz, v3.xyz)),
                               max(max(v4.xyz, v5.xyz), max(v6.xyz, v7.xyz)));

        gAABBMin = float4(minCorner, 1.0);
        gAABBMax = float4(maxCorner, 1.0);
    }

    GroupMemoryBarrierWithGroupSync();

    // Cull point lights
    for (uint i = gtid.x; i < Push.PointLightCount; i += LIGHTS_PER_THREAD) {
        PointLight light = pointLights[i];
        float4 viewPos = mul(gCamera.View, float4(light.Position, 1.0));

        if (SphereAABBIntersect(viewPos.xyz, light.Radius, gAABBMin.xyz, gAABBMax.xyz)) {
            uint idx;
            InterlockedAdd(gCluster.Count, 1, idx);
            if (idx < MAX_LIGHTS_PER_CLUSTER) {
                bins[gClusterOffset + idx] = i;
            }
        }
    }

    // Cull spot lights
    for (uint i = gtid.x; i < Push.SpotLightCount; i += LIGHTS_PER_THREAD) {
        SpotLight light = spotLights[i];
        float4 viewOrigin = mul(gCamera.View, float4(light.Position, 1.0));
        float3 viewForward = normalize(mul((float3x3)gCamera.View, light.Forward));
        float4 coneSphere = ConeBoundingSphere(viewOrigin.xyz, viewForward, light.Size, light.Angle);

        if (SphereAABBIntersect(coneSphere.xyz, coneSphere.w, gAABBMin.xyz, gAABBMax.xyz)) {
            uint idx;
            InterlockedAdd(gCluster.Count, 1, idx);
            if (idx < MAX_LIGHTS_PER_CLUSTER) {
                bins[gClusterOffset + idx] = i | (1u << 31);
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (gtid.x == 0) {
        clusters[clusterIndex].Count = gCluster.Count;
        clusters[clusterIndex].Offset = gClusterOffset;
    }
}
