#pragma once


#include <test_commons/PathProvider.h>

#include <arte/Image.h>

#include <renderer/Shading.h>
#include <renderer/Texture.h>
#include <renderer/VertexSpecification_deduction.h>

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

const GLchar* gAnimationFragmentShader = R"#(
    #version 400

    in vec4 ex_Color;
    in vec2 ex_UV;
    out vec4 out_Color;
    uniform sampler2DArray spriteSampler;
    uniform int frame;

    void main(void)
    {
        out_Color = texture(spriteSampler, vec3(ex_UV, frame));
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


namespace ad {
namespace graphics {

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

    specification.mVertexBuffers.emplace_back(loadVertexBuffer(specification.mVertexArray, 0, std::span<GLfloat[4]>(gVerticesPositions)));
    specification.mVertexBuffers.emplace_back(loadVertexBuffer(specification.mVertexArray, 1, std::span<GLfloat[4]>(gVerticesColors)));
    specification.mVertexBuffers.emplace_back(loadVertexBuffer(specification.mVertexArray, 2, std::span<GLfloat[2]>(gVerticesUVs)));

    ////
    //// the literal texture defined above
    ////
    //constexpr GLsizei width  = 4;
    //constexpr GLsizei height = 4;
    //const GLvoid * imageData = gTextureImage;

    //
    // From file image texture
    //
    static const arte::ImageRgba ring{
        resource::pathFor("sonic_big_ring_1991_sprite_sheet_by_augustohirakodias_dc3iwce.png").string(),
        arte::ImageOrientation::InvertVerticalAxis
    };

    ////
    //// Whole image
    ////
    //const GLsizei width  = ring.mDimension.width();
    //const GLsizei height = ring.mDimension.height();
    //const GLvoid * imageData = ring;

    //
    // Sub-parts
    //
    constexpr GLsizei width  = 347-3;
    constexpr GLsizei height = 303-3;

    // First-sprite
    // Found by measuring in the image raster
    static const arte::ImageRgba firstRing = ring.crop({{3, 3}, {width, height}});
    const GLvoid * imageData = static_cast<const unsigned char *>(firstRing);

    // Complete animation
    std::vector<Position2<int>> framePositions = {
            {3,    3},
            {353,  3},
            {703,  3},
            {1053, 3},
            {1403, 3},
            {1753, 3},
            {2103, 3},
            {2453, 3},
    };
    arte::ImageRgba animationArray = ring.prepareArray(framePositions.begin(), framePositions.end(), {width, height});

      //
      // Single image
      //

    // Texture
    Texture texture{GL_TEXTURE_2D};
    // Don't use the default GL_TEXTURE0, to make sure it does not work just by accident
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

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
        // We don't generate mipmaps level, so disable mipmap based filtering for the texture to be complete
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Otherwise, we'd generate mipmap levels
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
#endif

    //
    // Animation
    //
    Texture textureAnimation{GL_TEXTURE_2D_ARRAY};
    {
        ErrorCheck check;

        const GLenum target = GL_TEXTURE_2D_ARRAY;

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(target, textureAnimation);

        glTexImage3D(target, 0, GL_RGBA,
                     width, height, static_cast<GLsizei>(framePositions.size()),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<const unsigned char *>(animationArray));

        // Texture parameters
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
        // Sampler parameters
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }


    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Program
    Shader vertexShader{GL_VERTEX_SHADER};
    compileShader(vertexShader, gVertexShader);

    Shader fragmentShader{GL_FRAGMENT_SHADER};
    //compileShader(fragmentShader, gFragmentShader);
    compileShader(fragmentShader, gAnimationFragmentShader);

    Program program;
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    // Apparently, it is a good practice to detach as soon as link is done
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    handleLinkError(program);

    /// \TODO handle use program and un-use (glUseProgram(0)), otherwise preventing correct deletion
    ///       since the used program is a global status, it should not be altered in a specific program dtor
    glUseProgram(program);
#if !defined(GL_VERSION_4_2)
    {
        ErrorCheck check;
        //glUniform1i(glGetUniformLocation(program, "spriteSampler"), 1);
        glUniform1i(glGetUniformLocation(program, "spriteSampler"), 2);
    }
#endif

    glClearColor(0.1f, 0.2f, 0.3f, 1.f);

    /// \TODO return the shader resource instances to keep correct lifetime
    /// note that deleting the shader is just marking them for deletion until no program use the,
    /// so it should not be a problem to have them deleted here for the moment
    return {
        std::move(specification),
        //std::move(texture),
        std::move(textureAnimation),
        std::move(program),
    };
}

void updateScene(Scene &aScene, double aTimeSeconds)
{
    constexpr int rotationsPerSec = 1;
    constexpr int frames = 8;
    glUniform1i(glGetUniformLocation(aScene.mProgram, "frame"),
                static_cast<int>(aTimeSeconds*rotationsPerSec*frames) % frames);
    std::cerr << "time: " << aTimeSeconds << std::endl;
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, gVerticesCount);
}

} // namespace graphics
} // namespace ad
