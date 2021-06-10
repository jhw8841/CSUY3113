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

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, paddleOneMatrix, paddleTwoMatrix, ballMatrix, projectionMatrix;

//Paddle starting positions
glm::vec3 player_one_position = glm::vec3(-4.5, 0, 0);
glm::vec3 player_two_position = glm::vec3(4.5, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);

//Paddle movement
glm::vec3 player_one_movement = glm::vec3(0, 0, 0);
glm::vec3 player_two_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);
float player_speed = 1.5f;

//GLuint playerOneTextureID, playerTwoTextureID, ballTextureID;
/*
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
*/

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    paddleOneMatrix = glm::mat4(1.0f);
    paddleTwoMatrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 0.0f, 1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /*
    playerOneTextureID = LoadTexture("shroom.png");
    playerTwoTextureID = LoadTexture("shroom.png");
    ballTextureID = LoadTexture("circle.png");
    */
}

void ProcessInput() {
    player_one_movement = glm::vec3(0);
    player_two_movement = glm::vec3(0);
    ball_movement = glm::vec3(1);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W] && player_one_position.y <= 3.25) {
        player_one_movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_S] && player_one_position.y >= -3.25) {
        player_one_movement.y = -1.0f;
    }

    if (keys[SDL_SCANCODE_UP] && player_two_position.y <= 3.25) {
        player_two_movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_DOWN] && player_two_position.y >= -3.25) {
        player_two_movement.y = -1.0f;
    }

    if (glm::length(player_one_movement) > 1.0f) {
        player_one_movement = glm::normalize(player_one_movement);
    }
    if (glm::length(player_two_movement) > 1.0f) {
        player_two_movement = glm::normalize(player_two_movement);
    }
}

float lastTicks = 0.0f;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    //Update player position
    player_one_position += player_one_movement * player_speed * deltaTime;
    paddleOneMatrix = glm::mat4(1.0f);
    paddleOneMatrix = glm::translate(paddleOneMatrix, player_one_position);

    player_two_position += player_two_movement * player_speed * deltaTime;
    paddleTwoMatrix = glm::mat4(1.0f);
    paddleTwoMatrix = glm::translate(paddleTwoMatrix, player_two_position);
}

/*
void drawPaddleOne() {
    program.SetModelMatrix(paddleOneMatrix);
    glBindTexture(GL_TEXTURE_2D, playerOneTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawPaddleTwo() {
    program.SetModelMatrix(paddleTwoMatrix);
    glBindTexture(GL_TEXTURE_2D, playerTwoTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawBall() {
    program.SetModelMatrix(ballMatrix);
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
*/

void Render() {

    glClear(GL_COLOR_BUFFER_BIT);
    
    program.SetModelMatrix(paddleOneMatrix);
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_QUADS, 0, 3);
    glDisableVertexAttribArray(program.positionAttribute);

    SDL_GL_SwapWindow(displayWindow);
    /*
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glClear(GL_COLOR_BUFFER_BIT);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

    drawPaddleOne();
    drawPaddleTwo();
    drawBall();

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow);
    */
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