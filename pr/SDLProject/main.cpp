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

#include <vector>
#include "Entity.h"

#define WALL_COUNT 5

struct GameState {
    Entity* player;
    Entity* platform;
    Entity* wall;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;
int gameState = 1; //1 = game currently in progress, 0 = mission failed, 2 = mission successful

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint fontTextureID;

int boosterFuel = 10000;

//Vertices Coordinates
float playerVertices[] = { -0.3, -0.3, 0.3, -0.3, 0.3, 0.3, -0.3, -0.3, 0.3, 0.3, -0.3, 0.3 };
float playerTexCoord[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
float platVertices[] = { -2.0, -0.10, 2.0, -0.10, 2.0, 0.10, -2.0, -0.10, 2.0, 0.10, -2.0, 0.10 };
float platTexCoord[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
float wallVertices[] = { -5.0, -0.5, 5.0, -0.5, 5.0, 0.5, -5.0, -0.5, 5.0, 0.5, -5.0, 0.5 };
float wallTexCoord[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
float sideWallVertices[] = { -0.25, -3.25, 0.25, -3.25, 0.25, 3.25, -0.25, -3.25, 0.25, 3.25, -0.25, 3.25 };
float sideWallTexCoord[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
float platformWallVertices[] = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
float platformWallTexCoord[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

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

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    fontTextureID = LoadTexture("font1.png");

    // Initialize Game Objects
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(3.85f, 3.4f, 0);
    state.player->speed = 1.0f;
    state.player->acceleration = glm::vec3(0, -0.0015f, 0);
    state.player->textureID = LoadTexture("ship.png"); //Ship
    state.player->entityVertices = playerVertices;
    state.player->entityTexCoords = playerTexCoord;
    state.player->height = 0.3;
    state.player->width = 0.3;

    state.platform = new Entity();
    state.platform->entityType = PLATFORM;
    GLuint platformTextureID = LoadTexture("platformPack_tile017.png"); //Land on the water patch to win
    state.platform->textureID = platformTextureID;
    state.platform->position = glm::vec3(2, -2.85f, 0);
    state.platform->entityVertices = platVertices;
    state.platform->entityTexCoords = platTexCoord;
    state.platform->width = 3.15;
    state.platform->height = 0.5;
    state.platform->Update(0, NULL, NULL, NULL, 0);

    state.wall = new Entity[WALL_COUNT];

    for (int i = 0; i < WALL_COUNT; i++) {
        state.wall[i].entityType = WALL;
    }

    GLuint wallTextureID = LoadTexture("platformPack_tile016.png"); //Moon rock, you lose if you land/crash on the moon rock
    state.wall[0].textureID = wallTextureID;
    state.wall[0].position = glm::vec3(0, -3.25f, 0);
    state.wall[0].width = 9.0;
    state.wall[0].height = 1.30;
    state.wall[0].entityVertices = wallVertices;
    state.wall[0].entityTexCoords = wallTexCoord;

    GLuint sideWallTextureID = LoadTexture("platformPack_tile016.png");
    for (int i = 1; i < WALL_COUNT; i++) {
        state.wall[i].textureID = sideWallTextureID;
    }
    
    state.wall[1].position = glm::vec3(-4.75f, 0.5f, 0);
    state.wall[1].entityVertices = sideWallVertices;
    state.wall[1].entityTexCoords = sideWallTexCoord;
    state.wall[1].width = 0.8;
    state.wall[1].height = 7.0;

    state.wall[2].position = glm::vec3(4.75f, 0.5f, 0);
    state.wall[2].entityVertices = sideWallVertices;
    state.wall[2].entityTexCoords = sideWallTexCoord;
    state.wall[2].width = 0.8;
    state.wall[2].height = 7.0;

    state.wall[3].position = glm::vec3(2.0f, 1.4f, 0);
    state.wall[3].entityVertices = platformWallVertices;
    state.wall[3].entityTexCoords = platformWallTexCoord;
    state.wall[3].width = 5.3;
    state.wall[3].height = 0.8;

    state.wall[4].position = glm::vec3(-2.0f, -1.25f, 0);
    state.wall[4].entityVertices = platformWallVertices;
    state.wall[4].entityTexCoords = platformWallTexCoord;
    state.wall[4].width = 5.3;
    state.wall[4].height = 0.8;

    for (int i = 0; i < WALL_COUNT; i++) {
        state.wall[i].Update(0, NULL, NULL, NULL, 0);
    }
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
            case SDLK_SPACE:
                gameState = 1;
                state.player->position = glm::vec3(3.85f, 3.4f, 0);
                state.player->acceleration = glm::vec3(0, -0.0015f, 0);
                state.player->velocity = glm::vec3(0, 0, 0);
                boosterFuel = 10000;
                break;
            }
            break;
        }
    }

    if (gameState == 1) {
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT] && state.player->acceleration.x > -5.0f) {
            state.player->acceleration.x += -0.0025f;
        }
        else if (keys[SDL_SCANCODE_RIGHT] && state.player->acceleration.x < 5.0f) {
            state.player->acceleration.x += 0.0025f;
        }

        if (keys[SDL_SCANCODE_UP] && boosterFuel > 0 && state.player->acceleration.y < 0.002) {
            state.player->acceleration.y += 0.001f;
            boosterFuel -= 1;
        }

        if (glm::length(state.player->movement) > 1.0f) {
            state.player->movement = glm::normalize(state.player->movement);
        }
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    if (gameState == 1) {
        deltaTime += accumulator;
        if (deltaTime < FIXED_TIMESTEP) {
            accumulator = deltaTime;
            return;
        }

        while (deltaTime >= FIXED_TIMESTEP) {
            state.player->Update(FIXED_TIMESTEP, &gameState, state.platform, state.wall, WALL_COUNT);
            deltaTime -= FIXED_TIMESTEP;
        }
    }
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position) {
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;

        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;

        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });

        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });
    } // end of for loop

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    

    for (int i = 0; i < WALL_COUNT; i++) {
        state.wall[i].Render(&program);
    }
    state.platform->Render(&program);
    state.player->Render(&program);

    DrawText(&program, fontTextureID, "Booster Fuel:" + std::to_string(boosterFuel), 0.5f, -0.25f, glm::vec3(-4.7f, -3.45f, 0));

    if (gameState == 2) {
        DrawText(&program, fontTextureID, "Mission Successful", 1.0f, -0.5f, glm::vec3(-4.1f, 1.0f, 0));
    }
    else if (gameState == 0) {
        DrawText(&program, fontTextureID, "Mission Failed", 1.0f, -0.5f, glm::vec3(-3.4f, 1.0f, 0));
    }

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
