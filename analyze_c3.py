import struct

# Read the C3 file
with open(r'C:/dev/C3Renderer/Yamen/Assets/C3/ghost/085/1.c3', 'rb') as f:
    data = f.read()

print("="*60)
print("C3 MAXFILE Format Analysis")
print("="*60)

# Parse header
header = data[:20].decode('ascii', errors='ignore')
print(f"\nHeader: '{header}'")
print(f"Total file size: {len(data)} bytes")

# Try to parse structure starting at offset 20
offset = 20
print(f"\n--- Structure Analysis ---")

# Read potential counts
val1 = struct.unpack('<I', data[offset:offset+4])[0]
val2 = struct.unpack('<I', data[offset+4:offset+8])[0]
print(f"Offset {offset}: {val1} (could be data size or vertex count)")
print(f"Offset {offset+4}: {val2} (could be bone/part count)")

# Try to find string
offset = 28
strlen = 0
for i in range(offset, min(offset+100, len(data))):
    if data[i] == 0:
        strlen = i - offset
        break

if strlen > 0:
    string = data[offset:offset+strlen].decode('ascii', errors='ignore')
    print(f"\nString at offset {offset}: '{string}'")
    
# Look for more structure
print("\n--- Hex dump of first 200 bytes ---")
for i in range(0, min(200, len(data)), 16):
    hex_part = ' '.join(f'{data[j]:02X}' for j in range(i, min(i+16, len(data))))
    ascii_part = ''.join(chr(data[j]) if 32 <= data[j] < 127 else '.' for j in range(i, min(i+16, len(data))))
    print(f"{i:04X}: {hex_part:<48} {ascii_part}")

print("\n" + "="*60)
