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

enum EntityType { PLAYER, PLATFORM, WALL};

class Entity {
public:
    EntityType entityType;

    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;

    //adjustable hitbox values
    double width = 1;
    double height = 1;

    float speed;

    GLuint textureID;

    glm::mat4 modelMatrix;

    //Rendering vertices
    float* entityVertices;
    float* entityTexCoords;

    //Left these in, tried to add in directional animation, couldn't find/properly make animations
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

    Entity();

    bool CheckCollision(Entity* other, int otherCount);

    void Update(float deltaTime, int* gameState, Entity* platform, Entity* walls, int WALL_COUNT);
    void Render(ShaderProgram* program);
    void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
};
