import struct
import os

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

def inspect_vertex(f, offset):
    f.seek(offset)
    data = f.read(76)
    print(f"--- Vertex at {offset} (76 bytes) ---")
    print(f"Hex: {data.hex()}")
    
    # Try to identify fields
    # Pos (12), Norm (12), UV (8)
    floats = struct.unpack('<' + 'f'*19, data) # 76 bytes = 19 floats
    print("Floats:")
    for i in range(0, 19, 4):
        chunk = floats[i:min(i+4, 19)]
        print(f"  {i}-{i+3}: " + ", ".join([f"{v:.4f}" for v in chunk]))

if os.path.exists(filepath):
    with open(filepath, 'rb') as f:
        start_vertices = 46
        inspect_vertex(f, start_vertices)
