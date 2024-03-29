#pragma once


#include <handy/AtomicVariations.h>

#include <atomic>
#include <span>
#include <string_view>


namespace ad {
namespace imguiui {


/// @brief Add a checkbox on state aValue.
/// @return True if the checkbox is checked, false otherwise
bool addCheckbox(const char * aLabel, bool & aValue);

/// @brief Add a checkbox on state aValue.
/// @return True if the checkbox is checked, false otherwise.
bool addCheckbox(const char * aLabel, std::atomic<bool> & aValue);


template <class T_iterator, class F_stringify>
void addCombo(const char *aLabel,
              T_iterator & aValue,
              T_iterator aFirst, T_iterator aLast,
              F_stringify aToString);

template <class T_enumeration, std::size_t N_spanExtent>
void addCombo(const char *aLabel,
              T_enumeration & aValue,
              const std::span<const T_enumeration, N_spanExtent> & aAvailableValues);


template <class T_enumeration, std::size_t N_spanExtent>
void addCombo(const char * aLabel,
              std::atomic<T_enumeration> & aValue,
              const std::span<const T_enumeration, N_spanExtent> & aAvailableValues)
{
    T_enumeration value = aValue.load();
    addCombo<T_enumeration, N_spanExtent>(aLabel, value, aAvailableValues);
    aValue.store(value);
}


template <class T_enumeration, std::size_t N_spanExtent>
void addCombo(const char * aLabel,
              MovableAtomic<T_enumeration> & aValue,
              const std::span<const T_enumeration, N_spanExtent> & aAvailableValues)
{
    addCombo<T_enumeration, N_spanExtent>
            (aLabel, static_cast<std::atomic<T_enumeration> &>(aValue), aAvailableValues);
}


} // namespace imguiui
} // namespace ad
