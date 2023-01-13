#include "catch.hpp"

#include <graphics/ApplicationGlfw.h>

#include <renderer/FrameBuffer.h>
#include <renderer/VertexSpecification.h>

#include <memory>


using namespace ad;
using namespace ad::graphics;


#define INITIALIZE_GL_CONTEXT() \
    std::unique_ptr<graphics::ApplicationGlfw> glApp; \
    try \
    { \
        glApp = std::make_unique<graphics::ApplicationGlfw>("dummy", math::Size<2, int>{1, 1}); \
    } \
    catch (const std::exception &) \
    { \
        /* We are on a platform where we cannot obtain the GL context via Glfw, just pass. */ \
        return; \
    } \


SCENARIO("Vertex and Index buffer scoping.")
{
    INITIALIZE_GL_CONTEXT();

    GIVEN("Vertex and index buffers.")
    {
        VertexBufferObject vbo_1;
        IndexBufferObject  ibo_1;

        WHEN("They are (unguarded) bound in a scope.")
        {
            {
                bind(vbo_1);
                bind(ibo_1);

                THEN("They are the currently bound VBO and IBO within the scope.")
                {
                    CHECK(getBound(vbo_1) == vbo_1);
                    CHECK(getBound(ibo_1) == ibo_1);
                }

                WHEN("Another pair of VBO and IBO are scope-bound in a nested scope.")
                {
                    VertexBufferObject vbo_2;
                    IndexBufferObject  ibo_2;
                    REQUIRE(vbo_2 != vbo_1); // Sanity check
                    REQUIRE(ibo_2 != ibo_1); // Sanity check

                    {
                        ScopedBind scopedVbo{vbo_2};
                        ScopedBind scopedIbo{ibo_2};

                        THEN("The second pair are the currently bound VBO/IBO within the scope.")
                        {
                            CHECK(getBound(vbo_2) == vbo_2);
                            CHECK(getBound(ibo_2) == ibo_2);
                        }
                    }

                    WHEN("The nested scope is exited.")
                    {
                        THEN("The first pair are the bound VBO/IBO.")
                        {
                            CHECK(getBound(vbo_1) == vbo_1);
                            CHECK(getBound(ibo_1) == ibo_1);
                        }
                    }
                }
            }

            WHEN("The scope is exited.")
            {
                THEN("They are the still the bound VBO and IBO.")
                {
                    CHECK(getBound(vbo_1) == vbo_1);
                    CHECK(getBound(ibo_1) == ibo_1);
                }
            }
        }
    }
}


SCENARIO("Framebuffer scoping.")
{
    INITIALIZE_GL_CONTEXT();
    GIVEN("A Framebuffer.")
    {
        FrameBuffer fbo_1;

        WHEN("It is bound in a scope (without specifying the target).")
        {
            {
                bind(fbo_1);
                THEN("It is the currently bound FBO for both Read an Draw.")
                {
                    CHECK(getBound(fbo_1, FrameBufferTarget::Read) == fbo_1);
                    CHECK(getBound(fbo_1, FrameBufferTarget::Draw) == fbo_1);
                }
                WHEN("Another framebuffer is scope-bound in a nested scope")
                {
                    FrameBuffer fbo_2;
                    REQUIRE(fbo_2 != fbo_1); // Sanity check

                    {
                        ScopedBind scoped{fbo_2};
                        THEN("The second framebuffer is the currently bound FBO for both Read an Draw.")
                        {
                            CHECK(getBound(fbo_1, FrameBufferTarget::Read) == fbo_2);
                            CHECK(getBound(fbo_1, FrameBufferTarget::Draw) == fbo_2);
                        }
                    }

                    WHEN("The nested scope is exited.")
                    {
                        THEN("The first framebuffer is the bound FBO for both Read an Draw.")
                        {
                            CHECK(getBound(fbo_1, FrameBufferTarget::Read) == fbo_1);
                            CHECK(getBound(fbo_1, FrameBufferTarget::Draw) == fbo_1);
                        }
                    }
                }
            }
        }

        WHEN("It is bound in a scope to the Draw target.")
        {
            {
                bind(fbo_1, FrameBufferTarget::Draw);
                THEN("It is the currently bound FBO for both Draw, not Read.")
                {
                    CHECK(getBound(fbo_1, FrameBufferTarget::Read) != fbo_1);
                    CHECK(getBound(fbo_1, FrameBufferTarget::Draw) == fbo_1);
                }
                WHEN("Another framebuffer is bound in a nested scope to the Draw target")
                {
                    FrameBuffer fbo_2;
                    REQUIRE(fbo_2 != fbo_1); // Sanity check

                    {
                        ScopedBind scoped{fbo_2, FrameBufferTarget::Draw};
                        THEN("The second framebuffer is the bound FBO for Draw, not read.")
                        {
                            CHECK(getBound(fbo_1, FrameBufferTarget::Read) != fbo_2);
                            CHECK(getBound(fbo_1, FrameBufferTarget::Draw) == fbo_2);
                        }
                    }

                    WHEN("The nested scope is exited.")
                    {
                        THEN("The first framebuffer is the bound FBO for both Draw, not Read.")
                        {
                            CHECK(getBound(fbo_1, FrameBufferTarget::Read) != fbo_1);
                            CHECK(getBound(fbo_1, FrameBufferTarget::Draw) == fbo_1);
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("Texture scoping.")
{
    INITIALIZE_GL_CONTEXT();
}