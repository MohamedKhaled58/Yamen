import struct

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

with open(filepath, 'rb') as f:
    data = f.read()
    
offset = 0

# Skip MAXFILE header
if data[:10] == b'MAXFILE C3':
    print("✓ MAXFILE header detected")
    offset = 16
    print(f"Offset after header: {offset}")

# Read chunk header
chunk_id = data[offset:offset+4]
chunk_size = struct.unpack('<I', data[offset+4:offset+8])[0]
print(f"\n✓ Chunk ID: {chunk_id} ({chunk_id.decode('ascii', errors='replace')})")
print(f"✓ Chunk Size: {chunk_size} bytes")
offset += 8

chunk_end = offset + chunk_size
print(f"✓ Chunk data range: [{offset}, {chunk_end})")

# Read name length
name_len = struct.unpack('<I', data[offset:offset+4])[0]
print(f"\n✓ Name length: {name_len}")
offset += 4

# Read name
name = data[offset:offset+name_len].decode('ascii', errors='replace')
print(f"✓ Model name: '{name}'")
offset += name_len

# Read blend count
blend_count = struct.unpack('<I', data[offset:offset+4])[0]
print(f"✓ Blend count: {blend_count}")
offset += 4

# Read vertex counts
normal_verts = struct.unpack('<I', data[offset:offset+4])[0]
alpha_verts = struct.unpack('<I', data[offset+4:offset+8])[0]
total_verts = normal_verts + alpha_verts
print(f"✓ Normal vertices: {normal_verts}")
print(f"✓ Alpha vertices: {alpha_verts}")
print(f"✓ Total vertices: {total_verts}")
offset += 8

# Calculate expected vertex data size
# Each vertex: position (12) + normal (12) + uv (8) + bone indices (blend_count) + bone weights (blend_count * 4)
vertex_size = 32 + blend_count + (blend_count * 4)
total_vertex_bytes = total_verts * vertex_size
print(f"\n✓ Calculated vertex size: {vertex_size} bytes")
print(f"✓ Expected total vertex data: {total_vertex_bytes} bytes")

# Try to read all vertices
print(f"\n=== Parsing Vertices ===")
for i in range(min(3, total_verts)):  # Just first 3 for display
    vert_start = offset
    
    # Position
    pos = struct.unpack('<fff', data[offset:offset+12])
    offset += 12
    
    # Normal
    normal = struct.unpack('<fff', data[offset:offset+12])
    offset += 12
    
    # UV
    uv = struct.unpack('<ff', data[offset:offset+8])
    offset += 8
    
    # Bone indices
    bone_indices = list(struct.unpack(f'<{blend_count}B', data[offset:offset+blend_count]))
    offset += blend_count
    
    # Bone weights
    bone_weights = list(struct.unpack(f'<{blend_count}f', data[offset:offset+blend_count*4]))
    offset += blend_count * 4
    
    print(f"  Vertex {i}: pos={pos[:2]}... normal={normal[:2]}... uv={uv} bones={bone_indices} weights={bone_weights}")

# Skip remaining vertices
offset += (total_verts - 3) * vertex_size

print(f"\n✓ Offset after all vertices: {offset}")

# Read triangle counts
normal_tris = struct.unpack('<I', data[offset:offset+4])[0]
alpha_tris = struct.unpack('<I', data[offset+4:offset+8])[0]
total_tris = normal_tris + alpha_tris
print(f"✓ Normal triangles: {normal_tris}")
print(f"✓ Alpha triangles: {alpha_tris}")
print(f"✓ Total triangles: {total_tris}")
offset += 8

# Read indices
index_count = total_tris * 3
index_bytes = index_count * 2  # uint16_t
print(f"✓ Index count: {index_count}")
print(f"✓ Index data size: {index_bytes} bytes")
offset += index_bytes

print(f"\n✓ Offset after indices: {offset}")
print(f"✓ Chunk end: {chunk_end}")
print(f"✓ Remaining bytes in chunk: {chunk_end - offset}")

# Check if we read correctly
if offset <= chunk_end:
    print(f"\n✅ SUCCESS: Parsing completed within chunk bounds!")
    print(f"   Bytes consumed: {offset - 24}")  # -24 for chunk header + size
    print(f"   Chunk size: {chunk_size}")
    print(f"   Remaining: {chunk_end - offset}")
else:
    print(f"\n❌ ERROR: Exceeded chunk bounds!")
    print(f"   Offset: {offset}")
    print(f"   Chunk end: {chunk_end}")
    print(f"   Overrun: {offset - chunk_end} bytes")
