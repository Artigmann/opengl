#include <windows.h>
#include <assert.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "SOIL\SOIL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>


#include "win32_opengl.h"

static GLuint windowWidth = 800;
static GLuint windowHeight = 600;

static struct game gameState;
static struct sprite spriteRenderData;

static void initSprite(struct sprite *sprite)
{
    GLuint VBO;
    GLfloat vertices[] =
    {
        // Pos      // Tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 
    
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &sprite->quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(sprite->quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void drawSprite(struct sprite *sprite, Texture2D &texture, glm::vec2 position,
                       glm::vec2 size, GLfloat rotate, glm::vec3 color)
{
    sprite->shader.Use();
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); 
    model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f)); 
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    sprite->shader.SetMatrix4("model", model);
    sprite->shader.SetVector3f("spriteColor", color);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();
        
    glBindVertexArray(sprite->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

static void gameInitObject(struct gameObject *object)
{
    object->position = glm::vec2(0.0f, 0.0f);
    object->size = glm::vec2(1.0f, 1.0f);
    object->velocity = glm::vec2(0.0f);
    object->color = glm::vec3(1.0f);
    object->rotation = 0.0f;
    object->isSolid = false;
    object->destroyed = false;
}

static void drawGameObject(struct gameObject *object, struct sprite *spriteRenderData)
{
    drawSprite(spriteRenderData, object->sprite, object->position, object->size, object->rotation, object->color);
}

static void drawLevel(struct gameLevel *level, struct sprite *spriteRenderData)
{
    
    for (int i = 0; i < 3*6; i++)
    {
        drawGameObject(&level->bricks[i], spriteRenderData);
    }
}

static void gameInitLevel(struct gameLevel *level, GLuint levelWidth, GLuint levelHeight)
{
    
    int tileData[3][6] =
    {
        {1, 1, 1, 1, 1, 1}, 
        {2, 2, 0, 0, 2, 2},
        {3, 3, 4, 4, 3, 3},
    };

    
    GLuint height = 3;
    GLuint width = 6;
    GLfloat unitWidth = levelWidth / (GLfloat)width;
    GLfloat unitHeight = levelHeight / height;
    
    int gameObjectPos = 0;
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (tileData[y][x] == 1)
            {
                glm::vec2 pos(unitWidth * x, unitHeight * y);
                glm::vec2 size(unitWidth, unitHeight);
                glm::vec3 color(0.8f, 0.8f, 0.7f);
                level->bricks[gameObjectPos].position = pos;
                level->bricks[gameObjectPos].size = size;
                level->bricks[gameObjectPos].color = color;
                level->bricks[gameObjectPos].sprite = ResourceManager::GetTexture("block_solid");
                level->bricks[gameObjectPos].isSolid = true;

                gameObjectPos++;
            }
            else if (tileData[y][x] > 1)
            {
                glm::vec3 color = glm::vec3(1.0f); // original: white
                if (tileData[y][x] == 2)
                {
                    color = glm::vec3(0.2f, 0.6f, 1.0f);
                }
                else if (tileData[y][x] == 3)
                {
                    color = glm::vec3(0.0f, 0.7f, 0.0f);
                }
                else if (tileData[y][x] == 4)
                {
                    color = glm::vec3(0.8f, 0.8f, 0.4f);
                }
                else if (tileData[y][x] == 5)
                {
                    color = glm::vec3(1.0f, 0.5f, 0.0f);
                }

                glm::vec2 pos(unitWidth * x, unitHeight * y);
                glm::vec2 size(unitWidth, unitHeight);
                level->bricks[gameObjectPos].position = pos;
                level->bricks[gameObjectPos].size = size;
                level->bricks[gameObjectPos].color = color;
                level->bricks[gameObjectPos].sprite = ResourceManager::GetTexture("block_solid");

                gameObjectPos++;
            }
        }
    }
}

static void gameLoadLevel(struct gameLevel *level, GLchar *file, GLuint width, GLuint height)
{
    // TODO make dummy level here
    /*
      
      1 1 1 1 1 1 
      2 2 0 0 2 2
      3 3 4 4 3 3
      
     */
    gameInitLevel(level, width, height);
}

static void gameInit(struct game *gameState, struct sprite *sprite)
{
    gameState->width = windowWidth;
    gameState->height = windowHeight;

    // Load shaders
    ResourceManager::LoadShader("vertex.gl", "fragment.gl", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(gameState->width), 
        static_cast<GLfloat>(gameState->height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    // Load textures
    ResourceManager::LoadTexture("background.jpg", GL_FALSE, "background");
    ResourceManager::LoadTexture("awesomeface.png", GL_TRUE, "face");
    ResourceManager::LoadTexture("block.png", GL_FALSE, "block");
    ResourceManager::LoadTexture("block_solid.png", GL_FALSE, "block_solid");
//    ResourceManager::LoadTexture("paddle.png", true, "paddle");
    
    // Set render-specific controls
    initSprite(sprite);
    sprite->shader =  ResourceManager::GetShader("sprite");

    
    gameLoadLevel(&gameState->level, NULL, gameState->width, gameState->height * 0.5f);
    
    
}

static void gameUpdate(GLfloat dt)
{
    
}

static void gameProcessInput(GLfloat dt)
{
    
}

static void gameRender(struct game *gameState, struct sprite *spriteRenderData)
{
    if (gameState->state == GAME_ACTIVE)
    {
        // Draw background
//        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);
        drawSprite(spriteRenderData, ResourceManager::GetTexture("background"),
                   glm::vec2(0, 0), glm::vec2(gameState->width, gameState->height), 0.0f, glm::vec3(1.0f));
        // // Draw level
        drawLevel(&gameState->level, spriteRenderData);

        // // Draw player
        // Player->Draw(*Renderer);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);        
    }
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            gameState.keys[key] = GL_TRUE;
        }
        else if (action == GLFW_RELEASE)
        {
            gameState.keys[key] = GL_FALSE;
        }
            
    }
}

int CALLBACK WinMain(HINSTANCE intance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Breakout", NULL, NULL);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();

    glfwSetKeyCallback(window, keyCallback);

    glViewport(0, 0, windowWidth, windowHeight);

    //NOTE read up on these!
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gameInit(&gameState, &spriteRenderData);

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    gameState.state = GAME_ACTIVE;

    while(!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents();

        gameProcessInput(deltaTime);

        gameUpdate(deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        gameRender(&gameState, &spriteRenderData);

        glfwSwapBuffers(window);
    }

    ResourceManager::Clear();
    
    glfwTerminate();
    return 0;
}
