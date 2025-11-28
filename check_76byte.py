import struct
import os

filepath = r"C:\dev\C3Renderer\Yamen\Assets\C3\ghost\140\1.c3"

def read_at_offset(f, offset, label):
    f.seek(offset)
    data = f.read(32)
    print(f"--- {label} (Offset {offset}) ---")
    print(f"Hex: {data.hex()}")
    if len(data) >= 8:
        u1, u2 = struct.unpack('<II', data[:8])
        print(f"As uint32: {u1}, {u2}")
    if len(data) >= 12:
        f1, f2, f3 = struct.unpack('<fff', data[:12])
        print(f"As float: {f1:.2f}, {f2:.2f}, {f3:.2f}")
    print()

if os.path.exists(filepath):
    with open(filepath, 'rb') as f:
        # 76-byte hypothesis
        start_vertices = 46
        num_vertices = 1140
        offset_76 = start_vertices + (num_vertices * 76)
        
        print(f"End if 76 bytes: {offset_76}")
        read_at_offset(f, offset_76, "End of 76-byte vertices")
