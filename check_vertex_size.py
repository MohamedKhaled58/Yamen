import struct
import os

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

def read_at_offset(f, offset, label):
    f.seek(offset)
    data = f.read(32)
    print(f"--- {label} (Offset {offset}) ---")
    print(f"Hex: {data.hex()}")
    # Try interpreting as uint32 (triangle counts?)
    if len(data) >= 8:
        u1, u2 = struct.unpack('<II', data[:8])
        print(f"As uint32: {u1}, {u2}")
    # Try interpreting as float (bbox?)
    if len(data) >= 12:
        f1, f2, f3 = struct.unpack('<fff', data[:12])
        print(f"As float: {f1:.2f}, {f2:.2f}, {f3:.2f}")
    print()

if os.path.exists(filepath):
    with open(filepath, 'rb') as f:
        # Header is 16 bytes
        # Name "v_body" (6) + len(4) = 10 bytes -> Offset 26
        # BlendCount(4) -> Offset 30
        # VertCounts(8) -> Offset 38
        # Vertices start at Offset 46 (approx, based on logs it was 46)
        
        start_vertices = 46
        num_vertices = 1140
        
        offset_40 = start_vertices + (num_vertices * 40)
        offset_42 = start_vertices + (num_vertices * 42)
        
        print(f"Start of vertices: {start_vertices}")
        print(f"Num vertices: {num_vertices}")
        print(f"End if 40 bytes: {offset_40}")
        print(f"End if 42 bytes: {offset_42}")
        print()
        
        read_at_offset(f, offset_40, "End of 40-byte vertices")
        read_at_offset(f, offset_42, "End of 42-byte vertices")
        
        # Also check user's code hypothesis:
        # If 40 bytes, then Triangles -> Indices -> Texture -> BBox -> Matrix
        # Let's see if we find triangle counts at offset_40
