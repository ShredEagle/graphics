#pragma once

#include <arte/SpriteSheet.h>

#include <graphics/SpriteAnimator.h>
#include <graphics/Spriting.h>

#include <math/Transformations.h>
#include <math/Interpolation/Interpolation.h>

#include <resource/PathProvider.h>

#include <optional>


namespace ad {
namespace graphics {


const std::string gAnimationName = "run";
// Animation is given in milliseconds, so natural speeds must be in the order of 10^3
const double gAnimationSpeed = 500;


class Scene
{
public:
    Scene(Size2<int> aRenderResolution) :
        mSpriting{}
    {
        mSpriting.setViewportVirtualResolution(aRenderResolution);
        mSpriting.setCameraTransformation(
            math::trans2d::translate(-static_cast<math::Vec<2, GLfloat>>(aRenderResolution) / 2) );

        arte::AnimationSpriteSheet sheet = 
            arte::AnimationSpriteSheet::LoadAseFile(resource::pathFor("animations/" + gAnimationName + ".json"));

        mAnimator.load(sheet, mSpriting);

        mAnimationParameter =
            ParameterAnimation_t{
                mAnimator.get(gAnimationName).totalDuration,
                gAnimationSpeed
        };
    }

    void update(double aDelta)
    {
        Spriting::Instance instance = Spriting::Instance{
            {0.f, 0.f},
            mAnimator.at(gAnimationName, mAnimationParameter->advance(aDelta)),
        };
        mSpriting.updateInstances(std::vector<Spriting::Instance>{instance});
    }

    void render()
    {
        mSpriting.render();
    }

private:

    // TODO hardcoded type here is not ideal
    using ParameterAnimation_t =
        //math::ParameterAnimation<double, math::FullRange>;
        math::ParameterAnimation<double, math::FullRange, math::periodic::Repeat>;
        //math::ParameterAnimation<double, math::FullRange, math::periodic::PingPong>;
        //math::ParameterAnimation<double, math::FullRange, math::periodic::Repeat, math::ease::SmoothStep>;
        //math::ParameterAnimation<double, math::FullRange, math::periodic::PingPong, math::ease::SmoothStep>;
        //math::ParameterAnimation<double, math::FullRange, math::None, math::ease::SmoothStep>;

    Spriting mSpriting;
    sprite::Animator mAnimator;
    std::optional<ParameterAnimation_t> mAnimationParameter;
};


} // namespace graphics
} // namespace ad
