//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:05:59
//

#include "ShaderCompiler.h"

#include <Core/FileSystem.h>
#include <Core/Types.h>
#include <sstream>

#ifdef SERAPH_WINDOWS
    #include <Windows.h>
    #include <dxc/dxcerrors.h>
    #include <dxc/dxcapi.h>

    #undef ReadFile // I hate Windows
#elif defined(SERAPH_MAC)
    #include <dxc/WinAdapter.h>
    #include <dxc/dxcapi.h>
    #include <dlfcn.h>

    typedef HRESULT (*PFN_DxcCreateInstance)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
    PFN_DxcCreateInstance DxcCreateInstance_Mac;
    void* DxcDYLIB;
#endif

ShaderCompiler::Data ShaderCompiler::sData;

// I clauded this, sorry not sorry  
class CustomIncludeHandler : public IDxcIncludeHandler
{
private:
    IDxcUtils* m_pUtils;
    String m_shaderDirectory;
    uint m_refCount;

public:
    CustomIncludeHandler(IDxcUtils* pUtils, const String& shaderDirectory)
        : m_pUtils(pUtils), m_shaderDirectory(shaderDirectory), m_refCount(1)
    {
        if (m_pUtils)
            m_pUtils->AddRef();
    }

    virtual ~CustomIncludeHandler()
    {
        if (m_pUtils)
            m_pUtils->Release();
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++m_refCount;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG refCount = --m_refCount;
        if (refCount == 0)
            delete this;
        return refCount;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == __uuidof(IDxcIncludeHandler) || riid == __uuidof(IUnknown))
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    // IDxcIncludeHandler method
    HRESULT STDMETHODCALLTYPE LoadSource(
        LPCWSTR pFilename,
        IDxcBlob** ppIncludeSource) override
    {
        if (!pFilename || !ppIncludeSource)
            return E_INVALIDARG;

        // Convert wide char filename to string
        char filename[512];
#ifdef SERAPH_WINDOWS
        wcstombs_s(nullptr, filename, 512, pFilename, _TRUNCATE);
#else
        std::wcstombs(filename, pFilename, 512);
#endif

        // Build full path - try relative to shader directory first
        String fullPath = m_shaderDirectory + "/" + String(filename);
        
        // If file doesn't exist in shader directory, try as absolute path
        if (!FileSystem::Exists(fullPath))
        {
            fullPath = String(filename);
            if (!FileSystem::Exists(fullPath))
            {
                return E_FAIL;
            }
        }

        // Read the file
        String source = FileSystem::ReadFile(fullPath);
        if (source.empty())
            return E_FAIL;

        // Create blob from source
        IDxcBlobEncoding* pSourceBlob = nullptr;
        HRESULT hr = m_pUtils->CreateBlob(source.c_str(), source.size(), 0, &pSourceBlob);
        if (FAILED(hr))
            return hr;

        *ppIncludeSource = pSourceBlob;
        return S_OK;
    }
};

const char* ProfileFromStage(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::kVertex:
            return "vs_6_6";
        case ShaderStage::kFragment:
            return "ps_6_6";
        case ShaderStage::kCompute:
            return "cs_6_6";
        case ShaderStage::kMesh:
            return "ms_6_6";
        default: return "cs_6_6";
    }
    return "cs6_6";
}

void ShaderCompiler::Initialize(RHIBackend backend)
{
    sData.Backend = backend;

#ifdef SERAPH_MAC
    DxcDYLIB = dlopen("libdxcompiler.dylib", RTLD_LAZY);
    if (!DxcDYLIB) {
        SERAPH_FATAL("Failed to load DXC dylib!");
        return;
    }

    DxcCreateInstance_Mac = (PFN_DxcCreateInstance)dlsym(DxcDYLIB, "DxcCreateInstance");
#endif

    SERAPH_INFO("Initialized shader compiler!");
}

void ShaderCompiler::Shutdown()
{
#ifdef SERAPH_MAC
    dlclose(DxcDYLIB);
#endif
}

