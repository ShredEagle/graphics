#pragma once

#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification.h>

#include <glad/glad.h>


const GLchar* gVertexShader = R"#(
    #version 400

    layout(location=0) in vec4 in_Position;
    layout(location=1) in vec4 in_Color;
    layout(location=2) in vec2 in_UV;
    out vec4 ex_Color;
    out vec2 ex_UV;

    void main(void)
    {
        gl_Position = in_Position;
        ex_Color = in_Color;
        ex_UV = in_UV;
    }
)#";

const GLchar* gFragmentShader = R"#(
    #version 420

    in vec4 ex_Color;
    in vec2 ex_UV;
    out vec4 out_Color;
    //layout(binding=0) uniform sampler2D spriteSampler;
    uniform sampler2D spriteSampler;

    void main(void)
    {
        //out_Color = ex_Color;
        out_Color = texture(spriteSampler, ex_UV).rgba;
    }
)#";

constexpr size_t gVerticesCount = 4;

GLfloat gVerticesPositions[gVerticesCount][4] = {
    {-0.8f, -0.8f, 0.0f, 1.0f},
    {-0.8f,  0.8f, 0.0f, 1.0f},
    { 0.8f, -0.8f, 0.0f, 1.0f},
    { 0.8f,  0.8f, 0.0f, 1.0f},
};

GLfloat gVerticesColors[gVerticesCount][4] = {
   {1.0f, 0.0f, 0.0f, 1.0f},
   {0.0f, 1.0f, 0.0f, 1.0f},
   {0.0f, 0.0f, 1.0f, 1.0f},
   {0.8f, 0.5f, 0.5f, 1.0f},
};

GLfloat gVerticesUVs[gVerticesCount][2] = {
   {0.0f, 0.0f},
   {0.0f, 1.0f},
   {1.0f, 0.0f},
   {1.0f, 1.0f},
};

GLubyte gTextureImage[4][4][4] = {
    { {255, 0, 0, 255},   {0, 255, 0, 255},   {0, 0, 255, 255},   {0, 0, 0, 255} },
    { {128, 0, 100, 255}, {100, 128, 0, 255}, {0, 100, 128, 255}, {0, 0, 0, 255} },
    { {255, 0, 0, 255},   {0, 255, 0, 255},   {0, 0, 255, 255},   {0, 0, 0, 255} },
    { {128, 0, 100, 255}, {100, 128, 0, 255}, {0, 100, 128, 255}, {0, 0, 0, 255} },
};


namespace ad
{

struct [[nodiscard]] Scene
{
    Scene(VertexSpecification aVertexSpecification,
          Texture aTexture,
          Program aProgram) :
        mVertexSpecification{std::move(aVertexSpecification)},
        mTexture{std::move(aTexture)},
        mProgram{std::move(aProgram)}
    {}

    VertexSpecification mVertexSpecification;
    Texture mTexture;
    Program mProgram;
};

Scene setupScene()
{
    // Geometry
    VertexSpecification specification;
    glBindVertexArray(specification.mVertexArray);

    specification.mVertexBuffers.emplace_back(makeAndLoadBuffer(0, gVerticesPositions));
    specification.mVertexBuffers.emplace_back(makeAndLoadBuffer(1, gVerticesColors));
    specification.mVertexBuffers.emplace_back(makeAndLoadBuffer(2, gVerticesUVs));

    // Texture
    Texture texture;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //const GLenum PIXEL_FORMAT = GL_RGBA;
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4); 
    {
        GLint isSuccess;
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_FORMAT, &isSuccess);
        if ( isSuccess != GL_TRUE)
        {
            const std::string message{"Error calling 'glTexStorage2D'"};
            std::cerr << message << std::endl;
            throw std::runtime_error(message);
        }
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, gTextureImage);


    // Program
    Shader<GL_VERTEX_SHADER> vertexShader;
    compileShader(vertexShader, gVertexShader);

    Shader<GL_FRAGMENT_SHADER> fragmentShader;
    compileShader(fragmentShader, gFragmentShader);

    Program program;
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    // Apparently, it is a good practice to detach as soon as link is done
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    handleGlslError(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog);

    // Bind texture
    GLuint TextureID  = glGetUniformLocation(program, "spriteSampler");
    glUniform1i(TextureID, 0);

    /// \TODO handle use program and un-use (glUseProgram(0)), otherwise preventing correct deletion
    ///       since the used program is a global status, it should not be altered in a specific program dtor
    glUseProgram(program);

    /// \TODO return the shader resource instances to keep correct lifetime
    /// note that deleting the shader is just marking them for deletion until no program use the,
    /// so it should not be a problem to have them deleted here for the moment
    return {
        std::move(specification),
        std::move(texture),
        std::move(program),
    };
}

void updateScene()
{}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gVerticesCount);
}

} // namespace ad
