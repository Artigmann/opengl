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

/*
  1280×720 (HD, 720p)
  1920×1080 (FHD, Full HD, 2K 1080p)
  2560×1440 (QHD, WQHD, Quad HD, 1440p)
*/

static GLuint windowWidth = 1280;
static GLuint windowHeight = 720;

static struct game gameState;
static struct sprite spriteRenderData;

static glm::vec2 playerSize(100, 20);
static GLfloat playerVelocity(500.0f);

static glm::vec2 initialBallVelocity(100.0f, -350.0f);
static GLfloat ballRadius = 12.5f;

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
    
    for (int i = 0; i < LEVEL_HEIGHT*LEVEL_WIDTH; i++)
    {
        if (level->bricks[i].destroyed == false)
        {
            drawGameObject(&level->bricks[i], spriteRenderData);
        }
    }
}

static inline void drawPlayer(struct gameObject *player, struct sprite *spriteRenderData)
{
    drawGameObject(player, spriteRenderData);
}

static inline void drawBall(struct ball *ball, struct sprite *spriteRenderData)
{
    drawGameObject((struct gameObject*)ball, spriteRenderData);
}

static void gameInitLevel(struct gameLevel *level, GLuint levelWidth, GLuint levelHeight)
{
    
    int tileData[LEVEL_HEIGHT][LEVEL_WIDTH] =
    {
        { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, },
        { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, }, 	 
        { 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, }, 	 
        { 4, 1, 4, 1, 4, 0, 0, 1, 0, 0, 4, 1, 4, 1, 4, },	 
        { 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, }, 	 
        { 3, 3, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3, 3, },	 
        { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, },	 
        { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, }, 
    };

    
    GLuint height = LEVEL_HEIGHT;
    GLuint width = LEVEL_WIDTH;
    GLfloat unitWidth = levelWidth / (GLfloat)width;
    GLfloat unitHeight = levelHeight / height;
    
    int gameObjectPos = 0;
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // NOTE init object before use.
            gameInitObject(&level->bricks[gameObjectPos]);
            
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
                level->bricks[gameObjectPos].sprite = ResourceManager::GetTexture("block");

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

static inline void initPLayer(struct game *gameState)
{
    gameInitObject(&gameState->player);
    glm::vec2 playerPos = glm::vec2(gameState->width / 2 - playerSize.x / 2, gameState->height - playerSize.y);
    gameState->player.position = playerPos;
    gameState->player.size = playerSize;
//    gameState->player.velocity = playerVelocity;
    gameState->player.sprite = ResourceManager::GetTexture("paddle");

    gameInitObject(&gameState->player2);
    playerPos = glm::vec2(100,0);//glm::vec2(gameState->width / 2 - playerSize.x / 2, gameState->height - playerSize.y);
    gameState->player2.position = playerPos;
    gameState->player2.size = playerSize;
//    gameState->player.velocity = playerVelocity;
    gameState->player2.sprite = ResourceManager::GetTexture("paddle");
}

static inline void initBall(struct game *gameState)
{
    gameInitObject((struct gameObject*)&gameState->ball);
    glm::vec2 ballPos = gameState->player.position + glm::vec2(playerSize.x / 2 - ballRadius, -ballRadius * 2);
    gameState->ball.position = ballPos;
    gameState->ball.radius = ballRadius;
    gameState->ball.velocity = initialBallVelocity;
    gameState->ball.sprite = ResourceManager::GetTexture("face");
    gameState->ball.size = glm::vec2(ballRadius * 2, ballRadius * 2);
    gameState->ball.stuck = true;
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
    ResourceManager::LoadTexture("paddle.png", true, "paddle");
    
    // Set render-specific controls
    initSprite(sprite);
    sprite->shader =  ResourceManager::GetShader("sprite");

    
    gameLoadLevel(&gameState->level, NULL, gameState->width, gameState->height * 0.5f);
    initPLayer(gameState);
    initPLayer(gameState);
    initBall(gameState);
}

static void moveBall(struct game *gameState, GLfloat dt)
{
    if (!gameState->ball.stuck)
    {
        // Move the ball
        gameState->ball.position += gameState->ball.velocity * dt;
        // Then check if outside window bounds and if so, reverse velocity and restore at correct position
        if (gameState->ball.position.x <= 0.0f)
        {
            gameState->ball.velocity.x = -gameState->ball.velocity.x;
            gameState->ball.position.x = 0.0f;
        }
        else if (gameState->ball.position.x + gameState->ball.size.x >= windowWidth)
        {
            gameState->ball.velocity.x = -gameState->ball.velocity.x;
            gameState->ball.position.x = windowWidth - gameState->ball.size.x;
        }
        if (gameState->ball.position.y <= 0.0f)
        {
            gameState->ball.velocity.y = -gameState->ball.velocity.y;
            gameState->ball.position.y = 0.0f;
        }
    }
}

static GLboolean checkCollision(struct gameObject *object1, struct gameObject *object2)
{
    GLboolean collisionX = object1->position.x + object1->size.x >= object2->position.x
        && object2->position.x + object2->size.x >= object1->position.x;
    GLboolean collisionY = object1->position.y + + object1->size.y >= object2->position.y
        && object2->position.y + object2->size.y >= object1->position.y;

    GLboolean collided = collisionX && collisionY;
    return collided;
}

static void doCollision(struct game *gameState)
{
    for (int i = 0; i < LEVEL_HEIGHT*LEVEL_WIDTH; i++)
    {
        if (gameState->level.bricks[i].destroyed == false)
        {
            if (checkCollision((struct gameObject*)&gameState->ball, &gameState->level.bricks[i]))
            {
                if (gameState->level.bricks[i].isSolid == false)
                {
                    gameState->level.bricks[i].destroyed = true;
                }
            }
        }
    }
}

static void resetBall(struct game *gameState)
{
    initBall(gameState);
}

static void gameUpdate(struct game *gameState, GLfloat dt)
{
    moveBall(gameState, dt);
    if (gameState->ball.stuck)
    {
        resetBall(gameState);
    }
    doCollision(gameState);
}

static void gameProcessInput(struct game *gameState, GLfloat dt)
{
    if (gameState->state == GAME_ACTIVE)
    {
        GLfloat velocity = playerVelocity * dt;
        // Move playerboard
        if (gameState->keys[GLFW_KEY_A] || gameState->keys[GLFW_KEY_LEFT])
        {
            if (gameState->player.position.x >= 0)
            {
                gameState->player.position.x -= velocity; 
                if (gameState->ball.stuck)
                {
                    gameState->ball.position.x -= velocity;
                }
            }
        }
        if (gameState->keys[GLFW_KEY_D] || gameState->keys[GLFW_KEY_RIGHT])
        {
            if (gameState->player.position.x <= gameState->width - gameState->player.size.x)
            {
                gameState->player.position.x += velocity;
                if (gameState->ball.stuck)
                {
                    gameState->ball.position.x += velocity;
                }
            }
        }
        if (gameState->keys[GLFW_KEY_SPACE])
        {
            gameState->ball.stuck = false;
        }
        if (gameState->keys[GLFW_KEY_R])
        {
            gameState->ball.stuck = true;
        }
    }
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
	        drawPlayer(&gameState->player, spriteRenderData);
//            drawPlayer(&gameState->player2, spriteRenderData);
        drawBall(&gameState->ball, spriteRenderData);
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

static void readEntireShaderFromFile(struct shaderData *shader, GLenum shaderType)
{
    DWORD bytesRead = 0;
    char *fileData;
    shader->fileHandle = CreateFile(shader->filePath, GENERIC_READ,
                                          FILE_SHARE_READ|FILE_SHARE_WRITE,
                                          NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL, NULL);
    GetFileTime(shader->fileHandle, NULL, NULL, &shader->lastWriteTime);
    GetFileSizeEx(shader->fileHandle, &shader->fileSize);

    fileData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, shader->fileSize.QuadPart);
    assert(fileData);
    ReadFile(shader->fileHandle, fileData, shader->fileSize.QuadPart, &bytesRead, NULL);
    shader->fileLoaded = true;
        
    shader->shader = glCreateShader(shaderType);

    GLchar *shaderCode = (GLchar*)HeapAlloc(GetProcessHeap(),
                                            HEAP_ZERO_MEMORY, shader->fileSize.QuadPart);
    assert(shaderCode);
    CopyMemory((void*)shaderCode, (void*)fileData, shader->fileSize.QuadPart);
    glShaderSource(shader->shader, 1, &shaderCode, NULL);
    glCompileShader(shader->shader);

    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader->shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader->shader, 1024, NULL, infoLog);
        OutputDebugString("Shader Compilation ERROR: \n");
        OutputDebugString(infoLog);
        OutputDebugString("\n");
    }
    else
    {
        //NOTE don't load new shader if it didn't compile!!
        // ONLY SUPPORTS MAX_SHADER_CODE_SIZE bytes large shaders
        ZeroMemory(shader->code, MAX_SHADER_CODE_SIZE);
        CopyMemory(shader->code, fileData, MAX_SHADER_CODE_SIZE);
        shader->isModified = true;
    }

    HeapFree(GetProcessHeap(), 0, fileData);
    HeapFree(GetProcessHeap(), 0, shaderCode);
}

