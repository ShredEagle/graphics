#include "catch.hpp"

#include <graphics/ApplicationGlfw.h>

#include <renderer/FrameBuffer.h>
#include <renderer/Shading.h>
#include <renderer/UniformBuffer.h>
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


SCENARIO("Uniform buffer scoping, indexed.")
{
    INITIALIZE_GL_CONTEXT();

    GIVEN("Two uniform buffers.")
    {
        UniformBufferObject ubo_1;
        UniformBufferObject ubo_2;

        WHEN("The first is (unguarded) bound in a scope, then the second is bound to a specific index.")
        {
            const GLint bindingIndex = 3;
            {
                bind(ubo_1);
                bind(ubo_2, BindingIndex{bindingIndex});

                THEN("The second UBO is bound to the both the indexed and general binding points.")
                {
                    CHECK(getBound(ubo_1) == ubo_2);
                    CHECK(getBound(ubo_1, BindingIndex{bindingIndex}) == ubo_2);
                }

                WHEN("Another UBO is scope-bound to a specific index, inside a nested scope.")
                {
                    UniformBufferObject ubo_3;

                    {
                        ScopedBind scopedUbo{ubo_3, BindingIndex{bindingIndex}};

                        THEN("The third UBO is bound to the both the indexed and general binding points.")
                        {
                            CHECK(getBound(ubo_1) == ubo_3);
                            CHECK(getBound(ubo_1, BindingIndex{bindingIndex}) == ubo_3);
                        }
                    }

                    WHEN("The nested scope is exited.")
                    {
                        THEN("The second UBO is bound to the both the indexed and general binding points.")
                        {
                            CHECK(getBound(ubo_1) == ubo_2);
                            CHECK(getBound(ubo_1, BindingIndex{bindingIndex}) == ubo_2);
                        }
                    }
                }
            }

            WHEN("The scope is exited.")
            {
                THEN("The second UBO is still the bound to the both the indexed and general binding points.")
                {
                    CHECK(getBound(ubo_1) == ubo_2);
                    CHECK(getBound(ubo_1, BindingIndex{bindingIndex}) == ubo_2);
                }
            }
        }
    }
}


template <class T_tested, class ... VT_aArgs>
void testScoping(const std::string aName, VT_aArgs && ... aCtorArgs)
{
    GIVEN("A " + aName + ".")
    {
        T_tested tested_1{std::forward<VT_aArgs>(aCtorArgs)...};

        WHEN("It is (unguarded) bound in a scope.")
        {
            {
                bind(tested_1);

                THEN("It is the currently bound " + aName + " within the scope.")
                {
                    CHECK(getBound(tested_1) == tested_1);
                }

                WHEN("Another " + aName + " is scope-bound, inside a nested scope.")
                {
                    T_tested tested_2{std::forward<VT_aArgs>(aCtorArgs)...};
                    REQUIRE(tested_2 != tested_1); // Sanity check

                    {
                        ScopedBind scopedtested{tested_2};

                        THEN("The second " + aName + " is the currently bound tested within the scope.")
                        {
                            CHECK(getBound(tested_2) == tested_2);
                            CHECK(getBound(tested_1) == tested_2); // same thing
                        }
                    }

                    WHEN("The nested scope is exited.")
                    {
                        THEN("The first " + aName + " is the currently bound tested.")
                        {
                            CHECK(getBound(tested_1) == tested_1);
                        }
                    }
                }
            }

            WHEN("The scope is exited.")
            {
                THEN("It is still the bound " + aName + ".")
                {
                    CHECK(getBound(tested_1) == tested_1);
                }
            }
        }
    }
}


SCENARIO("Uniform buffer scoping (non-indexed).")
{
    INITIALIZE_GL_CONTEXT();

    testScoping<UniformBufferObject>("UBO");
}


SCENARIO("Texture scoping.")
{
    INITIALIZE_GL_CONTEXT();

    testScoping<Texture>("texture", (GLenum)GL_TEXTURE_2D);
}


SCENARIO("VertexArrayObject scoping.")
{
    INITIALIZE_GL_CONTEXT();

    testScoping<VertexArrayObject>("VAO");
}


SCENARIO("Program scoping.")
{
    INITIALIZE_GL_CONTEXT();

    GIVEN("A program.")
    {
        Program tested_1 = makeLinkedProgram({
            {GL_VERTEX_SHADER, "void main(){gl_Position = vec4(0.);}"},
            {GL_FRAGMENT_SHADER, "void main(){}"},
        });

        WHEN("It is (unguarded) bound in a scope.")
        {
            {
                bind(tested_1);

                THEN("It is the currently bound program within the scope.")
                {
                    CHECK(getBound(tested_1) == tested_1);
                }

                WHEN("Another program is scope-bound, inside a nested scope.")
                {
                    Program tested_2 = makeLinkedProgram({
                        {GL_VERTEX_SHADER, "void main(){gl_Position = vec4(0.);}"},
                        {GL_FRAGMENT_SHADER, "void main(){}"},
                    });
                    REQUIRE(tested_2 != tested_1); // Sanity check

                    {
                        ScopedBind scopedtested{tested_2};

                        THEN("The second program is the currently bound tested within the scope.")
                        {
                            CHECK(getBound(tested_2) == tested_2);
                            CHECK(getBound(tested_1) == tested_2); // same thing
                        }
                    }

                    WHEN("The nested scope is exited.")
                    {
                        THEN("The first program is the currently bound tested.")
                        {
                            CHECK(getBound(tested_1) == tested_1);
                        }
                    }
                }
            }

            WHEN("The scope is exited.")
            {
                THEN("It is still the bound program.")
                {
                    CHECK(getBound(tested_1) == tested_1);
                }
            }
        }
    }
}