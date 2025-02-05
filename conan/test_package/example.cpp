#include <arte/Image.h>
#include <graphics/Timer.h>
#include <renderer/VertexSpecification.h>

int main()
{
    ad::arte::Image<ad::math::sdr::Grayscale> img{ {2, 3}, 2};
    ad::graphics::Timer timer;
    return EXIT_SUCCESS;
}
