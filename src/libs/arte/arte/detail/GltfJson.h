#pragma once

#include "Json.h"


namespace ad::arte::gltf {

    template <class> struct Index;

} // namespace ad::arte::gltf


namespace ad::math {

    template <class> class Quaternion;

} // namespace ad::math


namespace nlohmann {

    template <class T_indexed>
    struct adl_serializer<ad::arte::gltf::Index<T_indexed>> 
    {
        static ad::arte::gltf::Index<T_indexed> from_json(const Json & aJson)
        {
            return aJson.get<typename ad::arte::gltf::Index<T_indexed>::Value_t>();
        }
    };

    //
    // Gltf json to math types
    //
    template <class T_number>
    struct adl_serializer<ad::math::Vec<3, T_number>> 
    {
        static ad::math::Vec<3, T_number> from_json(const Json & aJson)
        {
            return ad::math::Vec<3, T_number>{
                aJson.at(0).get<T_number>(),
                aJson.at(1).get<T_number>(),
                aJson.at(2).get<T_number>(),
            };
        }
    };


    template <class T_number>
    struct adl_serializer<ad::math::hdr::Rgba<T_number>> 
    {
        static ad::math::hdr::Rgba<T_number> from_json(const Json & aJson)
        {
            return ad::math::hdr::Rgba<T_number>{
                aJson.at(0).get<T_number>(),
                aJson.at(1).get<T_number>(),
                aJson.at(2).get<T_number>(),
                aJson.at(3).get<T_number>(),
            };
        }
    };


    template <class T_number>
    struct adl_serializer<ad::math::Quaternion<T_number>> 
    {
        static ad::math::Quaternion<T_number> from_json(const Json & aJson)
        {
            return ad::math::Quaternion<T_number>{
                aJson.at(0).get<T_number>(),
                aJson.at(1).get<T_number>(),
                aJson.at(2).get<T_number>(),
                aJson.at(3).get<T_number>(),
            };
        }
    };


    template <class T_number>
    struct adl_serializer<ad::math::AffineMatrix<4, T_number>> 
    {
        static ad::math::AffineMatrix<4, T_number> from_json(const Json & aJson)
        {
            return ad::math::AffineMatrix<4, T_number>{
                // Affine matrix implicitly has [0, 0, 0, 1] in projection elements.
                // The matrix is stored in column major order in the json:
                // simply reading it in the same order into a column major matrix does the transposition.
                ad::math::Matrix<4, 3, T_number>{
                    aJson.at(0).get<T_number>(),
                    aJson.at(1).get<T_number>(),
                    aJson.at(2).get<T_number>(),
                    //aJson.at(3).get<T_number>(),
                    aJson.at(4).get<T_number>(),
                    aJson.at(5).get<T_number>(),
                    aJson.at(6).get<T_number>(),
                    //aJson.at(7).get<T_number>(),
                    aJson.at(8).get<T_number>(),
                    aJson.at(9).get<T_number>(),
                    aJson.at(10).get<T_number>(),
                    //aJson.at(11).get<T_number>(),
                    aJson.at(12).get<T_number>(),
                    aJson.at(13).get<T_number>(),
                    aJson.at(14).get<T_number>(),
                    //aJson.at(15).get<T_number>(),
                }
            };
        }
    };


} // namespace nlohmann
