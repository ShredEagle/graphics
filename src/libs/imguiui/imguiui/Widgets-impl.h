#pragma once


#include "Widgets.h"

#include <renderer/MappedGL.h>

#include <imgui.h>


namespace ad {
namespace imguiui {


template <class T_iterator, class F_stringify>
void addCombo(const char *aLabel,
              T_iterator & aValue,
              T_iterator aFirst, T_iterator aLast,
              F_stringify aToString)
{
    static const ImGuiComboFlags flags = 0;
    // Pass in the preview value visible before opening the combo (it could be anything)
    const std::string combo_preview_value = aToString(aValue);
    if (ImGui::BeginCombo(aLabel, combo_preview_value.c_str(), flags))
    {
        Guard scopeCombo([]()
        {
            ImGui::EndCombo();
        });

        for (; aFirst != aLast; ++aFirst)
        {
            const bool isSelected = (aValue == aFirst);
            if (ImGui::Selectable(aToString(aFirst).c_str(), isSelected))
            {
                aValue = aFirst;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }
}


template <class T_enumeration, std::size_t N_spanExtent>
void addCombo(const char * aLabel,
              T_enumeration & aValue,
              const std::span<const T_enumeration, N_spanExtent> & aAvailableValues)
{
    // Note: this is intended to make the to_string(GLenum) visible
    // but is smelly. Can we address that another way?
    using graphics::to_string;

    static const ImGuiComboFlags flags = 0;
    // Pass in the preview value visible before opening the combo (it could be anything)
    const std::string combo_preview_value = to_string(aValue);
    if (ImGui::BeginCombo(aLabel, combo_preview_value.c_str(), flags))
    {
        Guard scopeCombo([]()
        {
            ImGui::EndCombo();
        });

        for (unsigned int n = 0; n < std::size(aAvailableValues); n++)
        {
            T_enumeration candidate = aAvailableValues[n];
            const bool isSelected = (aValue == candidate);
            if (ImGui::Selectable(to_string(candidate).c_str(), isSelected))
            {
                aValue = candidate;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
    }
}


} // namespace imguiui
} // namespace ad