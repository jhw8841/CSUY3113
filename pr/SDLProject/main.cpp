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

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, paddleOneMatrix, paddleTwoMatrix, ballMatrix, projectionMatrix;

glm::vec3 player_one_position = glm::vec3(-4.5, 0, 0);
glm::vec3 player_two_position = glm::vec3(4.5, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);

glm::vec3 player_one_movement = glm::vec3(0, 0, 0);
glm::vec3 player_two_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

float player_speed = 5.0f;
float ball_speed = 2.0f;

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Speed Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

    viewMatrix = glm::mat4(1.0f);
    paddleOneMatrix = glm::mat4(1.0f);
    paddleTwoMatrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(0.8f, 0.8f, 0.8f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

bool start = false;
void ProcessInput() {
    player_one_movement = glm::vec3(0);
    player_two_movement = glm::vec3(0);
    ball_movement.x = 1;
    //ball_movement.y = 1;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    //ball_movement.x = 1;
                    //ball_movement.y = 1;
                    break;
                }
                break; // SDL_KEYDOWN
            }
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W] && player_one_position.y <= 3.0) {
        player_one_movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_S] && player_one_position.y >= -3.0) {
        player_one_movement.y = -1.0f;
    }

    if (keys[SDL_SCANCODE_UP] && player_two_position.y <= 3.0) {
        player_two_movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_DOWN] && player_two_position.y >= -3.0) {
        player_two_movement.y = -1.0f;
    }

    if (glm::length(player_one_movement) > 1.0f) {
        player_one_movement = glm::normalize(player_one_movement);
    }
    if (glm::length(player_two_movement) > 1.0f) {
        player_two_movement = glm::normalize(player_two_movement);
    }
}

bool detectBallCollision() {
    return true;
}

float lastTicks = 0.0f;
void Update() { 
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    //Test collision Detection
        
    //End collision detection testing

    //Update position
    player_one_position += player_one_movement * player_speed * deltaTime;
    player_two_position += player_two_movement * player_speed * deltaTime;
    ball_position += ball_movement * ball_speed * deltaTime;

    paddleOneMatrix = glm::mat4(1.0f);
    paddleOneMatrix = glm::translate(paddleOneMatrix, player_one_position);

    paddleTwoMatrix = glm::mat4(1.0f);
    paddleTwoMatrix = glm::translate(paddleTwoMatrix, player_two_position);

    ballMatrix = glm::mat4(1.0f);
    ballMatrix = glm::translate(ballMatrix, ball_position);
}

void drawPaddleOne() {
    program.SetModelMatrix(paddleOneMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawPaddleTwo() {
    program.SetModelMatrix(paddleTwoMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawBall() {
    program.SetModelMatrix(ballMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableVertexAttribArray(program.positionAttribute);

    float vertices[] = { -0.10, -0.75, 0.10, -0.75, 0.10, 0.75, -0.10, -0.75, 0.10, 0.75, -0.10, 0.75};
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    drawPaddleOne();
    drawPaddleTwo();

    float ballVertices[] = { -0.15, -0.15, 0.15, -0.15, 0.15, 0.15, -0.15, -0.15, 0.15, 0.15, -0.15, 0.15 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
    drawBall();

    glDisableVertexAttribArray(program.positionAttribute);

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