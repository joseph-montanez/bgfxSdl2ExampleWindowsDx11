#include <thread>
#include <functional>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <random>
#include <array>
#include <fstream>
#include "bgfx/platform.h"
#include "bx/math.h"
#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "SDL_syswm.h"
#include "main.h"

// Primitives
bool initialized;
int done, frameCount;

// SDL2
SDL_Event event;
std::unique_ptr<SDL_Window, SDL_WindowDeleter> window;
SDL_Surface *surface = nullptr;

// BGFX
bgfx::Init init{};
bgfx::VertexLayout s_layout;
uint16_t error{};
uint32_t windowWidth{}, windowHeight{};
const bgfx::Memory *vertexShader{}, *fragmentShader{};
bgfx::VertexBufferHandle vb{};
bgfx::IndexBufferHandle ib{};
bgfx::UniformHandle s_texColor{};
bgfx::ProgramHandle program{};
bgfx::TextureHandle bunnyTexture{};

bgfx::ShaderHandle vsHandle, fsHandle;

// Graphics data arrays and pointers
float view[16]{}, proj[16]{};
std::vector<App_Vertex> vertices;
std::vector<uint16_t> indices;
const bgfx::Memory *memVertices{}, *memIndices{};

// Containers and data structures
SpriteMap sprites;

