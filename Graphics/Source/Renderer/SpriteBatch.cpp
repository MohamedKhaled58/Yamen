#include "Graphics/Renderer/SpriteBatch.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Graphics {

SpriteBatch::SpriteBatch(GraphicsDevice &device, uint32_t maxSprites)
    : m_Device(device), m_MaxSprites(maxSprites), m_SpriteCount(0),
      m_CurrentTexture(nullptr), m_Camera(nullptr), m_InBatch(false) {
  m_Vertices.reserve(maxSprites * 4);
  m_Indices.reserve(maxSprites * 6);
}

SpriteBatch::~SpriteBatch() {}

bool SpriteBatch::Initialize() {
  // Create vertex buffer (dynamic)
  m_VertexBuffer = std::make_unique<Buffer>(m_Device, BufferType::Vertex);
  if (!m_VertexBuffer->Create(nullptr, m_MaxSprites * 4 * sizeof(Vertex2D),
                              sizeof(Vertex2D), BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR("Failed to create sprite batch vertex buffer");
    return false;
  }

  // Create index buffer (immutable)
  std::vector<uint32_t> indices;
  indices.reserve(m_MaxSprites * 6);
  for (uint32_t i = 0; i < m_MaxSprites; ++i) {
    uint32_t base = i * 4;
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
    indices.push_back(base + 0);
  }

  m_IndexBuffer = std::make_unique<Buffer>(m_Device, BufferType::Index);
  if (!m_IndexBuffer->Create(
          indices.data(),
          static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
          sizeof(uint32_t), BufferUsage::Immutable)) {
    YAMEN_CORE_ERROR("Failed to create sprite batch index buffer");
    return false;
  }

  // Create shader
  m_Shader = std::make_unique<Shader>(m_Device);
  if (!m_Shader->CreateFromFiles(
          "C:/dev/C3Renderer/Yamen/Assets/Shaders/Sprite2D.hlsl",
          "C:/dev/C3Renderer/Yamen/Assets/Shaders/Sprite2D.hlsl", "VSMain",
          "PSMain")) {
    YAMEN_CORE_ERROR("Failed to load Sprite2D shader");
    return false;
  }

  // Create constant buffer for camera matrix
  m_ConstantBuffer = std::make_unique<Buffer>(m_Device, BufferType::Constant);
  // Constant buffer size must be multiple of 16 bytes
  uint32_t cbSize = sizeof(mat4); // 64 bytes (4x4 float matrix)
  if (!m_ConstantBuffer->Create(nullptr, cbSize, 0, BufferUsage::Dynamic)) {
    YAMEN_CORE_ERROR("Failed to create sprite batch constant buffer");
    return false;
  }

  // Create input layout for Vertex2D
  m_InputLayout = std::make_unique<InputLayout>(m_Device);
  std::vector<InputElement> elements = {
      InputElement(InputSemantic::Position, InputFormat::Float3, 0, 0, 0),
      InputElement(InputSemantic::Color, InputFormat::Float4, 0, 0, 12),
      InputElement(InputSemantic::TexCoord, InputFormat::Float2, 0, 0, 28)};
  if (!m_InputLayout->Create(elements,
                             m_Shader->GetVertexShaderBytecode().data(),
                             m_Shader->GetVertexShaderBytecode().size())) {
    YAMEN_CORE_ERROR("Failed to create sprite batch input layout");
    return false;
  }

  YAMEN_CORE_INFO("SpriteBatch initialized (max sprites: {})", m_MaxSprites);
  return true;
}

void SpriteBatch::Begin(Camera2D *camera) {
  if (m_InBatch) {
    YAMEN_CORE_WARN("SpriteBatch::Begin called while already in batch");
    return;
  }

  m_Camera = camera;
  m_InBatch = true;
  m_SpriteCount = 0;
  m_CurrentTexture = nullptr;
  m_Vertices.clear();
  m_Indices.clear();
}

void SpriteBatch::DrawSprite(Texture2D *texture, const vec2 &position,
                             const vec2 &size, float rotation,
                             const vec4 &color, const vec2 &origin) {
  if (!m_InBatch) {
    YAMEN_CORE_WARN("SpriteBatch::DrawSprite called outside Begin/End");
    return;
  }

  // Flush if texture changed or batch is full
  if (m_CurrentTexture != texture || m_SpriteCount >= m_MaxSprites) {
    Flush();
    m_CurrentTexture = texture;
  }

  // Calculate transform
  mat4 transform =
      Math::Translate(mat4(1.0f), vec3(position.x, position.y, 0.0f));
  if (rotation != 0.0f) {
    transform = Math::Rotate(transform, rotation, vec3(0.0f, 0.0f, 1.0f));
  }
  transform = Math::Translate(
      transform, vec3(-origin.x * size.x, -origin.y * size.y, 0.0f));
  transform = Math::Scale(transform, vec3(size.x, size.y, 1.0f));

  // Quad vertices
  // Transform is row-major in our math lib (DirectXMath wrapper), but operator*
  // is matrix multiplication. We need to transform vectors. vec4
  // operator*(float) exists, but mat4 * vec4 does NOT exist in our wrapper yet.
  // Actually, let's check Math.h. I didn't see mat4 * vec4.
  // I saw vec3 Rotate(quat, vec3).
  // I should probably use XMVector3Transform or similar if I want to transform
  // points. Or just implement mat4 * vec4.

  // Wait, I can't easily add operator* to mat4 for vec4 without modifying
  // Math.h again. But I can use a helper or just do it manually if needed.
  // Actually, let's look at how I implemented mat4. It wraps XMFLOAT4X4.
  // I can use XMVector3TransformCoord for points (w=1) or
  // XMVector3TransformNormal for vectors (w=0). Or XMVector4Transform.

  // Let's use a helper lambda or just do it here using DirectXMath directly
  // since we are in .cpp and can include it? No, Math.h includes it. But `using
  // namespace Yamen::Core` gives us `mat4`.

  // Let's assume I can use XMVector4Transform.
  // But wait, `mat4` is `XMFLOAT4X4`. `vec4` is `XMFLOAT4`.
  // I need to load them to XMVECTOR/XMMATRIX.

  auto TransformPoint = [](const mat4 &m, const vec4 &v) -> vec3 {
    XMMATRIX mat = XMLoadFloat4x4(&m);
    XMVECTOR vec = XMLoadFloat4(&v);
    XMVECTOR result = XMVector4Transform(vec, mat);
    vec3 res;
    XMStoreFloat3(&res, result);
    return res;
  };

  vec3 positions[4] = {TransformPoint(transform, vec4(0.0f, 0.0f, 0.0f, 1.0f)),
                       TransformPoint(transform, vec4(1.0f, 0.0f, 0.0f, 1.0f)),
                       TransformPoint(transform, vec4(1.0f, 1.0f, 0.0f, 1.0f)),
                       TransformPoint(transform, vec4(0.0f, 1.0f, 0.0f, 1.0f))};

  vec2 texCoords[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

  // Add vertices
  for (int i = 0; i < 4; ++i) {
    m_Vertices.emplace_back(positions[i], color, texCoords[i]);
  }

  m_SpriteCount++;
}

void SpriteBatch::End() {
  if (!m_InBatch) {
    YAMEN_CORE_WARN("SpriteBatch::End called without Begin");
    return;
  }

  Flush();
  m_InBatch = false;
}

void SpriteBatch::Flush() {
  if (m_Vertices.empty()) {
    return;
  }

  // Update vertex buffer
  m_VertexBuffer->Update(
      m_Vertices.data(),
      static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex2D)));

  // Bind resources
  m_VertexBuffer->Bind();
  m_IndexBuffer->Bind();
  m_InputLayout->Bind(); // Bind input layout
  if (m_CurrentTexture) {
    m_CurrentTexture->Bind(0);
  }

  // Bind shader
  m_Shader->Bind();

  // Update and bind constant buffer with camera matrix
  if (m_Camera) {
    mat4 viewProj = Math::Transpose(m_Camera->GetViewProjectionMatrix());
    m_ConstantBuffer->Update(&viewProj, sizeof(mat4));
    m_ConstantBuffer->BindToVertexShader(0); // Bind to slot 0 (b0)
  }

  // Draw
  auto context = m_Device.GetContext();
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  uint32_t indexCount = m_SpriteCount * 6;
  context->DrawIndexed(indexCount, 0, 0);

  // Clear batch
  m_Vertices.clear();
  m_SpriteCount = 0;
}

} // namespace Yamen::Graphics
