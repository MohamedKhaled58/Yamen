import struct

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

with open(filepath, 'rb') as f:
    data = f.read()

# After InitMatrix, we're at offset 48014
offset = 48014

print(f"=== Data at offset {offset} (where we expect triangle counts) ===")
print(f"Next 64 bytes (hex): {data[offset:offset+64].hex()}")
print()

# Try reading as uint32_t
print("Interpreting as uint32_t:")
for i in range(0, 32, 4):
    val = struct.unpack('<I', data[offset+i:offset+i+4])[0]
    print(f"  Offset {offset+i}: {val}")

print()
print("=== Searching for reasonable triangle count ===")
# We have 1140 vertices, reasonable triangle count would be 1000-3000
# Let's search backwards from offset 48014 for reasonable values
for search_offset in range(48014, 47900, -4):
    val1 = struct.unpack('<I', data[search_offset:search_offset+4])[0]
    val2 = struct.unpack('<I', data[search_offset+4:search_offset+8])[0]
    total = val1 + val2
    
    if 500 < total < 5000:  # Reasonable triangle count range
        print(f"Found potential triangle counts at offset {search_offset}:")
        print(f"  Normal tris: {val1}, Alpha tris: {val2}, Total: {total}")
        print(f"  Bytes before this: {search_offset - 47926} (after vertices at 47926)")
        print(f"  Next 32 bytes (hex): {data[search_offset:search_offset+32].hex()}")
        print()
