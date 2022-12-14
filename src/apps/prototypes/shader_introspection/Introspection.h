#include <renderer/GL_Loader.h>
#include <renderer/Shading.h>

#include <cassert>


namespace ad {


inline void inspectProgram(const graphics::Program & aProgram)
{
    if(GLAD_GL_ARB_program_interface_query)
    {
        GLint resourceCount;
        GLint maxNameLength;

        //
        // Vertex Attributes
        //
        GLenum programInterface = GL_PROGRAM_INPUT;
        glGetProgramInterfaceiv(aProgram, programInterface, GL_ACTIVE_RESOURCES, &resourceCount);
        glGetProgramInterfaceiv(aProgram, programInterface, GL_MAX_NAME_LENGTH, &maxNameLength);

        auto iterate = [&](const std::string & aCategory)
        {
            auto nameBuffer = std::make_unique<char[]>(maxNameLength);
            for(GLuint resourceId = 0; resourceId != resourceCount; ++resourceId)
            {
                const GLenum properties[] = {
                    GL_NAME_LENGTH, // The result is actually unused if we use the shared namedBuffer
                    GL_TYPE,
                    GL_LOCATION,
                    //GL_BLOCK_INDEX, // Index of the active interface block containing, -1 if not part of any.
                };
                GLint params[std::size(properties)];
                GLint length;

                glGetProgramResourceiv(aProgram, programInterface, resourceId,
                                       (GLsizei)std::size(properties), properties,
                                       (GLsizei)std::size(params), &length, params);

                //// The returned length makes provision for a terminating null character.
                //std::string attributeName(params[0], '\0');
                //// This does write a null character at the end.
                //glGetProgramResourceName(aProgram, programInterface, resourceId,
                //                        (GLsizei)std::size(attributeName), &params[0], attributeName.data());
                glGetProgramResourceName(aProgram, programInterface, resourceId,
                                         maxNameLength, &params[0], nameBuffer.get());
                // WARNING: glGetProgramResource() returned the length **including** null terminator,
                //          whereas glGetProgramResourceName() returns the length **excluding** null terminator.
                // No need to substract 1 from the length, as it does not include null terminator.
                std::string attributeName(nameBuffer.get(), params[0]);

                // (Notably) uniforms within uniform blocks are getting -1 as location,
                // use it as a q&d filter.
                if (params[2] != -1)
                {
                    std::cout << aCategory << " " << attributeName 
                        << " at location " << params[2]
                        << " has type " << params[1] 
                        << "." << std::endl;
                }
            }
        };
        iterate("Attribute");

        //
        // Uniforms
        //
        programInterface = GL_UNIFORM;
        glGetProgramInterfaceiv(aProgram, programInterface, GL_ACTIVE_RESOURCES, &resourceCount);
        glGetProgramInterfaceiv(aProgram, programInterface, GL_MAX_NAME_LENGTH, &maxNameLength);

        iterate("Uniform");

        //
        // Uniform blocks
        //
        programInterface = GL_UNIFORM_BLOCK;
        glGetProgramInterfaceiv(aProgram, programInterface, GL_ACTIVE_RESOURCES, &resourceCount);
        glGetProgramInterfaceiv(aProgram, programInterface, GL_MAX_NAME_LENGTH, &maxNameLength);

        auto blockIterate = [&](const std::string & aCategory, GLenum aVariableInterface)
        {
            auto nameBuffer = std::make_unique<char[]>(maxNameLength);
            for(GLuint blockId = 0; blockId != resourceCount; ++blockId)
            {
                const GLenum properties[] = {
                    GL_NAME_LENGTH, // The result is actually unused if we use the shared namedBuffer
                    GL_BUFFER_BINDING,
                    GL_NUM_ACTIVE_VARIABLES,
                };
                GLint params[std::size(properties)];
                GLint length;

                glGetProgramResourceiv(aProgram, programInterface, blockId,
                                       (GLsizei)std::size(properties), properties,
                                       (GLsizei)std::size(params), &length, params);

                glGetProgramResourceName(aProgram, programInterface, blockId,
                                         maxNameLength, &params[0], nameBuffer.get());
                // WARNING: glGetProgramResource() returned the length **including** null terminator,
                //          whereas glGetProgramResourceName() returns the length **excluding** null terminator.
                // No need to substract 1 from the length, as it does not include null terminator.
                std::string blockName(nameBuffer.get(), params[0]);

                std::cout << aCategory << " " << blockName 
                    << " at binding index " << params[1]
                    << " has " << params[2] << " active variables"
                    << "." << std::endl;

                //
                // Get the variables composing this interface block
                //
                const GLenum variableProperty[] = {
                    GL_ACTIVE_VARIABLES, // array of active variables indices
                };
                std::vector<GLint> variableIds(params[2]);

                glGetProgramResourceiv(aProgram, programInterface, blockId,
                                       (GLsizei)std::size(variableProperty), variableProperty,
                                       (GLsizei)std::size(variableIds), &length, variableIds.data());
                assert(length == params[2]);

                for(GLint variableId : variableIds)
                {
                    const GLenum properties[] = {
                        GL_NAME_LENGTH, // The result is actually unused if we use the shared namedBuffer
                        GL_TYPE,
                        GL_BLOCK_INDEX,
                        //GL_LOCATION, // should be -1 for uniforms in uniform blocks,
                                       // but is not available for shader storabe blocks' variables
                    };
                    GLint params[std::size(properties)];
                    GLint length;

                    glGetProgramResourceiv(aProgram, aVariableInterface, variableId,
                                           (GLsizei)std::size(properties), properties,
                                           (GLsizei)std::size(params), &length, params);

                    std::vector<char> nameBuffer(params[0]);
                    glGetProgramResourceName(aProgram, aVariableInterface, variableId,
                                             (GLsizei)std::size(nameBuffer), &params[0], nameBuffer.data());
                    // WARNING: glGetProgramResource() returned the length **including** null terminator,
                    //          whereas glGetProgramResourceName() returns the length **excluding** null terminator.
                    // No need to substract 1 from the length, as it does not include null terminator.
                    std::string variableName(nameBuffer.data(), params[0]);

                    assert(params[2] == blockId);
                    //assert(params[3] == -1);
                    std::cout << "\t* " << variableName 
                        << " has type " << params[1]
                        << "." << std::endl;
                }
            }
        };

        blockIterate("Uniform block", GL_UNIFORM);

        //
        // Shader storage blocks
        //
        if (GLAD_GL_ARB_shader_storage_buffer_object)
        {
            programInterface = GL_SHADER_STORAGE_BLOCK;
            glGetProgramInterfaceiv(aProgram, programInterface, GL_ACTIVE_RESOURCES, &resourceCount);
            glGetProgramInterfaceiv(aProgram, programInterface, GL_MAX_NAME_LENGTH, &maxNameLength);

            blockIterate("Shader storage block", GL_BUFFER_VARIABLE);
        }
    }
}


} // namespace ad