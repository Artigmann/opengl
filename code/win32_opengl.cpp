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

static int windowWidth = 800, windowHeight = 600;
static int framebufferWidth, framebufferHeight;
static bool rotateRigth;
static bool rotateLeft;

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

    GLfloat vertices[] =
    {
        -0.3f, -0.3f, 0.0f,
        0.3f, -0.3f, 0.0f,
        0.0f, 0.8f, 0.0f
    };

    GLuint VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glfwSwapInterval(1);
    glEnable(GL_MULTISAMPLE);  

    GLfloat rotate = 0.0f;

    double lastTime = glfwGetTime();
    int frameCounter = 0;
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
        //transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));


        if (rotateRigth)
        {
            rotate -= 1.0f;
        }
        else if (rotateLeft)
        {
            rotate += 1.0f;
        }

        transform = glm::scale(transform, glm::vec3(0.2, 0.2, 0.2));
        transform = glm::rotate(transform, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));

        // Get matrix's uniform location and set matrix
        GLint transformLoc = glGetUniformLocation(shaders.Program, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }
    
    return 0;
}
