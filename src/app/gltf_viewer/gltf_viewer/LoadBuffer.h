#pragma once


#include <arte/Gltf.h>


namespace ad {
namespace gltfviewer {


std::vector<std::byte> 
loadBufferData(arte::Const_Owned<arte::gltf::BufferView> aBufferView);

// TODO Ad 2022/03/15 Implement a way to only load exactly the bytes of an accessor
// This could notably allow optimization when a contiguous accessor is used as-is.


} // namespace gltfviewer
} // namespace ad