int main(int argc, char *argv[]) {
    frameCount = 0;
    auto lastTime = std::chrono::high_resolution_clock::now();

    windowWidth = 800;
    windowHeight = 450;

    //------------------------------------------------------------------------------------------------------------------
    //-- 1) Initialize BGFX & SDL2 (DirectX 11)
    //------------------------------------------------------------------------------------------------------------------
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D11_DEBUG, "1");

    // Initialize SDL
    if (SDL_Init(0 | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    window = std::unique_ptr<SDL_Window, SDL_WindowDeleter>(SDL_CreateWindow(
            "App",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            (int) windowWidth, (int) windowHeight,
            SDL_WINDOW_SHOWN
    ));

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Available SDL Video Drivers" << std::endl;
    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
        SDL_RendererInfo rendererInfo = {};
        SDL_GetRenderDriverInfo(i, &rendererInfo);
        std::cout << "\t" << rendererInfo.name << std::endl;
    }

    SDL_SysWMinfo wmi;
    SDL_zero(wmi);
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(window.get(), &wmi)) {
        std::cerr << "Couldn't get window wminfo: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Set resolution
    init.type = bgfx::RendererType::Direct3D11;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = windowWidth;
    init.resolution.height = windowHeight;
    init.resolution.reset = BGFX_RESET_VSYNC;
    init.platformData.ndt = wmi.info.win.window;
    init.platformData.nwh = nullptr;
    init.platformData.type = bgfx::NativeWindowHandleType::Default;
    init.limits.transientVbSize = 1024 * 1024 * 24;

    if (!bgfx::init(init)) {
        std::cerr << "BGFX Init Error" << std::endl;
        return 1;
    }

    bgfx::reset(windowWidth, windowHeight, BGFX_RESET_VSYNC, init.resolution.format);

    if (bgfx::getRendererType() == bgfx::RendererType::Enum::Direct3D11) {
        std::cout << "BGFX Render Type: DirectX11" << std::endl;
    }

    App_PrintInitDetails(init);

    bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT);

    bgfx::frame();

    SDL_SetWindowSize(window.get(), (int) windowWidth, (int) windowHeight);
    SDL_GetWindowSize(window.get(), reinterpret_cast<int *>(&windowWidth), reinterpret_cast<int *>(&windowHeight));

    //------------------------------------------------------------------------------------------------------------------
    //-- 2) Create BGFX Resources
    //------------------------------------------------------------------------------------------------------------------
    // Set up the vertex layout
    s_layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();

    std::array<App_Vertex, 4> vertexData = {
            App_Vertex{0.0f, 32.0f, 0.0f, 0.0f, 1.0f},  // bottom-left
            App_Vertex{32.0f, 32.0f, 0.0f, 1.0f, 1.0f}, // bottom-right
            App_Vertex{32.0f, 0.0f, 0.0f, 1.0f, 0.0f},  // top-right
            App_Vertex{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}    // top-left
    };

    std::array<uint16_t, 6> indexData = {
            0, 1, 2, 0, 2, 3
    };

    vertices = std::vector<App_Vertex>(vertexData.begin(), vertexData.end());
    indices = std::vector<uint16_t>(indexData.begin(), indexData.end());

    memVertices = bgfx::copy(vertices.data(), sizeof(App_Vertex) * vertices.size());
    vb = bgfx::createVertexBuffer(memVertices, s_layout);

    memIndices = bgfx::copy(indices.data(), sizeof(uint16_t) * indices.size());
    ib = bgfx::createIndexBuffer(memIndices);

    s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

    std::unique_ptr<char, CharDeleter> vs_texture(
    App_GetAssetPath(R"(assets\shaders\directx11\vs_texture)", "bin")
    );

    if (!vs_texture) {
        std::cerr << "Failed to load vertex shader from assets\n";
        return 1;  // Or handle the error appropriately
    }

    std::unique_ptr<char, CharDeleter> fs_texture(
        App_GetAssetPath(R"(assets\shaders\directx11\fs_texture)", "bin")
    );

    if (!fs_texture) {
        std::cerr << "Failed to load fragment shader from assets\n";
        return 1;  // Or handle the error appropriately
    }

    vertexShader = App_LoadShader(vs_texture.get());
    fragmentShader = App_LoadShader(fs_texture.get());

    // Load the compiled vertex shader
    vsHandle = bgfx::createShader(vertexShader);
    if (!bgfx::isValid(vsHandle)) {
        // Handle the error
        std::cerr << "Vertex shader not valid\n";
    }

    // Load the compiled fragment shader
    fsHandle = bgfx::createShader(fragmentShader);
    if (!bgfx::isValid(fsHandle)) {
        // Handle the error
        std::cerr << "Fragment shader not valid\n";
    }

    // Create the program from the shaders
    program = bgfx::createProgram(vsHandle, fsHandle, true);

    if (!bgfx::isValid(program)) {
        // Handle the error
        std::cerr << "Program not valid\n";
    }

    std::unique_ptr<char, CharDeleter> path(
        App_GetAssetPath(R"(assets\wabbit_alpha)", "png")
    );

    bunnyTexture = App_LoadTexture(path.get(), &surface, &windowWidth, &windowHeight);

    //------------------------------------------------------------------------------------------------------------------
    //-- 3) Render Loop
    //------------------------------------------------------------------------------------------------------------------
    done = 0;
    while (!done) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
        }

        //-- Add 1 sprite every frame until there are 100 sprites
        for (int i = 0; i < 1; ++i) {
            if (sprites.size() < 100) {
                App_Sprite sprite = {
                        0,
                        {windowWidth / 2., windowHeight / 2., 0.0},
                        {1.0, 1.0, 1.0},
                        {32.0, 32.0},
                        {0.0, 0.0, 0.0},
                        {255, 255, 255, 255},
                        {
                                App_GetRandomFloat(-250.f, 250.f) / 60.f,
                                App_GetRandomFloat(-250.f, 250.f) / 60.f
                        }
                };

                sprites[{sprites.size() + 1, 0}] = sprite;
            }
        }


        for (auto &pair: sprites) {
            auto &sprite = pair.second;

            sprite.position[0] += sprite.speed[0];
            sprite.position[1] += sprite.speed[1];

            if ((sprite.position[0] + (sprite.size[0] / 2) + 16) > windowWidth ||
                (sprite.position[0] - (sprite.size[0] / 2) + 16) < 0) {
                sprite.speed[0] *= -1;
            }

            if ((sprite.position[1] + (sprite.size[1] / 2) + 16) > windowHeight ||
                (sprite.position[1] - (sprite.size[1] / 2) + 16) < 0) {
                sprite.speed[1] *= -1;
            }
        }

        bgfx::setViewRect(0, 0, 0, windowWidth, windowHeight);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

        bx::mtxOrtho(proj, 0, (float) windowWidth, (float) windowHeight, 0, -100, 100, 0.0f,
                     bgfx::getCaps()->homogeneousDepth);
        bx::mtxIdentity(view);
        bgfx::setViewTransform(0, view, proj);

        for (const auto &pair: sprites) {
            const auto &sprite = pair.second;

            auto x = (float) sprite.position[0];
            auto y = (float) sprite.position[1];
            float z = -60.f;

            std::array<float, 16> instanceTransform = {
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    x, y, z, 1.0f
            };

            bgfx::setTransform(&instanceTransform);

            bgfx::setVertexBuffer(0, vb);
            bgfx::setIndexBuffer(ib);

            bgfx::setTexture(0, s_texColor, bunnyTexture);

            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_ALPHA | BGFX_STATE_DEPTH_TEST_LESS);

            bgfx::submit(0, program, 0, BGFX_DISCARD_ALL);
        }

        bgfx::touch(0);

        // Increment frame counter
        frameCount++;

        // Check time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime);

        if (elapsedTime.count() >= 1) {
            std::cout << "FPS: " << frameCount << std::endl;
            std::cout << "Sprites: " << sprites.size() << std::endl;

            // Reset frame counter and lastTime
            frameCount = 0;
            lastTime = currentTime;
        }

        bgfx::frame();
    }

    //------------------------------------------------------------------------------------------------------------------
    //-- 4) Shutdown
    //------------------------------------------------------------------------------------------------------------------
    bgfx::destroy(s_texColor);

    if (bgfx::isValid(vb)) {
        bgfx::destroy(vb);
    }

    if (bgfx::isValid(ib)) {
        bgfx::destroy(ib);
    }

    if (bgfx::isValid(program)) {
        bgfx::destroy(program);
    }

    if (bgfx::isValid(vsHandle)) {
        bgfx::destroy(vsHandle);
    }

    if (bgfx::isValid(fsHandle)) {
        bgfx::destroy(fsHandle);
    }

    if (bgfx::isValid(bunnyTexture)) {
        bgfx::destroy(bunnyTexture);
    }

    bgfx::shutdown();

    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window.get());
    SDL_Quit();

    return 0;
}

