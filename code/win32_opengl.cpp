#include <windows.h>
#include <assert.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "win32_opengl.h"

#include "SOIL\SOIL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

static int windowWidth = 800, windowHeight = 600;
static int framebufferWidth, framebufferHeight;
static bool rotateRigth;
static bool rotateLeft;

typedef float mat4x4[16];

struct vector3
{
    float x, y, z;
};

static void makeIdentityMatrix(mat4x4 matrix)
{
    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
}

static void scaleMatrix4x4(mat4x4 matrix, vector3 scaleVec)
{
    matrix[0] *= scaleVec.x;
    matrix[5] *= scaleVec.y;
    matrix[10] *= scaleVec.z;
    matrix[15] *= 1.0f;
}

static void rotateMatrix4x4(mat4x4 matrix, float rotation)
{
    matrix[0] = (cos(rotation));
    matrix[1] = -(sin(rotation));
    matrix[4] = (sin(rotation));
    matrix[5] = (cos(rotation));
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        rotateRigth = true;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        rotateLeft = true;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
    {
        rotateRigth = false;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
    {
        rotateLeft = false;
    }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int nCmdShow)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 16);

    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "openGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    Shader shaders("vertex.gl", "fragment.gl");

    GLfloat shipVertices[] =
    {
        // left triangle of ship
        -0.5f, -0.5f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        
            // // right triangle of ship
        0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    GLfloat astroidVertices[] =
    {
        // top triangle
        0.5f,  0.5f, 0.0f,  // Top Right
        0.5f, -0.5f, 0.0f,  // Bottom Right
        -0.5f,  0.5f, 0.0f,  // Top Left 
        // bottom triangle
        -0.5f,  0.5f, 0.0f,  // Top Left
        -0.5f, -0.5f, 0.0f,  // Bottom Left
        0.5f,  -0.5f, 0.0f  // Bottom Left 
    };

    GLuint VBO[2], VAO[2];

    glGenVertexArrays(1, &VAO[0]);
    glGenBuffers(1, &VBO[0]);
    
    glGenVertexArrays(1, &VAO[1]);
    glGenBuffers(1, &VBO[1]);
    
    // ship
    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shipVertices), shipVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // astroids
    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(astroidVertices), astroidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    glfwSwapInterval(1);
    glEnable(GL_MULTISAMPLE);  

    GLfloat rotate = 0.0f;

    double lastTime = glfwGetTime();
    int frameCounter = 0;    


    float asteroidPositionX = -5.0f;  
    float asteroidPositionY = 5.0f;
    
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        frameCounter++;
        if ( currentTime - lastTime >= 1.0 ){
            char buffer[512];
            snprintf(buffer, sizeof(buffer), "%f ms/frame %dfps\n", 1000.0/double(frameCounter), frameCounter);
            OutputDebugString(buffer);
            frameCounter = 0;
            lastTime += 1.0;
        }
        
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaders.Use();

        glm::mat4 transform;
        


        if (rotateRigth)
        {
            rotate -= 1.0f;
        }
        else if (rotateLeft)
        {
            rotate += 1.0f;
        }

        // mat4x4 trans = {};
        // makeIdentityMatrix(trans);
        // vector3 scaleVec = {0.2f, 0.2f, 0.2f};
        // scaleMatrix4x4(trans, scaleVec);
        // rotateMatrix4x4(trans, 90.0f);
        
        transform = glm::scale(transform, glm::vec3(0.15, 0.15, 0.15));
        transform = glm::rotate(transform, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));

        // Get matrix's uniform location and set matrix
        GLint transformLoc = glGetUniformLocation(shaders.Program, "transform");
        float *glmValue = glm::value_ptr(transform);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glmValue);
        
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

#if 1
        
        if (frameCounter == 0)
        {
            asteroidPositionX = glm::linearRand(-5.0f, 5.0f);
            asteroidPositionY = glm::linearRand(-5.0f, 5.0f);
        }
#endif
            
        glm::vec3 asteroidPosition(asteroidPositionX, asteroidPositionY, 0.0f);
        
        transform = glm::translate(transform, asteroidPosition);
        transform = glm::scale(transform, glm::vec3(0.3, 0.3, 0.3));
        transform = glm::rotate(transform, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));

        transformLoc = glGetUniformLocation(shaders.Program, "transform");
        glmValue = glm::value_ptr(transform);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glmValue);
        
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glfwSwapBuffers(window);
    }
    
    return 0;
}
