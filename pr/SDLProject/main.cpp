#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#define PLATFORM_COUNT 36
#define ENEMY_COUNT 3

struct GameState {
    Entity* player;
    Entity* platforms;
    Entity* enemies;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Textured!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.8588f, 0.9137f, 0.9569f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Initialize Game Objects
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(-4, -2, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->speed = 2.0f;
    state.player->textureID = LoadTexture("playerBlue_walk2.png");
    state.player->height = 0.48f;
    state.player->width = 0.15f;
    state.player->jumpPower = 5.75;

    /*
    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };
    state.player->animUp = new int[4]{ 2, 6, 10, 14 };
    state.player->animDown = new int[4]{ 0, 4, 8, 12 };
    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;
    */

    state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platformTextureID = LoadTexture("tilesheet_complete.png");

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTextureID;
        state.platforms[i].width = 0.5f;
        state.platforms[i].height = 0.5f;
        state.platforms[i].animRows = 12;
        state.platforms[i].animCols = 22;
        state.platforms[i].animFrames = 1;
    }

    //Ground Platform
    state.platforms[0].position = glm::vec3(-4.75f, -3.5f, 0);
    state.platforms[0].animIndices = new int[1]{ 133 };
    state.platforms[19].position = glm::vec3(-4.75f + (19 * 0.5), -3.5f, 0);
    state.platforms[19].animIndices = new int[1]{ 135 };
    for (int i = 1; i < PLATFORM_COUNT - 17; i++) {
        state.platforms[i].position = glm::vec3(-4.75f + (i*0.5), -3.5f, 0);
        state.platforms[i].animIndices = new int[1]{ 134 };
    }

    //Low-Mid Platform
    int platInc = 0; //Platform Incrementer
    state.platforms[20].position = glm::vec3(-1.25f, -2.0f, 0);
    state.platforms[20].animIndices = new int[1]{ 161 };
    for (int i = 21; i < PLATFORM_COUNT - 11; i++) {
        state.platforms[i].position = glm::vec3(-0.75f + (platInc * 0.5), -2.0f, 0);
        state.platforms[i].animIndices = new int[1]{ 134 };
        ++platInc;
    }
    state.platforms[25].position = glm::vec3(-0.75f + (platInc * 0.5), -2.0f, 0);
    state.platforms[25].animIndices = new int[1]{ 162 };

    //Mid-Right Triangular Platform
    state.platforms[26].position = glm::vec3(3.25f, -1.0f, 0);
    state.platforms[26].animIndices = new int[1]{ 183 };
    state.platforms[27].position = glm::vec3(3.75f, -1.0f, 0);
    state.platforms[27].animIndices = new int[1]{ 184 };

    //Mid-High Triangular 1
    state.platforms[28].position = glm::vec3(1.25f, 0.5f, 0);
    state.platforms[28].animIndices = new int[1]{ 183 };
    state.platforms[29].position = glm::vec3(1.75f, 0.5f, 0);
    state.platforms[29].animIndices = new int[1]{ 184 };
    //Mid-High Triangular 2
    state.platforms[30].position = glm::vec3(-1.75f, 0.5f, 0);
    state.platforms[30].animIndices = new int[1]{ 183 };
    state.platforms[31].position = glm::vec3(-1.25f, 0.5f, 0);
    state.platforms[31].animIndices = new int[1]{ 184 };

    //Left-High Platform 
    state.platforms[32].position = glm::vec3(-4.75f, 1.5f, 0);
    state.platforms[32].animIndices = new int[1]{ 161 };
    platInc = 0;
    for (int i = 33; i < PLATFORM_COUNT - 1; i++) {
        state.platforms[i].position = glm::vec3(-4.25f + (platInc * 0.5), 1.5f, 0);
        state.platforms[i].animIndices = new int[1]{ 134 };
        ++platInc;
    }
    state.platforms[35].position = glm::vec3(-4.25f + (platInc * 0.5), 1.5f, 0);
    state.platforms[35].animIndices = new int[1]{ 162 };

    //Update platforms to initialized positions
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Update(0, NULL, NULL, 0);
    }

    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemyTextureID = LoadTexture("enemyFloating_1.png"); //Placeholder image

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].entityType = ENEMY;
    }

    state.enemies[0].entityType = ENEMY;
    state.enemies[0].textureID = enemyTextureID;
    state.enemies[0].position = glm::vec3(4.25f, -3.0f, 0);
    state.enemies[0].movement = glm::vec3(-1, 0, 0);
    state.enemies[0].speed = 1;
    state.enemies[0].height = 0.5f;
    state.enemies[0].width = 0.5f;
    state.enemies[0].aiType = WALKER;

    state.enemies[1].entityType = ENEMY;
    state.enemies[1].textureID = enemyTextureID;
    state.enemies[1].position = glm::vec3(-4.75f, 2.0f, 0);
    state.enemies[1].speed = 1;
    state.enemies[1].height = 0.5f;
    state.enemies[1].width = 0.5f;
    state.enemies[1].aiType = SLAMMER;
    state.enemies[1].aiState = IDLE;
}

void ProcessInput() {

    state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                break;

            case SDLK_SPACE:
                if (state.player->collidedBottom == true) {
                    state.player->jump = true;
                }
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        //state.player->animIndices = state.player->animLeft;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        //state.player->animIndices = state.player->animRight;
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        state.player->Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT);

        for (int i = 0; i < ENEMY_COUNT - 1; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT);
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Render(&program);
    }
    for (int i = 0; i < ENEMY_COUNT - 1; i++) {
        state.enemies[i].Render(&program);
    }

    state.player->Render(&program);

    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}