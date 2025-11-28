import struct
import os

base_path = r"C:\dev\C3Renderer\Yamen\Assets\C3"

# Test files from different ghost IDs
test_files = [
    "ghost/085/1.c3",
    "ghost/086/1.c3", 
    "ghost/087/1.c3",
    "ghost/097/1.c3",
    "ghost/098/1.c3",
    "ghost/099/1.c3",
]

def quick_parse_c3(filepath):
    """Quickly parse C3 to check if it has triangles"""
    with open(filepath, 'rb') as f:
        data = f.read()
    
    offset = 0
    
    # Check for MAXFILE
    is_maxfile = data[:10] == b'MAXFILE C3'
    if is_maxfile:
        offset = 16
    
    # Look for PHY chunk
    while offset < len(data) - 8:
        chunk_id = data[offset:offset+4]
        chunk_size = struct.unpack('<I', data[offset+4:offset+8])[0]
        offset += 8
        
        if chunk_id in [b'PHY ', b'PHYS']:
            # Found PHY chunk, parse it
            chunk_end = offset + chunk_size
            
            # Name
            name_len = struct.unpack('<I', data[offset:offset+4])[0]
            offset += 4
            name = data[offset:offset+name_len].decode('ascii', errors='replace')
            offset += name_len
            
            # Blend count
            blend_count = struct.unpack('<I', data[offset:offset+4])[0]
            offset += 4
            
            # Vertex counts
            normal_verts, alpha_verts = struct.unpack('<II', data[offset:offset+8])
            offset += 8
            
            total_verts = normal_verts + alpha_verts
            
            # Skip vertices
            vert_size = 32 + blend_count + (blend_count * 4)
            offset += total_verts * vert_size
            
            # Skip bbox and InitMatrix
            offset += 24 + 64
            
            # Read triangle counts
            if offset + 8 <= len(data):
                normal_tris, alpha_tris = struct.unpack('<II', data[offset:offset+8])
                total_tris = normal_tris + alpha_tris
                
                return {
                    'is_maxfile': is_maxfile,
                    'name': name,
                    'vertices': total_verts,
                    'triangles': total_tris,
                    'has_geometry': total_tris > 0
                }
        
        offset = offset + chunk_size - 8  # Move to next chunk
        if offset >= len(data):
            break
    
    return None

print("=== Scanning C3 Files for Geometry ===\n")

for test_file in test_files:
    full_path = os.path.join(base_path, test_file)
    if os.path.exists(full_path):
        result = quick_parse_c3(full_path)
        if result:
            status = "✓ HAS MESH" if result['has_geometry'] else "✗ NO MESH"
            print(f"{status} | {test_file}")
            print(f"         MAXFILE: {result['is_maxfile']}, Verts: {result['vertices']}, Tris: {result['triangles']}")
        else:
            print(f"? PARSE FAILED | {test_file}")
    else:
        print(f"✗ NOT FOUND | {test_file}")
    print()

print("\n=== Recommendation ===")
print("If any file shows '✓ HAS MESH', update C3AnimationDemoScene to load that model!")