CompiledShader ShaderCompiler::Compile(const String& path)
{
    String actualPath = "Data/Shaders/" + path;

    if (!FileSystem::Exists(actualPath))
        return {};

    CompiledShader result = {};

    String source = FileSystem::ReadFile(actualPath);
    Array<String> lines = FileSystem::ReadAllLines(actualPath);

    UnorderedMap<String, ShaderStage> entryPoints;
    for (String line : lines) {
        std::istringstream iss(line);
        String pragma, stage, entry;
        iss >> pragma >> stage >> entry;
        
        if (pragma == "#pragma") {
            if (stage == "vertex")
                entryPoints[entry] = ShaderStage::kVertex;
            else if (stage == "fragment")
                entryPoints[entry] = ShaderStage::kFragment;
            else if (stage == "kernel")
                entryPoints[entry] = ShaderStage::kCompute;
            else if (stage == "mesh")
                entryPoints[entry] = ShaderStage::kMesh;
        }
    }

    for (auto& [entry, type] : entryPoints) {
        // Compile that shyte
        const char* sourceCstr = source.c_str();

        wchar_t wideTarget[512];
        swprintf_s(wideTarget, 512, L"%hs", ProfileFromStage(type));
        
        wchar_t wideEntry[512];
        swprintf_s(wideEntry, 512, L"%hs", entry.c_str());

        IDxcUtils* pUtils = nullptr;
        IDxcCompiler* pCompiler = nullptr;
#ifdef SERAPH_WINDOWS
        ASSERT_EQ(SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils))), "Failed to create DXC utils!");
        ASSERT_EQ(SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler))), "Failed too create DXC compiler!");
#else
        ASSERT_EQ(SUCCEEDED(DxcCreateInstance_Mac(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils))), "Failed to create DXC utils!");
        ASSERT_EQ(SUCCEEDED(DxcCreateInstance_Mac(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler))), "Failed too create DXC compiler!");
#endif

        // Create custom include handler that looks in Data/Shaders directory
        CustomIncludeHandler* pIncludeHandler = new CustomIncludeHandler(pUtils, "Data/Shaders");

        IDxcBlobEncoding* pSourceBlob = nullptr;
        ASSERT_EQ(SUCCEEDED(pUtils->CreateBlob(sourceCstr, source.size(), 0, &pSourceBlob)), "Failed to create source blob!");

        Array<LPCWSTR> args = {
            L"-Zi",
            L"-Qembed_debug"
        };
        if (sData.Backend == RHIBackend::kVulkan) {
            args.push_back(L"-DVULKAN");
            args.push_back(L"-spirv");
            args.push_back(L"-fspv-extension=SPV_EXT_mesh_shader");
            args.push_back(L"-fspv-extension=SPV_EXT_descriptor_indexing");
            args.push_back(L"-fspv-extension=SPV_KHR_ray_tracing");
            args.push_back(L"-fspv-extension=SPV_KHR_ray_query");
            args.push_back(L"-fspv-extension=SPV_KHR_shader_draw_parameters");
            args.push_back(L"-fspv-extension=SPV_EXT_demote_to_helper_invocation");
            args.push_back(L"-fvk-allow-rwstructuredbuffer-arrays");
            args.push_back(L"-fspv-target-env=vulkan1.3");
        }

        IDxcOperationResult* pResult = nullptr;
        pCompiler->Compile(pSourceBlob, L"Shader", wideEntry, wideTarget, args.data(), args.size(), nullptr, 0, pIncludeHandler, &pResult);

        IDxcBlobEncoding* pErrors = nullptr;
        pResult->GetErrorBuffer(&pErrors);

        if (pErrors && pErrors->GetBufferSize() != 0) {
            IDxcBlobUtf8* pErrorsU8 = nullptr;
            pErrors->QueryInterface(IID_PPV_ARGS(&pErrorsU8));
            SERAPH_ERROR("Shader errors: %s", (char*)pErrorsU8->GetStringPointer());
            pErrorsU8->Release();
            pErrors->Release();
            return {};
        }

        HRESULT Status;
        pResult->GetStatus(&Status);

        IDxcBlob* pShaderBlob = nullptr;
        pResult->GetResult(&pShaderBlob);

        result.Entries[entry] = {};
        result.Entries[entry].Stage = type;
        result.Entries[entry].Entry = entry;
        result.Entries[entry].Bytecode.resize(pShaderBlob->GetBufferSize());
        memcpy(result.Entries[entry].Bytecode.data(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());

        if (pShaderBlob) pShaderBlob->Release();
        if (pErrors) pErrors->Release();
        if (pResult) pResult->Release();
        if (pSourceBlob) pSourceBlob->Release();
        if (pIncludeHandler) pIncludeHandler->Release();
        if (pCompiler) pCompiler->Release();
        if (pUtils) pUtils->Release();
    }

    SERAPH_INFO("Compiled shader : %s", path.c_str());
    return result;
}
