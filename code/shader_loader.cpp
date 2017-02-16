#include <windows.h>

#define MAX_SHADER_CODE_SIZE 1024
//NOTE make one struct for all shaders??
struct shaderData
{
    GLuint shader;
    BOOL fileLoaded;
    GLboolean isModified;
    char filePath[512];
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER fileSize;
    FILETIME lastWriteTime;
    char code[MAX_SHADER_CODE_SIZE];    
};
//NOTE like this??
struct shaders
{
    GLuint shaderProgram;
    struct shaderData vertex;
    struct shaderData fragmentShader;
};


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

    fileData = (char*)malloc((size_t)shader->fileSize.QuadPart);
    
    ReadFile(shader->fileHandle, fileData, shader->fileSize.QuadPart, &bytesRead, NULL);

    shader->fileLoaded = true;

        
    shader->shader = glCreateShader(shaderType);

    GLchar *shaderCode = (GLchar*)malloc(shader->fileSize.QuadPart);
    CopyMemory(shaderCode, fileData, shader->fileSize.QuadPart);
    //TODO use temporary shader and only use shader struct shader after sucessfull compile?
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
        CopyMemory(shader->code, fileData, MAX_SHADER_CODE_SIZE);
        shader->isModified = true;
    }

    free(fileData);
    free((void*)shaderCode);
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
