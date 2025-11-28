import struct

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

with open(filepath, 'rb') as f:
    data = f.read()

# Jump to offset 47926 (after vertices)
offset = 47926

print(f"=== Data at offset {offset} (after vertices) ===")
print(f"Next 64 bytes (hex): {data[offset:offset+64].hex()}")
print(f"Next 64 bytes (raw): {list(data[offset:offset+64])}")

# Try interpreting as different data types
print(f"\nAs uint32_t: {[struct.unpack('<I', data[offset+i:offset+i+4])[0] for i in range(0, 32, 4)]}")
print(f"As float32: {[struct.unpack('<f', data[offset+i:offset+i+4])[0] for i in range(0, 32, 4)]}")
print(f"As uint16_t: {[struct.unpack('<H', data[offset+i:offset+i+2])[0] for i in range(0, 32, 2)]}")

# Check if there's a bounding box (6 floats = 24 bytes)
print(f"\n=== Checking for bounding box (6 floats) ===")
bbox = struct.unpack('<ffffff', data[offset:offset+24])
print(f"Potential bbox: min({bbox[0]:.2f}, {bbox[1]:.2f}, {bbox[2]:.2f}), max({bbox[3]:.2f}, {bbox[4]:.2f}, {bbox[5]:.2f})")

# Try skipping 24 bytes (bounding box)
offset_after_bbox = offset + 24
print(f"\n=== Data at offset {offset_after_bbox} (after potential bbox) ===")
print(f"Next 32 bytes (hex): {data[offset_after_bbox:offset_after_bbox+32].hex()}")

# Try reading triangle counts there
tri_normal, tri_alpha = struct.unpack('<II', data[offset_after_bbox:offset_after_bbox+8])
print(f"Triangle counts if bbox exists: normal={tri_normal}, alpha={tri_alpha}, total={tri_normal + tri_alpha}")

# Also check for InitMatrix (64 bytes = 4x4 matrix)
print(f"\n=== Checking for InitMatrix (64 bytes) ===")
offset_after_bbox_and_matrix = offset + 24 + 64
print(f"Offset after bbox+matrix: {offset_after_bbox_and_matrix}")
if offset_after_bbox_and_matrix < len(data):
    tri_normal2, tri_alpha2 = struct.unpack('<II', data[offset_after_bbox_and_matrix:offset_after_bbox_and_matrix+8])
    print(f"Triangle counts if bbox+matrix exist: normal={tri_normal2}, alpha={tri_alpha2}, total={tri_normal2 + tri_alpha2}")
