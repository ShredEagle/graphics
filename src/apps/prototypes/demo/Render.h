#include <glad/glad.h>

#include <vector>

struct [[nodiscard]] VertexArrayObject 
{
    // Disable copy
    VertexArrayObject(const VertexArrayObject &) = delete;
    VertexArrayObject & operator=(const VertexArrayObject &) = delete;

    // Movable
    VertexArrayObject(VertexArrayObject && aOther) :
        mVertexArrayId{aOther.mVertexArrayId}
    {
        aOther.mVertexArrayId = 0;
    }
    //VertexArrayObject & operator=(VertexArrayObject && aOther)
    //{
    //    mVertexArrayId = aOther.mVertexArrayId;
    //    aOther.mVertexArrayId = 0;
    //    return *this;
    //}

    VertexArrayObject(GLuint aVertexArrayId) :
        mVertexArrayId(aVertexArrayId)
    {}

    ~VertexArrayObject()
    {
        glDeleteVertexArrays(1, &mVertexArrayId);
    }

    GLuint mVertexArrayId;
};


struct [[nodiscard]] VertexBufferObject
{
    // Disable copy
    VertexBufferObject(const VertexBufferObject &) = delete;
    VertexBufferObject & operator=(const VertexBufferObject &) = delete;

    // Movable
    VertexBufferObject(VertexBufferObject && aOther) :
        mVBOId{aOther.mVBOId},
        mAttributeId{aOther.mAttributeId}
    {
        aOther.mVBOId = 0;
    }
    //VertexBufferObject & operator=(VertexBufferObject && aOther)
    //{
    //    mVBOId = aOther.mVBOId;
    //    mAttributeId = aOther.mAttributeId;
    //    aOther.mVBOId = 0;
    //}

    VertexBufferObject(GLuint aVBOId, GLuint aAttributeId) :
        mVBOId{aVBOId},
        mAttributeId{aAttributeId}
    {}

    ~VertexBufferObject()
    {
        if (mVBOId) // avoid disabling if it was moved from
        {
            glDisableVertexAttribArray(mAttributeId); 
            glDeleteBuffers(1, &mVBOId); 
        }
    }

    GLuint mVBOId;
    GLuint mAttributeId;
};


struct [[nodiscard]] VertexSpecification
{
    VertexSpecification(VertexArrayObject aVertexArray,
                        std::vector<VertexBufferObject> aVertexBuffers) :
        mVertexArray{std::move(aVertexArray)},
        mVertexBuffers{std::move(aVertexBuffers)}
    {}

    VertexArrayObject  mVertexArray; 
    std::vector<VertexBufferObject> mVertexBuffers; 
};


template <int N_stage>
struct [[nodiscard]] Shader
{
    // Disable copy
    Shader(const Shader &) = delete;
    Shader & operator=(const Shader &) = delete;

    // Movable
    Shader(Shader && aOther) :
        mShaderId{aOther.mShaderId}
    {
        aOther.mShaderId = 0;
    }

    Shader(GLuint aShaderId) :
        mShaderId(aShaderId)
    {}

    ~Shader()
    {
        glDeleteShader(mShaderId);
    }

    operator GLuint ()
    { return mShaderId; }

    GLuint mShaderId;
};


struct [[nodiscard]] Program
{
    // Disable copy
    Program(const Program &) = delete;
    Program & operator=(const Program &) = delete;

    // Movable
    Program(Program && aOther) :
        mProgramId{aOther.mProgramId}
    {
        aOther.mProgramId = 0;
    }

    Program(GLuint aProgramId) :
        mProgramId(aProgramId)
    {}

    ~Program()
    {
        glDeleteProgram(mProgramId);
    }

    operator GLuint ()
    { return mProgramId; }

    GLuint mProgramId;
};


VertexSpecification initializeGeometry();
Program initializeProgram();

void render();

