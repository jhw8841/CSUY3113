#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;

    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity* other, int otherCount) { //Check collisions, check for wall or platform first, since multiple walls
    if (other->entityType == WALL) {
        for (int i = 0; i < otherCount; i++) {
            Entity* object = &other[i];

            double xdist = fabs(position.x - object->position.x) - ((width + object->width) / 2.0);
            double ydist = fabs(position.y - object->position.y) - ((height + object->height) / 2.0);
            if (xdist < 0 && ydist < 0) {
                return true;
            }
        }
    }
    else {
        double xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0);
        double ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0);
        if (xdist < 0 && ydist < 0) {
            return true;
        }
    }
    return false;
}

void Entity::Update(float deltaTime, int* gameState, Entity* platform, Entity* walls, int WALL_COUNT) //Check for excess acceleration and collisions first, then animations and position/movement/speeds
{
    if (entityType == PLAYER) {
        if (acceleration.x > 0) {
            acceleration.x -= 0.0006f;
        }
        else if (acceleration.x < 0) {
            acceleration.x += 0.0006f;
        }
        if (acceleration.y > -0.00075f)
        {
            acceleration.y -= 0.00075f;
        }

        if (position.y > 4.1) {
            *gameState = 0;
        }
        if (CheckCollision(platform, 0) == true) {
            *gameState = 2;
        }
        else if (CheckCollision(walls, WALL_COUNT) == true) {
            *gameState = 0;
        }
    }

    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else {
            animIndex = 0;
        }
    }

    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    position.y += velocity.y * deltaTime;
    position.x += velocity.x * deltaTime;

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index) //not using texture atlas at the moment
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) {
    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, entityVertices);
    glEnableVertexAttribArray(program->positionAttribute);


    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, entityTexCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
