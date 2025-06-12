//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-12 22:58:56
//

#include "Common/Bindless.slang"

struct MeshOutput
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MSMain(OutputVertices<MeshOutput, 3> vertices,
            OutputIndices<uint3, 1> triangles)
{
    SetMeshOutputCounts(3, 1);

    triangles[0] = uint3(0, 1, 2);

    vertices[0].Position = float4(-0.5, -0.5, 0.0, 1.0);
    vertices[0].Color = float3(1.0, 0.0, 0.0);

    vertices[1].Position = float4(0.5, -0.5, 0.0, 1.0);
    vertices[1].Color = float3(0.0, 1.0, 0.0);

    vertices[2].Position = float4(0.0, 0.5, 0.0, 1.0);
    vertices[2].Color = float3(0.0, 0.0, 1.0);
}

[shader("fragment")]
float4 FSMain(MeshOutput input) : SV_Target
{
    return float4(input.Color, 1.0);
}
