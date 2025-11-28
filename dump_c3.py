import struct

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

with open(filepath, 'rb') as f:
    # Read first 512 bytes
    data = f.read(512)
    
    print("=== File Header ===")
    print(f"First 20 bytes (ASCII): {data[:20]}")
    print(f"First 20 bytes (hex): {data[:20].hex()}")
    print()
    
    # Check for MAXFILE header
    if data[:10] == b'MAXFILE C3':
        print("DETECTED: MAXFILE format")
        print(f"Full header (16 bytes): {data[:16]}")
        offset = 16
    else:
        print("NOT MAXFILE - checking for chunk format")
        # Check for chunk ID
        chunk_id = data[:4]
        print(f"First 4 bytes (chunk ID?): {chunk_id}")
        offset = 0
    
    print()
    print("=== Chunk Header ===")
    if offset < len(data) - 8:
        chunk_id = data[offset:offset+4]
        chunk_size = struct.unpack('<I', data[offset+4:offset+8])[0]
        print(f"Chunk ID: {chunk_id} ({chunk_id.decode('ascii', errors='replace')})")
        print(f"Chunk Size: {chunk_size} bytes")
        offset += 8
        
        # If it's PHY chunk, read the structure
        if chunk_id in [b'PHY ', b'PHYS']:
            print()
            print("=== PHY Chunk Data ===")
            
            # Read name length
            name_len = struct.unpack('<I', data[offset:offset+4])[0]
            print(f"Name length: {name_len}")
            offset += 4
            
            if name_len > 0 and name_len < 256:
                name = data[offset:offset+name_len].decode('ascii', errors='replace')
                print(f"Model name: '{name}'")
                offset += name_len
                
            # Read blend count
            blend_count = struct.unpack('<I', data[offset:offset+4])[0]
            print(f"Blend count: {blend_count}")
            offset += 4
            
            # Read vertex counts
            normal_verts = struct.unpack('<I', data[offset:offset+4])[0]
            alpha_verts = struct.unpack('<I', data[offset+4:offset+8])[0]
            print(f"Normal vertices: {normal_verts}")
            print(f"Alpha vertices: {alpha_verts}")
            offset += 8
            
            total_verts = normal_verts + alpha_verts
            print(f"Total vertices: {total_verts}")
            
            print()
            print("=== First Vertex Data ===")
            # Position (3 floats = 12 bytes)
            pos = struct.unpack('<fff', data[offset:offset+12])
            print(f"Position: {pos}")
            offset += 12
            
            # Normal (3 floats = 12 bytes)
            normal = struct.unpack('<fff', data[offset:offset+12])
            print(f"Normal: {normal}")
            offset += 12
            
            # UV (2 floats = 8 bytes)
            uv = struct.unpack('<ff', data[offset:offset+8])
            print(f"UV: {uv}")
            offset += 8
            
            print(f"\nCurrent offset after pos/normal/uv: {offset}")
            print(f"Next 20 bytes (hex): {data[offset:offset+20].hex()}")
            print(f"Next 20 bytes as float32: {[struct.unpack('<f', data[offset+i:offset+i+4])[0] for i in range(0, 20, 4)]}")
            print(f"Next 20 bytes as uint8: {list(data[offset:offset+20])}")
