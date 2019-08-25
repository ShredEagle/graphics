#pragma once

#include <vector>

namespace ad {

    template <class T_element>
    std::size_t getStoredSize(const std::vector<T_element> & aVector)
    {
        return sizeof(T_element) * aVector.size();
    }

} // namespace ad
