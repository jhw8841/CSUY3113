#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#include <vector>
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
#define ENEMY_COUNT 4

struct GameState {
    Entity* player;
    Entity* platforms;
    Entity* enemies;
    Entity* sword;
};

GameState state;
int gameStatus = 1; //1 = game ongoing, 0 = lose, 2 = win

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint fontTextureID;
GLuint playerTextures[2];
GLuint swordTextures[2];
int remainingEnemies = ENEMY_COUNT;

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

    fontTextureID = LoadTexture("font1.png");

    // Initialize Game Objects
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->startingPosition = glm::vec3(-4, -2, 0);
    state.player->position = state.player->startingPosition;
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->speed = 2.0f;
    state.player->height = 0.48f;
    state.player->width = 0.25f;
    state.player->jumpPower = 5.75;
    playerTextures[0] = LoadTexture("blueWalkLeft.png");
    playerTextures[1] = LoadTexture("playerBlue_walk3.png");
    state.player->textureID = playerTextures[0];

    state.sword = new Entity();
    state.sword->entityType = SWORD;
    state.sword->position = glm::vec3(state.player->position.x + 0.33, state.player->position.y, 0);
    state.sword->height = 0.1f;
    state.sword->width = 0.25;
    state.sword->animTime = 0.0f;
    state.sword->isActive = false;
    swordTextures[0] = LoadTexture("swordFlip.png");
    swordTextures[1] = LoadTexture("sword.png");
    state.sword->textureID = swordTextures[0];

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
        state.platforms[i].Update(0, NULL, NULL, NULL, NULL, NULL, 0);
    }

    state.enemies = new Entity[ENEMY_COUNT];
    GLuint walkerTextureID = LoadTexture("enemySwimmerRotated.png");
    GLuint slammerTextureID = LoadTexture("enemyFloating_1.png");
    GLuint chaserTextureID = LoadTexture("enemyFloating_3.png");

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].height = 0.5f;
        state.enemies[i].width = 0.5f;
    }

    state.enemies[0].textureID = walkerTextureID;
    state.enemies[0].startingPosition = glm::vec3(4.25f, -3.0f, 0);
    state.enemies[0].position = state.enemies[0].startingPosition;
    state.enemies[0].movement = glm::vec3(-1, 0, 0);
    state.enemies[0].acceleration = glm::vec3(0, -9.81f, 0);
    state.enemies[0].aiType = WALKER;
    state.enemies[0].speed = 1.0f;
    state.enemies[0].turningLeft = glm::vec3(4.25f, 0, 0);
    state.enemies[0].turningRight = glm::vec3(-4.25f, 0, 0);

    state.enemies[1].textureID = walkerTextureID;
    state.enemies[1].startingPosition = glm::vec3(1.25f, -1.5f, 0);
    state.enemies[1].position = state.enemies[1].startingPosition;
    state.enemies[1].movement = glm::vec3(-1, 0, 0);
    state.enemies[1].acceleration = glm::vec3(0, -9.81f, 0);
    state.enemies[1].aiType = WALKER;
    state.enemies[1].speed = 1.0f;
    state.enemies[1].turningLeft = glm::vec3(1.25f, 0, 0);
    state.enemies[1].turningRight = glm::vec3(-1.25f, 0, 0);

    state.enemies[2].textureID = slammerTextureID;
    state.enemies[2].startingPosition = glm::vec3(-4.75f, 2.0f, 0);
    state.enemies[2].position = state.enemies[2].startingPosition;
    state.enemies[2].aiType = SLAMMER;
    state.enemies[2].aiState = IDLE;
    state.enemies[2].speed = 1.0f;
    state.enemies[2].turningLeft = glm::vec3(-2.25f, 0, 0);

    state.enemies[3].textureID = chaserTextureID;
    state.enemies[3].startingPosition = glm::vec3(3.5f, 1.0f, 0);
    state.enemies[3].position = state.enemies[3].startingPosition;
    state.enemies[3].aiType = CHASER;
    state.enemies[3].aiState = IDLE;
    state.enemies[3].speed = 0.5f;
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
            case SDLK_x:
                if (state.sword->isActive == false && gameStatus == 1) {
                    state.sword->isActive = true;
                    if (state.player->direction == LEFT) {
                        state.sword->textureID = swordTextures[0];
                    }
                    else if (state.player->direction == RIGHT) {
                        state.sword->textureID = swordTextures[1];
                    }
                }
                break;
            case SDLK_SPACE:
                if (state.player->collidedBottom == true) {
                    state.player->jump = true;
                }
                break;
            case SDLK_r:
                gameStatus = 1;
                state.player->position = state.player->startingPosition;
                state.player->velocity = glm::vec3(0);
                state.sword->isActive = false;

                remainingEnemies = ENEMY_COUNT;
                for (int i = 0; i < ENEMY_COUNT; i++) {
                    state.enemies[i].isActive = true;
                    state.enemies[i].position = state.enemies[i].startingPosition;
                }
                state.enemies[2].movement = glm::vec3(0);
                state.enemies[2].aiState = IDLE;
                state.enemies[3].movement = glm::vec3(0);
                state.enemies[3].velocity = glm::vec3(0);
                state.enemies[3].aiState = IDLE;
            }
            break;
        }
    }

    if (gameStatus == 1) {
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT]) {
            state.player->movement.x = -1.0f;
            if (state.sword->isActive == false) {
                state.player->direction = LEFT;
                state.player->textureID = playerTextures[0];
            }
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            state.player->movement.x = 1.0f;
            if (state.sword->isActive == false) {
                state.player->direction = RIGHT;
                state.player->textureID = playerTextures[1];
            }
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

    if (gameStatus == 1) {
        deltaTime += accumulator;
        if (deltaTime < FIXED_TIMESTEP) {
            accumulator = deltaTime;
            return;
        }

        while (deltaTime >= FIXED_TIMESTEP) {
            state.player->Update(FIXED_TIMESTEP, &gameStatus, &remainingEnemies, state.player, state.sword, state.platforms, PLATFORM_COUNT);
            state.sword->Update(FIXED_TIMESTEP, &gameStatus, &remainingEnemies, state.player, state.sword, state.platforms, PLATFORM_COUNT);

            for (int i = 0; i < ENEMY_COUNT; i++) {
                state.enemies[i].Update(FIXED_TIMESTEP, &gameStatus, &remainingEnemies, state.player, state.sword, state.platforms, PLATFORM_COUNT);
            }

            deltaTime -= FIXED_TIMESTEP;
        }
    }
    accumulator = deltaTime;
    if (remainingEnemies <= 0) {
        gameStatus = 2;
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

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].Render(&program);
    }
    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].Render(&program);
    }

    state.player->Render(&program);
    state.sword->Render(&program);

    if (gameStatus == 2) {
        DrawText(&program, fontTextureID, "Victory!", 1.0f, -0.5f, glm::vec3(-1.65f, 1.0f, 0));
    }
    else if (gameStatus == 0) {
        DrawText(&program, fontTextureID, "Defeat!", 1.0f, -0.5f, glm::vec3(-1.4f, 1.0f, 0));
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