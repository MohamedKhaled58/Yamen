bool C3PhyLoader::ParseMAXFILEFormat(const uint8_t *data, size_t size,
                                     C3Phy &outPhy) {
  size_t offset = 0;

  // Check for MAXFILE header (20 bytes)
  char header[21] = {0};
  if (!Read(data, offset, size, header, 20)) {
    return false;
  }

  if (memcmp(header, "MAXFILE C3", 10) != 0) {
    YAMEN_CORE_ERROR("Not a MAXFILE format");
    return false;
  }

  YAMEN_CORE_INFO("Detected MAXFILE format: {}", std::string(header, 20));

  // Read data size (offset 20)
  uint32_t dataSize;
  if (!Read(data, offset, size, dataSize)) {
    return false;
  }

  // Read part/bone count (offset 24)
  uint32_t partCount;
  if (!Read(data, offset, size, partCount)) {
    return false;
  }

  YAMEN_CORE_INFO("MAXFILE data size: {}, parts: {}", dataSize, partCount);

  // Read name (null-terminated string)
  std::string name;
  while (offset < size && data[offset] != 0) {
    name += (char)data[offset++];
  }
  offset++; // Skip null terminator

  YAMEN_CORE_INFO("Model name: {}", name);

  // Read vertex count
  uint32_t vertexCount;
  if (!Read(data, offset, size, vertexCount)) {
    return false;
  }

  // Read triangle count
  uint32_t triangleCount;
  if (!Read(data, offset, size, triangleCount)) {
    return false;
  }

  YAMEN_CORE_INFO("Vertices: {}, Triangles: {}", vertexCount, triangleCount);

  // Allocate vertices
  outPhy.vertices.resize(vertexCount);
  outPhy.normalVertexCount = vertexCount;
  outPhy.alphaVertexCount = 0;

  // Read vertex data
  for (uint32_t i = 0; i < vertexCount; ++i) {
    auto &v = outPhy.vertices[i];

    // Position
    if (!Read(data, offset, size, v.position.x))
      return false;
    if (!Read(data, offset, size, v.position.y))
      return false;
    if (!Read(data, offset, size, v.position.z))
      return false;

    // Normal
    if (!Read(data, offset, size, v.normal.x))
      return false;
    if (!Read(data, offset, size, v.normal.y))
      return false;
    if (!Read(data, offset, size, v.normal.z))
      return false;

    // UV
    if (!Read(data, offset, size, v.uv.x))
      return false;
    if (!Read(data, offset, size, v.uv.y))
      return false;

    // Bone indices (2 bones for dual-bone blending)
    if (!Read(data, offset, size, v.boneIndices[0]))
      return false;
    if (!Read(data, offset, size, v.boneIndices[1]))
      return false;

    // Bone weights
    if (!Read(data, offset, size, v.boneWeights[0]))
      return false;
    v.boneWeights[1] = 1.0f - v.boneWeights[0]; // Ensure they sum to 1
  }

  // Read indices
  outPhy.indices.resize(triangleCount * 3);
  outPhy.normalTriCount = triangleCount;
  outPhy.alphaTriCount = 0;

  for (uint32_t i = 0; i < triangleCount * 3; ++i) {
    uint16_t index;
    if (!Read(data, offset, size, index))
      return false;
    outPhy.indices[i] = index;
  }

  YAMEN_CORE_INFO(
      "Successfully parsed MAXFILE format: {} vertices, {} triangles",
      vertexCount, triangleCount);

  return true;
}

// Helper for reading fixed-size arrays
template <typename T>
bool C3PhyLoader::Read(const uint8_t *data, size_t &offset, size_t dataSize,
                       T *buffer, size_t count) {
  size_t bytesToRead = sizeof(T) * count;
  if (offset + bytesToRead > dataSize) {
    return false;
  }
  memcpy(buffer, data + offset, bytesToRead);
  offset += bytesToRead;
  return true;
}
