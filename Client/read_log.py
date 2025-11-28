
with open(r'c:\dev\C3Renderer\Yamen\Client\Yamen.log', 'r', encoding='utf-8', errors='ignore') as f:
    lines = f.readlines()
    for i, line in enumerate(lines):
        if "Stride=24" in line:
            print("FOUND Stride=24 at line", i)
            for j in range(max(0, i-10), min(len(lines), i+20)):
                print(lines[j].strip())