//TODO make code size dynamic, not static
static void hotLoadShaderFromFile(struct shaderData *vertexShader, struct shaderData *fragmentShader, GLuint *shaderProgram)
{
    DWORD bytesRead = 0;
    
    if (!vertexShader->fileLoaded)
    {
        readEntireShaderFromFile(vertexShader, GL_VERTEX_SHADER);
    }
    else
    {        
        FILETIME lastWriteTime = vertexShader->lastWriteTime;
        GetFileTime(vertexShader->fileHandle, NULL, NULL, &vertexShader->lastWriteTime);
        if (CompareFileTime(&lastWriteTime, &vertexShader->lastWriteTime) != 0)
        {
            readEntireShaderFromFile(vertexShader, GL_VERTEX_SHADER);
        }
    }

    if (!fragmentShader->fileLoaded)
    {
        readEntireShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);
    }
    else
    {
        FILETIME lastWriteTime = fragmentShader->lastWriteTime;
        GetFileTime(fragmentShader->fileHandle, NULL, NULL, &fragmentShader->lastWriteTime);
        if (CompareFileTime(&lastWriteTime, &fragmentShader->lastWriteTime) != 0)
        {
            readEntireShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);
        }
    }

    if (vertexShader->isModified || fragmentShader->isModified)
    {
        glDeleteProgram(*shaderProgram);

        *shaderProgram = glCreateProgram();
        glAttachShader(*shaderProgram, vertexShader->shader);
        glAttachShader(*shaderProgram, fragmentShader->shader);
        glLinkProgram(*shaderProgram);

        
        GLint success;
        GLchar infoLog[1024];
        glGetShaderiv(*shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(*shaderProgram, 1024, NULL, infoLog);
            OutputDebugString("Shader Link ERROR: \n");
            OutputDebugString(infoLog);
            OutputDebugString("\n");
        }
        
        glDeleteShader(vertexShader->shader);
        glDeleteShader(fragmentShader->shader);

        vertexShader->isModified = false;
        fragmentShader->isModified = false;
    }
} 

