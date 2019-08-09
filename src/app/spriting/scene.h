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
    #version 400

    in vec4 ex_Color;
    in vec2 ex_UV;
    out vec4 out_Color;
    // Only works starting with 4.2
    //layout(binding=1) uniform sampler2D spriteSampler;
    uniform sampler2D spriteSampler;

    void main(void)
    {
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
    // Don't use the default GL_TEXTURE0, to make sure it does not work just by accident
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

    ////
    //// the literal texture defined above
    ////
    //constexpr GLsizei width  = 4;
    //constexpr GLsizei height = 4;
    //const GLvoid * imageData = gTextureImage;

    //
    // From file image texture
    //
    /// \TODO ensure correct lifetime of the image data vis-à-vis async OpenGL loading to texture
    //static const Image ring("/tmp/sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png");
    static const Image ring("d:/projects/sprites/sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png");

    //// Whole image
    //const GLsizei width  = ring.mDimension.width();
    //const GLsizei height = ring.mDimension.height();
    //const GLvoid * imageData = ring;

    // Sub-part
    // Found by measuring in the image raster
    constexpr GLsizei width  = 347-3;
    constexpr GLsizei height = 303-3;
    static const Image firstRing = ring.crop({{3, 3}, {width, height}});
    const GLvoid * imageData = firstRing;

    //std::vector<std::vector<unsigned char>> cutouts = ring.cutouts({
    //        {3,    3},
    //        {353,  3},
    //        {703,  3},
    //        {1053, 3},
    //        {1403, 3},
    //        {1753, 3},
    //        {2103, 3},
    //        {2453, 3},
    //    },
    //    width,
    //    height
    //);

#if defined(GL_VERSION_4_2)
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height); 
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
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
#else
    {
        ErrorCheck check;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
#endif

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    /// \TODO handle use program and un-use (glUseProgram(0)), otherwise preventing correct deletion
    ///       since the used program is a global status, it should not be altered in a specific program dtor
    glUseProgram(program);
#if !defined(GL_VERSION_4_2)
    {
        ErrorCheck check;
        glUniform1i(glGetUniformLocation(program, "spriteSampler"), 1);
    }
#endif


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
