#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;
    direction = NONE;

    sensorRight = glm::vec3(0);
    sensorLeft = glm::vec3(0);
    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity* other) {
    if (isActive == false || other->isActive == false) {
        return false;
    }
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0);

    if (xdist < 0 && ydist < 0) {
        return true;
    }
    return false;
}

bool Entity::CheckSensorCollision(glm::vec3 sensor, Entity* objects, int objectCount) {
    bool sensorConnecting = false;
    double xCoordOne, yCoordOne;
    double xCoordTwo, yCoordTwo;

    for (int i = 0; i < objectCount; i++) {
        Entity* object = &objects[i];
        xCoordOne = object->position.x - 0.25;
        yCoordOne = object->position.y + 0.25;

        xCoordTwo = object->position.x + 0.25;
        yCoordTwo = object->position.y - 0.25;

        if (sensor.x >= xCoordOne && sensor.x <= xCoordTwo && sensor.y <= yCoordOne && sensor.y >= yCoordTwo) {
            sensorConnecting = true;
        }
    }
    return sensorConnecting;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

void Entity::AIWalker(Entity* platforms, int platformCount) {
    /*
    if (CheckSensorCollision(sensorLeft, platforms, platformCount) == false) {
        movement = glm::vec3(-1, 0, 0);
    }
    else if (CheckSensorCollision(sensorRight, platforms, platformCount) == false) {
        movement = glm::vec3(1, 0, 0);
    }*/

    if (position.x >= 4.25f) {
        movement = glm::vec3(-1, 0, 0);
    }
    else if (position.x <= -4.25f) {
        movement = glm::vec3(1, 0, 0);
    }
}

void Entity::AISlammer(Entity* player) {
    switch (aiState) {
        case IDLE:
            if (glm::distance(position.x, player->position.x) < 3.0f && glm::distance(position.y, player->position.y) < 0.25f) {
                aiState = ATTACKING;
                speed = 5;
            }
            break;
        case WALKING:
            if (position.x <= -4.75f) {
                aiState = IDLE;
                speed = 0;
            }
            movement = glm::vec3(-1, 0, 0);
            break;
        case ATTACKING:
            if (position.x >= -2.25f) {
                aiState = WALKING;
                speed = 1;
            }
            movement = glm::vec3(1, 0, 0);
            break;
    }
}

void Entity::AIChaser(Entity* player) {
    switch (aiState) {
        case IDLE:
            if (glm::distance(position, player->position) < 1.5f) {
                aiState = ATTACKING;
                speed = 0.5f;
            }
            break;
        case ATTACKING:
            velocity.y = movement.y * speed;
            if (player->position.x < position.x - 0.025f) {
                movement.x = -1;
            }
            else if (player->position.x > position.x + 0.05f) {
                movement.x = 1;
            }
            if (player->position.y < position.y - 0.025f) {
                movement.y = -1;
            }
            else if (player->position.y > position.y + 0.025f) {
                movement.y = 1;
            }
            break;
    }
}

void Entity::AI(int* gameStatus, int* remainingEnemies, Entity* player, Entity* sword, Entity* platforms, int platformCount) {
    sensorLeft = glm::vec3(position.x - 0.3f, position.y - 0.3f, 0);
    sensorRight = glm::vec3(position.x + 0.3f, position.y - 0.3f, 0);

    if (CheckCollision(player) == true) {
        *gameStatus = 0;
    }
    if (CheckCollision(sword) == true) {
        isActive = false;
        *remainingEnemies -= 1;
    }

    switch (aiType) {
        case WALKER:
            AIWalker(platforms, platformCount);
            break;
        case SLAMMER:
            AISlammer(player);
            break;
        case CHASER:
            AIChaser(player);
            break;
    }
}

void Entity::Update(float deltaTime, int* gameStatus, int* remainingEnemies, Entity* player, Entity* sword, Entity* platforms, int platformCount)
{
    if (isActive == false) {
        return;
    }

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType == ENEMY && isActive == true) {
        AI(gameStatus, remainingEnemies, player, sword, platforms, platformCount);
    }
    if (entityType == SWORD) {
        if (player->direction == LEFT) {
            position = glm::vec3(player->position.x - 0.37f, player->position.y - 0.07f, 0);
        }
        else if (player->direction == RIGHT) {
            position = glm::vec3(player->position.x + 0.37f, player->position.y - 0.07f, 0);
        }

        animTime += deltaTime;
        if (animTime >= 0.5f) {
            isActive = false;
            animTime = 0.0f;
        }
    }

    if (jump) {
        jump = false;
        velocity.y += jumpPower;
    }

    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    position.y += velocity.y * deltaTime; // Move on Y
    if (aiType != CHASER) {
        CheckCollisionsY(platforms, platformCount);// Fix if needed
    }

    position.x += velocity.x * deltaTime; // Move on X
    if (aiType != CHASER) {
        CheckCollisionsX(platforms, platformCount);// Fix if needed
    }

    if (entityType == PLAYER) {
        if (position.y < -3.25) {
            *gameStatus = 0;
        }
    }

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };

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
    
    if (isActive == false) {
        return;
    }

    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    float vertices[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}