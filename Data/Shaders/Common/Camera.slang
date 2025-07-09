//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 22:35:41
//

module Camera;

static public const float CAMERA_NEAR = 0.1f;
static public const float CAMERA_FAR = 150.0f;

public struct Camera
{
    public column_major float4x4 View;
    public column_major float4x4 Proj;
    public column_major float4x4 ViewProj;
    public column_major float4x4 InvView;
    public column_major float4x4 InvProj;
    public column_major float4x4 InvViewProj;
    public float4 Position;
};
