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
bool gameEnded = false;

ShaderProgram program;
glm::mat4 viewMatrix, paddleOneMatrix, paddleTwoMatrix, ballMatrix, projectionMatrix;

glm::vec3 paddle_one_position = glm::vec3(-4.5, 0, 0);
glm::vec3 paddle_two_position = glm::vec3(4.5, 0, 0);
glm::vec3 ball_position = glm::vec3(0, 0, 0);

glm::vec3 player_one_movement = glm::vec3(0, 0, 0);
glm::vec3 player_two_movement = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

float player_speed = 3.0f;
float ball_speed = 3.0f;

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

void ProcessInput() {
    player_one_movement = glm::vec3(0);
    player_two_movement = glm::vec3(0);

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
                gameEnded = false;
                paddle_one_position.y = 0.0f;
                paddle_two_position.y = 0.0f;
                ball_position.x = 0.0f;
                ball_position.y = 0.0f;
                ball_movement.x = 1.0f;
                ball_movement.y = -1.0f;
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    if (gameEnded == false) {
        //Paddles have a radius height of 0.75. The edge is 3.75/-3.75. 3.75 - 0.75 = 3.0. Use 3.0 to prevent paddles from going out of the edge of map
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_W] && paddle_one_position.y < 3.0f) {
            player_one_movement.y = 1.0f;
        }
        else if (keys[SDL_SCANCODE_S] && paddle_one_position.y > -3.0f) {
            player_one_movement.y = -1.0f;
        }

        if (keys[SDL_SCANCODE_UP] && paddle_two_position.y < 3.0f) {
            player_two_movement.y = 1.0f;
        }
        else if (keys[SDL_SCANCODE_DOWN] && paddle_two_position.y > -3.0f) {
            player_two_movement.y = -1.0f;
        }

        if (glm::length(player_one_movement) > 1.0f) {
            player_one_movement = glm::normalize(player_one_movement);
        }
        if (glm::length(player_two_movement) > 1.0f) {
            player_two_movement = glm::normalize(player_two_movement);
        }
    }
}

void detectPaddleCollision() { //During collisions with wall/paddle, we bring the ball back the distance it penetrated + 0.0001 to make sure it's not inside paddle/wall
    float paddleWidth = 0.10f, paddleHeight = 0.75f;
    float ballWidth = 0.15f, ballHeight = 0.15f;

    float paddleTwoXDiff = fabs(paddle_two_position.x - ball_position.x);
    float paddleTwoYDiff = fabs(paddle_two_position.y - ball_position.y);

    float paddleOneXDiff = fabs(paddle_one_position.x - ball_position.x);
    float paddleOneYDiff = fabs(paddle_one_position.y - ball_position.y);

    float paddleTwoXDistance = paddleTwoXDiff - (paddleWidth + ballWidth);
    float paddleTwoYDistance = paddleTwoYDiff - (paddleHeight + ballHeight);

    float paddleOneXDistance = paddleOneXDiff - (paddleWidth + ballWidth);
    float paddleOneYDistance = paddleOneYDiff - (paddleHeight + ballHeight);

    if (paddleOneXDistance < 0.0f && paddleOneYDistance < 0.0f) { //Ball is hitting left paddle
        if (paddleOneXDistance < -0.005f) {//Ball is hitting the side
            float diagonal = ((fabs(paddleOneXDistance) * fabs(paddleOneXDistance)) + (fabs(paddleOneYDistance) * fabs(paddleOneYDistance))); //Not doing sqrt looks more natural
            if (ball_position.y > paddle_one_position.y) { //if the ball did hit the side, which side did it hit
                ball_position.y += diagonal + 0.0001f;
            }
            else if (ball_position.y <= paddle_one_position.y) {
                ball_position.y -= diagonal - 0.0001f;
            }
            ball_position.x += diagonal + 0.0001f;
            ball_movement.x *= -1.0f;
            ball_movement.y *= -1.0f;
        }
        else { //Ball is hitting the front
            ball_position.x += (fabs(paddleOneXDistance) + 0.0001f);
            ball_movement.x *= -1.0f; //if the ball collides with a paddle, bounce it back in the x direction
        }
    }
    else if (paddleTwoXDistance < 0.0f && paddleTwoYDistance < 0.0f) { //Ball is hitting right paddle
        if (paddleTwoXDistance < -0.005f) { //Ball is hitting the side
            float diagonal = ((fabs(paddleTwoXDistance) * fabs(paddleTwoXDistance)) + (fabs(paddleTwoYDistance) * fabs(paddleTwoYDistance)));
            if (ball_position.y > paddle_two_position.y) { //which side is getting hit
                ball_position.y += diagonal + 0.0001f;
            }
            else if (ball_position.y <= paddle_two_position.y) {
                ball_position.y -= diagonal - 0.0001f;
            }
            ball_position.x -= diagonal - 0.0001f;
            ball_movement.x *= -1.0f;
            ball_movement.y *= -1.0f;
        }
        else { //Ball is hitting the front
            ball_position.x -= (fabs(paddleTwoXDistance) + 0.0001f);
            ball_movement.x *= -1.0f;
        }
    }
}

void detectWallCollision() { //If the ball hits the Y wall, bounce it back in the other y direction
    float ballPen;
    if (ball_position.y >= 3.6f) {
        ballPen = ball_position.y - 3.6f;
        ball_position.y -= (fabs(ballPen) + 0.0001f);
        ball_movement.y *= -1.0f;
    }
    else if (ball_position.y <= -3.6f) {
        ballPen = ball_position.y + 3.6f;
        ball_position.y += (fabs(ballPen) + 0.0001f);
        ball_movement.y *= -1.0f;
    }
}

void outOfBounds() { //When ball goes out of bounds/hits left or right wall, someone won, stop game
    if (ball_position.x <= -4.85f || ball_position.x >= 4.85f) {
        gameEnded = true;
        ball_movement = glm::vec3(0);
    }
}

float lastTicks = 0.0f;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    if (gameEnded == false) {
        //Test collision Detection
        detectPaddleCollision();
        detectWallCollision();

        //Update position
        paddle_one_position += player_one_movement * player_speed * deltaTime;
        paddle_two_position += player_two_movement * player_speed * deltaTime;
        ball_position += ball_movement * ball_speed * deltaTime;

        paddleOneMatrix = glm::mat4(1.0f);
        paddleOneMatrix = glm::translate(paddleOneMatrix, paddle_one_position);

        paddleTwoMatrix = glm::mat4(1.0f);
        paddleTwoMatrix = glm::translate(paddleTwoMatrix, paddle_two_position);

        ballMatrix = glm::mat4(1.0f);
        ballMatrix = glm::translate(ballMatrix, ball_position);

        outOfBounds();
    }
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

    float vertices[] = { -0.10, -0.75, 0.10, -0.75, 0.10, 0.75, -0.10, -0.75, 0.10, 0.75, -0.10, 0.75 };
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