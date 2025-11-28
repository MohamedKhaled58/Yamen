import struct

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

with open(filepath, 'rb') as f:
    data = f.read()
    
offset = 0

# Skip MAXFILE header
offset = 16

# Skip chunk header
offset += 8

# Read name length
name_len = struct.unpack('<I', data[offset:offset+4])[0]
offset += 4

# Read name
name = data[offset:offset+name_len]
offset += name_len

# Read blend count
blend_count = struct.unpack('<I', data[offset:offset+4])[0]
offset += 4

# Read vertex counts
normal_verts, alpha_verts = struct.unpack('<II', data[offset:offset+8])
total_verts = normal_verts + alpha_verts
offset += 8

print(f"Blend count: {blend_count}")
print(f"Total vertices: {total_verts}")
print()

# Manually parse first 3 vertices with detailed byte inspection
for v_idx in range(min(5, total_verts)):
    vert_offset_start = offset
    print(f"=== Vertex {v_idx} (offset {offset}) ===")
    
    # Position (3 floats = 12 bytes)
    px, py, pz = struct.unpack('<fff', data[offset:offset+12])
    print(f"  Position: ({px:.2f}, {py:.2f}, {pz:.2f})")
    offset += 12
    
    # Normal (3 floats = 12 bytes)
    nx, ny, nz = struct.unpack('<fff', data[offset:offset+12])
    print(f"  Normal: ({nx:.2f}, {ny:.2f}, {nz:.2f})")
    offset += 12
    
    # UV (2 floats = 8 bytes)
    u, v = struct.unpack('<ff', data[offset:offset+8])
    print(f"  UV: ({u:.4f}, {v:.4f})")
    offset += 8
    
    # Show next 24 bytes in detail
    print(f"  Next 24 bytes (hex): {data[offset:offset+24].hex()}")
    print(f"  Next 24 bytes (raw): {list(data[offset:offset+24])}")
    
    # Try reading as blend_count bone indices
    bone_indices = list(struct.unpack(f'<{blend_count}B', data[offset:offset+blend_count]))
    print(f"  Bone indices (as {blend_count} bytes): {bone_indices}")
    offset += blend_count
    
    # Try reading as blend_count bone weights
    bone_weights = list(struct.unpack(f'<{blend_count}f', data[offset:offset+blend_count*4]))
    print(f"  Bone weights (as {blend_count} floats): {bone_weights}")
    offset += blend_count * 4
    
    bytes_consumed = offset - vert_offset_start
    print(f"  Total bytes for this vertex: {bytes_consumed}")
    print()
