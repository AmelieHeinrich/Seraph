//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 13:20:00
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelGPU/KGPU_ShaderModule.h>

namespace KDS
{
	struct IncludeRecord
	{
		KC::String Path = "";
		int Line = 0;
	};

	struct EntryPointRecord
	{
		KGPU::ShaderStage Stage;
		KC::String Name = "";
	};

	struct ParseResult
	{
		KC::Array<EntryPointRecord> EntryPoints;
		KC::Array<IncludeRecord> Includes;
	};

	class Parser
	{
	public:
		static ParseResult ParseShaderSource(const KC::String& source);
	};
}
