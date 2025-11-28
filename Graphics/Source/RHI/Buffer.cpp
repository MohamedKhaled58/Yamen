
#include "Graphics/RHI/Buffer.h"
#include <Core/Logging/Logger.h>

namespace Yamen::Graphics {

Buffer::Buffer(GraphicsDevice &device, BufferType type)
    : m_Device(device), m_Type(type), m_Usage(BufferUsage::Default), m_Size(0),
      m_Stride(0) {}

Buffer::~Buffer() { m_Buffer.Reset(); }

bool Buffer::Create(const void *data, uint32_t size, uint32_t stride,
                    BufferUsage usage) {
  m_Size = size;
  m_Stride = stride;
  m_Usage = usage;

  // Determine D3D11 usage and CPU access flags
  D3D11_USAGE d3dUsage;
  UINT cpuAccessFlags = 0;

  switch (usage) {
  case BufferUsage::Default:
    d3dUsage = D3D11_USAGE_DEFAULT;
    break;
  case BufferUsage::Dynamic:
    d3dUsage = D3D11_USAGE_DYNAMIC;
    cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
    break;
  case BufferUsage::Immutable:
    d3dUsage = D3D11_USAGE_IMMUTABLE;
    break;
  default:
    d3dUsage = D3D11_USAGE_DEFAULT;
    break;
  }

  // Determine bind flags based on buffer type
  UINT bindFlags = 0;
  switch (m_Type) {
  case BufferType::Vertex:
    bindFlags = D3D11_BIND_VERTEX_BUFFER;
    break;
  case BufferType::Index:
    bindFlags = D3D11_BIND_INDEX_BUFFER;
    break;
  case BufferType::Constant:
    bindFlags = D3D11_BIND_CONSTANT_BUFFER;
    break;
  }

  // Create buffer description
  D3D11_BUFFER_DESC bufferDesc = {};
  bufferDesc.ByteWidth = size;
  bufferDesc.Usage = d3dUsage;
  bufferDesc.BindFlags = bindFlags;
  bufferDesc.CPUAccessFlags = cpuAccessFlags;
  bufferDesc.MiscFlags = 0;
  bufferDesc.StructureByteStride = 0;

  // Initial data (optional)
  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = data;
  D3D11_SUBRESOURCE_DATA *pInitData = data ? &initData : nullptr;

  // Create buffer
  HRESULT hr =
      m_Device.GetDevice()->CreateBuffer(&bufferDesc, pInitData, &m_Buffer);
  if (FAILED(hr)) {
    YAMEN_CORE_ERROR("Failed to create buffer: 0x{:08X}",
                     static_cast<uint32_t>(hr));
    return false;
  }

  const char *typeStr = "Unknown";
  switch (m_Type) {
  case BufferType::Vertex:
    typeStr = "Vertex";
    break;
  case BufferType::Index:
    typeStr = "Index";
    break;
  case BufferType::Constant:
    typeStr = "Constant";
    break;
  }

  YAMEN_CORE_TRACE("Created {} buffer ({} bytes, stride: {})", typeStr, size,
                   stride);
  return true;
}

void Buffer::Update(const void *data, uint32_t size) {
  if (!m_Buffer) {
    YAMEN_CORE_WARN("Cannot update null buffer (Create failed?)");
    return;
  }
  if (m_Usage != BufferUsage::Dynamic) {
    YAMEN_CORE_WARN("Cannot update non-dynamic buffer");
    return;
  }

  if (size > m_Size) {
    YAMEN_CORE_WARN("Update size ({}) exceeds buffer size ({})", size, m_Size);
    size = m_Size;
  }

  // Map buffer for writing
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  HRESULT hr = m_Device.GetContext()->Map(
      m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

  if (FAILED(hr)) {
    YAMEN_CORE_ERROR("Failed to map buffer for update");
    return;
  }

  // Copy data
  memcpy(mappedResource.pData, data, size);

  // Unmap
  m_Device.GetContext()->Unmap(m_Buffer.Get(), 0);
}

void Buffer::Bind() {
  auto context = m_Device.GetContext();

  switch (m_Type) {
  case BufferType::Vertex: {
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_Buffer.GetAddressOf(), &m_Stride,
                                &offset);
    break;
  }
  case BufferType::Index: {
    DXGI_FORMAT format =
        (m_Stride == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    context->IASetIndexBuffer(m_Buffer.Get(), format, 0);
    break;
  }
  case BufferType::Constant: {
    YAMEN_CORE_WARN(
        "Use SetConstantBuffer to bind constant buffers, not Bind()");
    break;
  }
  }
}

void Buffer::BindToVertexShader(uint32_t slot) {
  if (m_Type != BufferType::Constant) {
    YAMEN_CORE_WARN("BindToVertexShader called on non-constant buffer");
    return;
  }

  auto context = m_Device.GetContext();
  //YAMEN_CORE_TRACE("Buffer::BindToVertexShader slot {} - Buffer Ptr: {}", slot,
   //                (void *)m_Buffer.Get());
  context->VSSetConstantBuffers(slot, 1, m_Buffer.GetAddressOf());
}

void Buffer::BindToPixelShader(uint32_t slot) {
  if (m_Type != BufferType::Constant) {
    YAMEN_CORE_WARN("BindToPixelShader called on non-constant buffer");
    return;
  }

  auto context = m_Device.GetContext();
  context->PSSetConstantBuffers(slot, 1, m_Buffer.GetAddressOf());
}

} // namespace Yamen::Graphics
