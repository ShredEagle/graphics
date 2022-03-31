#pragma once


#include <arte/Image.h>
#include <arte/gltf/Gltf.h>


namespace ad {
namespace gltfviewer {


//
// Helpers
//
/// \brief Returns the buffer view associated to the accessor, or throw if there is none.
arte::Const_Owned<arte::gltf::BufferView>
checkedBufferView(arte::Const_Owned<arte::gltf::Accessor> aAccessor);


//
// Loaders
//
std::vector<std::byte> 
loadBufferData(arte::Const_Owned<arte::gltf::Buffer> aBuffer);

/// \brief Unified interface to handle both sparse and non-sparse accessors.
/// \attention Returns the complete underlying buffer, offset and size are not applied!
std::vector<std::byte> 
loadBufferData(arte::Const_Owned<arte::gltf::Accessor> aAccessor);

arte::Image<math::sdr::Rgba>
loadImageData(arte::Const_Owned<arte::gltf::Image> aImage);

// TODO Ad 2022/03/15 Implement a way to only load exactly the bytes of an accessor
// This could notably allow optimization when a contiguous accessor is used as-is.


} // namespace gltfviewer
} // namespace ad
