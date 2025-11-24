# Yamen Engine & YServer

Professional C++23 game engine and Conquer Online 2.0 client recreation with high-performance server.

## Features

- **Modern C++23** - Latest language features
- **DirectX 11** - Hardware-accelerated rendering
- **ECS Architecture** - Entity Component System with entt
- **Professional Event System** - Priority-based, thread-safe events
- **Advanced Layer System** - Isolated execution contexts
- **World Management** - Spatial partitioning, streaming, LOD
- **C3 Asset Pipeline** - Hot reload, streaming, dependency management
- **ASIO Networking** - Async TCP for client and server
- **Protobuf** - Efficient server-client communication
- **Original Shaders** - Ported from Conquer Online

## Project Structure

```
Yamen/
├── EngineCore/          # Core systems (memory, logging, math)
├── Platform/            # Win32 platform layer
├── Graphics/            # DirectX 11 renderer
├── ECS/                 # Entity Component System
├── Network/             # Client networking
├── IO/                  # File format parsers
├── AssetsC3/            # C3 asset pipeline
├── World/               # World management
├── Client/              # Game client
├── Tools/               # Debug tools
├── YServer/             # Server solution
├── ThirdParty/          # Git submodules
└── LookAt/              # Original Conquer assets
```

## Building

### Prerequisites

- Visual Studio 2022 or later
- Windows 10/11 SDK
- Git
- Premake5

### Setup

```bash
# Clone with submodules
git clone --recursive <repository-url>

# Or if already cloned
git submodule update --init --recursive

# Generate project files
premake5 vs2022

# Open solution
start Yamen.sln
```

## Third-Party Libraries

- [entt](https://github.com/skypjack/entt) - Entity Component System
- [ASIO](https://github.com/chriskohlhoff/asio) - Async I/O
- [spdlog](https://github.com/gabime/spdlog) - Logging
- [fmt](https://github.com/fmtlib/fmt) - Formatting
- [ImGui](https://github.com/ocornut/imgui) - Debug UI
- [GLM](https://github.com/g-truc/glm) - Math library
- [DirectXTex](https://github.com/microsoft/DirectXTex) - Texture processing
- [lz4](https://github.com/lz4/lz4) - Compression
- [xxHash](https://github.com/Cyan4973/xxHash) - Hashing
- [Protobuf](https://github.com/protocolbuffers/protobuf) - Serialization

## License

MIT License - See LICENSE file for details
