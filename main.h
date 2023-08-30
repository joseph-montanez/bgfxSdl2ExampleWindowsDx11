#ifndef BGFXSDL2EXAMPLEWINDOWSDX11_MAIN_H
#define BGFXSDL2EXAMPLEWINDOWSDX11_MAIN_H

#include <utility>
#include <unordered_map>
#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "bgfx/bgfx.h"

struct CharDeleter {
    void operator()(char* ptr) const {
        free(ptr);
    }
};

struct SDL_WindowDeleter {
    void operator()(SDL_Window* ptr) const {
        SDL_DestroyWindow(ptr);
    }
};

typedef struct {
    size_t textureId;
    double position[3];
    double scale[3];
    double size[2];
    double rotation[3];
    int64_t color[4];
    double speed[2];
} App_Sprite;

using App_Vertex = std::tuple<float, float, float, float, float>;

struct PairHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);  // hash the first element
        auto h2 = std::hash<T2>{}(p.second); // hash the second element

        // Combine the two hashes. This is a simple example. In practice, you might want a more complex combination.
        return h1 ^ h2;
    }
};

using SpriteMap = std::unordered_map<std::pair<uint64_t, uint64_t>, App_Sprite, PairHash>;

//-- Helper Functions
std::filesystem::path App_GetExecutablePath();
char* App_GetAssetPath(const char* assetName, const char* ofType);
float App_GetRandomFloat(float min, float max);
uint8_t* App_LoadFileToMemory(const char* filename, uint32_t* size);
const bgfx::Memory* App_LoadShader(const char* filename);
bgfx::TextureHandle App_LoadTexture(const char *file, SDL_Surface** pSurface, uint32_t *widthOut, uint32_t *heightOut);
void App_PrintInitDetails(const bgfx::Init& bgInit);

#endif //BGFXSDL2EXAMPLEWINDOWSDX11_MAIN_H
