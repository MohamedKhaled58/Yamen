#include "Graphics/RHI/InputLayout.h"
#include "Core/Logging/Logger.h"

namespace Yamen::Graphics {

    static DXGI_FORMAT GetDXGIFormat(InputFormat format) {
        switch (format) {
            case InputFormat::Float:    return DXGI_FORMAT_R32_FLOAT;
            case InputFormat::Float2:   return DXGI_FORMAT_R32G32_FLOAT;
            case InputFormat::Float3:   return DXGI_FORMAT_R32G32B32_FLOAT;
            case InputFormat::Float4:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case InputFormat::UInt:     return DXGI_FORMAT_R32_UINT;
            case InputFormat::UInt2:    return DXGI_FORMAT_R32G32_UINT;
            case InputFormat::UInt3:    return DXGI_FORMAT_R32G32B32_UINT;
            case InputFormat::UInt4:    return DXGI_FORMAT_R32G32B32A32_UINT;
            default:                    return DXGI_FORMAT_UNKNOWN;
        }
    }

    static const char* GetSemanticName(InputSemantic semantic) {
        switch (semantic) {
            case InputSemantic::Position:   return "POSITION";
            case InputSemantic::Normal:     return "NORMAL";
            case InputSemantic::TexCoord:   return "TEXCOORD";
            case InputSemantic::Color:      return "COLOR";
            case InputSemantic::Tangent:    return "TANGENT";
            case InputSemantic::Binormal:   return "BINORMAL";
            default:                        return "UNKNOWN";
        }
    }

    InputLayout::InputLayout(GraphicsDevice& device)
        : m_Device(device)
    {
    }

    InputLayout::~InputLayout() {
        m_InputLayout.Reset();
    }

    bool InputLayout::Create(const std::vector<InputElement>& elements, const void* shaderBytecode, size_t bytecodeLength) {
        std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElements;
        d3dElements.reserve(elements.size());

        for (const auto& element : elements) {
            D3D11_INPUT_ELEMENT_DESC desc = {};
            desc.SemanticName = GetSemanticName(element.semantic);
            desc.SemanticIndex = element.semanticIndex;
            desc.Format = GetDXGIFormat(element.format);
            desc.InputSlot = element.inputSlot;
            desc.AlignedByteOffset = element.alignedByteOffset;
            desc.InputSlotClass = element.perInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
            desc.InstanceDataStepRate = element.perInstance ? 1 : 0;

            d3dElements.push_back(desc);
        }

        HRESULT hr = m_Device.GetDevice()->CreateInputLayout(
            d3dElements.data(),
            static_cast<UINT>(d3dElements.size()),
            shaderBytecode,
            bytecodeLength,
            &m_InputLayout
        );

        if (FAILED(hr)) {
            YAMEN_CORE_ERROR("Failed to create input layout: 0x{:08X}", static_cast<uint32_t>(hr));
            return false;
        }

        return true;
    }

    void InputLayout::Bind() {
        m_Device.GetContext()->IASetInputLayout(m_InputLayout.Get());
    }

} // namespace Yamen::Graphics
