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

enum EntityType {PLAYER, PLATFORM, ENEMY, SWORD};
enum Direction {NONE, LEFT, RIGHT};
//Walker walks between point A and point B, Slammer is a sideways stomper, when the player gets too close it'll quickly "stomp" sideways towards the player
//The Chaser activates when the player gets too close, and starts to follow the player indefinitely until it dies
enum AIType {WALKER, SLAMMER, CHASER};
enum AIState {IDLE, WALKING, ATTACKING};

class Entity {
public:

    EntityType entityType;
    Direction direction; //Direction of player, could also be used for AI direction
    AIType aiType;
    AIState aiState;

    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;
    //glm::vec3 sensorLeft;
    //glm::vec3 sensorRight;

    glm::vec3 startingPosition;
    glm::vec3 turningLeft; //Where an AI turns left
    glm::vec3 turningRight; //Where an AI turns right

    float width = 1;
    float height = 1;

    bool jump = false;
    float jumpPower = 0;

    float speed;

    GLuint textureID;

    glm::mat4 modelMatrix;

    int* animRight = NULL;
    int* animLeft = NULL;
    int* animUp = NULL;
    int* animDown = NULL;

    int* animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;

    bool isActive = true;
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;

    Entity();

    bool CheckCollision(Entity* other);
    //bool CheckSensorCollision(glm::vec3 sensor, Entity* object, int objectCount);
    void CheckCollisionsY(Entity* objects, int objectCount);
    void CheckCollisionsX(Entity* objects, int objectCount);

    void Update(float deltaTime, int* gameStatus, int* remainingEnemies, Entity* player, Entity* sword, Entity* platforms, int platformCount);
    void Render(ShaderProgram* program);
    void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);

    void AI(int* gameStatus, int* remainingEnemies, Entity* player, Entity* sword, Entity* platforms, int platformCount);
    void AIWalker(Entity* platforms, int platformCount);
    void AISlammer(Entity* player);
    void AIChaser(Entity* player);
};