std::filesystem::path App_GetExecutablePath() {
    TCHAR buffer[MAX_PATH] = {0};
    GetModuleFileName(GetModuleHandle(nullptr), buffer, MAX_PATH);
    return std::filesystem::canonical(buffer);
}

char *App_GetAssetPath(const char *assetName, const char *ofType) {
    std::filesystem::path execPath = App_GetExecutablePath();
    std::string pathStr = (execPath.parent_path() / (std::string(assetName) + "." + ofType)).string();
    char *pathCStr = new char[pathStr.length() + 1];
    strcpy_s(pathCStr, pathStr.length() + 1, pathStr.c_str());
    return pathCStr;
}

float App_GetRandomFloat(float min, float max) {
    static std::random_device rd;  // Random device engine, usually based on /dev/urandom on UNIX-like systems
    static std::mt19937 rng(rd());  // Using the Mersenne Twister for random number generation
    std::uniform_real_distribution<float> dist(min, max);  // Distribution in range [min, max]

    return dist(rng);
}

uint8_t *App_LoadFileToMemory(const char *filename, uint32_t *size) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return nullptr;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate buffer
    auto *buffer = new(std::nothrow) uint8_t[fileSize];
    if (!buffer) {
        return nullptr;
    }

    // Read the file into buffer
    if (!file.read(reinterpret_cast<char *>(buffer), fileSize)) {
        delete[] buffer;
        return nullptr;
    }

    *size = static_cast<uint32_t>(fileSize);
    return buffer;
}

const bgfx::Memory *App_LoadShader(const char *filename) {
    uint32_t size;
    uint8_t *shaderData = App_LoadFileToMemory(filename, &size);

    if (shaderData == nullptr) {
        return nullptr; // or handle the error appropriately
    }

    return bgfx::makeRef(shaderData, size, [](void *_data, void *_userData) {
        delete[] reinterpret_cast<uint8_t *>(_data);
    });
}

bgfx::TextureHandle App_LoadTexture(const char *file, SDL_Surface **pSurface, uint32_t *widthOut, uint32_t *heightOut) {
    *pSurface = IMG_Load(file);
    if (!*pSurface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return BGFX_INVALID_HANDLE;
    }

    uint32_t surfaceWidth = (*pSurface)->w;
    uint32_t surfaceHeight = (*pSurface)->h;
    uint32_t pitch = (*pSurface)->pitch;
    const auto *imagePixels = (const uint8_t *) (*pSurface)->pixels;

    const bgfx::Memory *mem = bgfx::makeRef(imagePixels, pitch * surfaceHeight);

    if (widthOut) *widthOut = surfaceWidth;
    if (heightOut) *heightOut = surfaceHeight;

    return bgfx::createTexture2D(surfaceWidth, surfaceHeight, false, 1, bgfx::TextureFormat::RGBA8, 0, mem);
}

void App_PrintInitDetails(const bgfx::Init &bgInit) {
    std::cout
            << "init.type              = " << bgInit.type << std::endl
            << "init.vendorId          = " << bgInit.vendorId << std::endl
            << "init.platformData.nwh  = " << bgInit.platformData.nwh << std::endl
            << "init.platformData.ndt  = " << bgInit.platformData.ndt << std::endl
            << "init.platformData.type = " << bgInit.platformData.type << std::endl
            << "init.resolution.width  = " << bgInit.resolution.width << std::endl
            << "init.resolution.height = " << bgInit.resolution.height << std::endl
            << "init.resolution.reset  = " << bgInit.resolution.reset << std::endl
            << "init.resolution.format  = " << bgInit.resolution.format << std::endl;
}