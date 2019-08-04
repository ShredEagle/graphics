#pragma once

#include <renderer/Shading.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>


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


namespace ad
{

VertexArrayObject makeVAO()
{
    GLuint vertexArrayId;
    glGenVertexArrays(1, &vertexArrayId);
    return {vertexArrayId};
}

struct [[nodiscard]] Scene
{
    Scene(VertexSpecification aVertexSpecification, Program aProgram) :
        mVertexSpecification{std::move(aVertexSpecification)},
        mProgram{std::move(aProgram)}
    {}

    VertexSpecification mVertexSpecification;
    Program mProgram;
};

Scene setupScene()
{
    // Geometry
    VertexSpecification specification{makeVAO(), {}};
    glBindVertexArray(specification.mVertexArray);

    specification.mVertexBuffers.emplace_back(makeAndLoadBuffer(0, gVerticesPositions));
    specification.mVertexBuffers.emplace_back(makeAndLoadBuffer(1, gVerticesColors));


    // Program
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
    return {
        std::move(specification),
        {programId},
    };
}

void updateScene()
{}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

} // namespace ad
