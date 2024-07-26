#include "MappedGL.h"


namespace ad::graphics {


#define GLENUMCASE(enumval)     \
    case enumval:               \
        return #enumval;


std::string to_string(GLenum aGLEnumerator) 
{
    switch(aGLEnumerator)
    {
        // Primitive modes
        GLENUMCASE(GL_POINTS);
        GLENUMCASE(GL_LINE_STRIP);
        GLENUMCASE(GL_LINE_LOOP);
        GLENUMCASE(GL_LINES);
        GLENUMCASE(GL_LINE_STRIP_ADJACENCY);
        GLENUMCASE(GL_LINES_ADJACENCY);
        GLENUMCASE(GL_TRIANGLE_STRIP);
        GLENUMCASE(GL_TRIANGLE_FAN);
        GLENUMCASE(GL_TRIANGLES);
        GLENUMCASE(GL_TRIANGLE_STRIP_ADJACENCY);
        GLENUMCASE(GL_TRIANGLES_ADJACENCY);
        GLENUMCASE(GL_PATCHES);

        //
        // Types
        //
        GLENUMCASE(GL_FLOAT);
        GLENUMCASE(GL_DOUBLE);
        GLENUMCASE(GL_BYTE);
        GLENUMCASE(GL_UNSIGNED_BYTE);
        GLENUMCASE(GL_SHORT);
        GLENUMCASE(GL_UNSIGNED_SHORT);
        GLENUMCASE(GL_INT);
        GLENUMCASE(GL_UNSIGNED_INT);
        GLENUMCASE(GL_BOOL);

        //
        // Targets
        //
        GLENUMCASE(GL_ARRAY_BUFFER);
        GLENUMCASE(GL_ATOMIC_COUNTER_BUFFER);
        GLENUMCASE(GL_COPY_READ_BUFFER);
        GLENUMCASE(GL_COPY_WRITE_BUFFER);
        //TYPEENUMCASE(GL_DISPATCH_INDIRECT_BUFFER);
        GLENUMCASE(GL_DRAW_INDIRECT_BUFFER);
        GLENUMCASE(GL_ELEMENT_ARRAY_BUFFER);
        GLENUMCASE(GL_PIXEL_PACK_BUFFER);
        GLENUMCASE(GL_PIXEL_UNPACK_BUFFER);
        //TYPEENUMCASE(GL_QUERY_BUFFER);
        GLENUMCASE(GL_SHADER_STORAGE_BUFFER);
        GLENUMCASE(GL_TEXTURE_BUFFER);
        GLENUMCASE(GL_TRANSFORM_FEEDBACK_BUFFER);
        GLENUMCASE(GL_UNIFORM_BUFFER);

        //
        // Fitering
        //
        GLENUMCASE(GL_NEAREST);
        GLENUMCASE(GL_LINEAR);
        GLENUMCASE(GL_NEAREST_MIPMAP_NEAREST);
        GLENUMCASE(GL_LINEAR_MIPMAP_NEAREST);
        GLENUMCASE(GL_NEAREST_MIPMAP_LINEAR);
        GLENUMCASE(GL_LINEAR_MIPMAP_LINEAR);

        //
        // Culling
        //
        GLENUMCASE(GL_CW);
        GLENUMCASE(GL_CCW);
        GLENUMCASE(GL_FRONT);
        GLENUMCASE(GL_FRONT_AND_BACK);
        GLENUMCASE(GL_BACK);

        //
        // Shader types
        //
        GLENUMCASE(GL_VERTEX_SHADER);
        GLENUMCASE(GL_GEOMETRY_SHADER);
        GLENUMCASE(GL_FRAGMENT_SHADER);

        //
        // Polygon modes
        //
        GLENUMCASE(GL_POINT);
        GLENUMCASE(GL_LINE);
        GLENUMCASE(GL_FILL);

        //
        // Texture internal formats
        // 
        GLENUMCASE(GL_COMPRESSED_RG_RGTC2);
        GLENUMCASE(GL_COMPRESSED_SIGNED_RG_RGTC2);
        GLENUMCASE(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT);
        GLENUMCASE(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT);
        GLENUMCASE(GL_COMPRESSED_RGBA_BPTC_UNORM);
        GLENUMCASE(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM);

    default:
        throw std::domain_error{"Invalid GL enumerator."};
    }
}


#undef GLENUMCASE


} // namespace ad::graphics