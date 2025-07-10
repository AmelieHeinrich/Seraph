//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 22:35:41
//

static const float CAMERA_NEAR = 0.1f;
static const float CAMERA_FAR = 150.0f;

struct Camera
{
    column_major float4x4 View;
    column_major float4x4 Proj;
    column_major float4x4 ViewProj;
    column_major float4x4 InvView;
    column_major float4x4 InvProj;
    column_major float4x4 InvViewProj;
    float4 Position;
};
