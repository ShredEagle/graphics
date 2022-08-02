#include "Render.h"

#include <glad/glad.h>

#include <functional>
#include <string>
#include <vector>
#include <iostream>


const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 in_Position;
    layout(location=1) in vec4 in_Color;
    out vec4 ex_Color;

    void main(void)
    {
        gl_Position = in_Position;
        ex_Color = in_Color;
    }
)#";

const GLchar* gFragmentShader = R"#(
    #version 400

    in vec4 ex_Color;
    out vec4 out_Color;

    void main(void)
    {
        out_Color = ex_Color;
    }
)#";

GLfloat gVerticesPositions[][4] = {
    {-0.8f, -0.8f, 0.0f, 1.0f},
    {0.0f,  0.8f, 0.0f, 1.0f},
    {0.8f, -0.8f, 0.0f, 1.0f},
};

GLfloat gVerticesColors[][4] = {
   {1.0f, 0.0f, 0.0f, 1.0f},
   {0.0f, 1.0f, 0.0f, 1.0f},
   {0.0f, 0.0f, 1.0f, 1.0f},
};


template <class T_scalar>
struct MappedGl;

template <> struct MappedGl<GLfloat>
{ static const GLenum enumerator = GL_FLOAT; };


template <class T_element, int N_vertices, int N_attributeDimension>
VertexBufferObject loadUpBuffer(GLuint aAttributeId,
                                T_element (& data)[N_vertices][N_attributeDimension])
{
    GLuint vertexBufferId;
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glVertexAttribPointer(aAttributeId,
                          N_attributeDimension,
                          MappedGl<T_element>::enumerator,
                          GL_FALSE,
                          0,
                          0);
    glEnableVertexAttribArray(aAttributeId);

    return {vertexBufferId, aAttributeId};
}

VertexSpecification initializeGeometry()
{
    GLuint vertexArrayId;
    glGenVertexArrays(1, &vertexArrayId);
    glBindVertexArray(vertexArrayId);

    std::vector<VertexBufferObject> buffers;
    buffers.emplace_back(loadUpBuffer(0, gVerticesPositions));
    buffers.emplace_back(loadUpBuffer(1, gVerticesColors));

    return {
        {vertexArrayId},
        // Cannot list initialize vector of move-only object
        // see: https://stackoverflow.com/a/7232135/1027706
        std::move(buffers),
    };
}


void handleGlslError(GLuint objectId,
                     GLenum aStatusEnumerator,
                     std::function<void(GLuint, GLenum, GLint*)> statusGetter,
                     std::function<void(GLuint, GLsizei, GLsizei*, GLchar*)> infoLogGetter)
{
    GLint status;
    statusGetter(objectId, aStatusEnumerator, &status);
    if(status == GL_FALSE)
    {
        GLint maxLength = 0;
        statusGetter(objectId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        infoLogGetter(objectId, maxLength, &maxLength, &infoLog[0]);

        std::string errorLog(infoLog.begin(), infoLog.end());
        std::cerr << "GLSL error[" << maxLength << "]: " << errorLog;
        
        throw std::runtime_error("GLSL error");
    } 
}

template <int N_stage>
Shader<N_stage> compileShader(const std::string &aSource)
{
    Shader<N_stage> shader{glCreateShader(N_stage)};

    //glShadersource(shader, 1, char*[]{aSource.data()}, NULL);
    const char* sourceProxy = aSource.data(); 
    glShaderSource(shader, 1, &sourceProxy, NULL);
    glCompileShader(shader);

    handleGlslError(shader,
                    GL_COMPILE_STATUS,
                    glGetShaderiv,
                    glGetShaderInfoLog);

    return shader;
}


Program initializeProgram()
{
    Shader<GL_VERTEX_SHADER> vertexShader 
        = compileShader<GL_VERTEX_SHADER>(gVertexShader);
    Shader<GL_FRAGMENT_SHADER> fragmentShader 
        = compileShader<GL_FRAGMENT_SHADER>(gFragmentShader);

    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);

    glLinkProgram(programId);
    // Apparently, it is a good practice to detach as soon as link is done
    glDetachShader(programId, vertexShader);
    glDetachShader(programId, fragmentShader);

    handleGlslError(programId, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog);

    glUseProgram(programId);

    /// \TODO return the resource holder classes to keep correct lifetime of resources
    /// note that deleting the shader is just marking them for deletion, so it 
    /// should not be a problem to have them deleted here for the moment
    return {programId};
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDrawArrays(GL_TRIANGLES, 0, 3);
}
