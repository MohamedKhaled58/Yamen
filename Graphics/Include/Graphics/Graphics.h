#pragma once

// RHI
#include "Graphics/RHI/GraphicsDevice.h"
#include "Graphics/RHI/SwapChain.h"
#include "Graphics/RHI/RenderTarget.h"
#include "Graphics/RHI/DepthStencilBuffer.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Sampler.h"

// Shader
#include "Graphics/Shader/ShaderCompiler.h"
#include "Graphics/Shader/Shader.h"

// Texture
#include "Graphics/Texture/Texture2D.h"
#include "Graphics/Texture/TextureLoader.h"

// Renderer
#include "Graphics/Renderer/Camera2D.h"
#include "Graphics/Renderer/Camera3D.h"

// Mesh
#include "Graphics/Mesh/Vertex.h"

// Lighting
#include "Graphics/Lighting/Light.h"

namespace Yamen::Graphics {

    /**
     * @brief Initialize graphics subsystem
     */
    inline bool InitializeGraphics() {
        // Placeholder for future initialization
        return true;
    }

    /**
     * @brief Shutdown graphics subsystem
     */
    inline void ShutdownGraphics() {
        // Placeholder for future cleanup
    }

} // namespace Yamen::Graphics