static struct shaderData vertexShader;
static struct shaderData fragmentShader;

static void renderTriangle(GLuint *VAO)
{
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };  
    
    GLuint VBO;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(*VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int CALLBACK WinMain(HINSTANCE intance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // glfwGetPrimaryMonitor()
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Breakout",
                                          NULL, NULL);
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
    glEnable(GL_MULTISAMPLE);

    gameInit(&gameState, &spriteRenderData);

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    gameState.state = GAME_ACTIVE;


    GLuint VAO, shaderProgram;
    shaderProgram = 0;
    
    strcpy_s(vertexShader.filePath, 512, "vertex_test.gl");
    strcpy_s(fragmentShader.filePath, 512, "fragment_test.gl");
    hotLoadShaderFromFile(&vertexShader, &fragmentShader, &shaderProgram);
    renderTriangle(&VAO);


    GLint nbFrames = 0;
    GLfloat lastTime = glfwGetTime();

    glfwSwapInterval(1);

    while(!glfwWindowShouldClose(window))
    {
        hotLoadShaderFromFile(&vertexShader, &fragmentShader, &shaderProgram);
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        GLfloat currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0f)
        {
            char string[512];
            snprintf(string, sizeof(string), "ms/frame: %f fps: %d\n", (1000.0f/(GLfloat)nbFrames),
                     nbFrames);
            OutputDebugString(string);
            nbFrames = 0;
            lastTime += 1.0f;

        }
        glfwPollEvents();
#if 1
        gameProcessInput(&gameState, deltaTime);

        gameUpdate(&gameState, deltaTime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

                

        gameRender(&gameState, &spriteRenderData);

        glfwSwapBuffers(window);
#else
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
#endif

    }

    ResourceManager::Clear();
    
    glfwTerminate();
    return 0;
}
