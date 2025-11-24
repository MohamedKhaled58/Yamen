#pragma once

#include "Graphics/RHI/GraphicsDevice.h"
#include <vector>

namespace Yamen::Graphics {

    /**
     * @brief Input element semantic
     */
    enum class InputSemantic {
        Position,
        Normal,
        TexCoord,
        Color,
        Tangent,
        Binormal
    };

    /**
     * @brief Input element format
     */
    enum class InputFormat {
        Float,
        Float2,
        Float3,
        Float4,
        UInt,
        UInt2,
        UInt3,
        UInt4
    };

    /**
     * @brief Input element description
     */
    struct InputElement {
        InputSemantic semantic;
        InputFormat format;
        uint32_t semanticIndex;
        uint32_t inputSlot;
        uint32_t alignedByteOffset;
        bool perInstance;

        InputElement(InputSemantic sem, InputFormat fmt, uint32_t semIdx = 0, uint32_t slot = 0, 
                    uint32_t offset = 0xFFFFFFFF, bool instance = false)
            : semantic(sem), format(fmt), semanticIndex(semIdx), inputSlot(slot)
            , alignedByteOffset(offset), perInstance(instance)
        {}
    };

    /**
     * @brief Vertex input layout
     */
    class InputLayout {
    public:
        InputLayout(GraphicsDevice& device);
        ~InputLayout();

        // Non-copyable
        InputLayout(const InputLayout&) = delete;
        InputLayout& operator=(const InputLayout&) = delete;

        /**
         * @brief Create input layout from elements and shader bytecode
         */
        bool Create(const std::vector<InputElement>& elements, const void* shaderBytecode, size_t bytecodeLength);

        /**
         * @brief Bind input layout
         */
        void Bind();

        /**
         * @brief Get D3D11 input layout
         */
        ID3D11InputLayout* GetInputLayout() const { return m_InputLayout.Get(); }

    private:
        GraphicsDevice& m_Device;
        ComPtr<ID3D11InputLayout> m_InputLayout;
    };

} // namespace Yamen::Graphics
