#include "log_window.hpp"
namespace emp {
LogWindow::LogWindow() {
    Log::s_out.reset();
    Log::s_out = std::make_unique<LogToGUIWindow>(this);
}
void LogWindow::draw(const char* title) {
    if(!isOpen)
        return;

    if (!ImGui::Begin(title, &isOpen)) {
        ImGui::End();
        return;
    }

    // As a specific feature guaranteed by the library, after calling
    // Begin() the last Item represent the title bar. So e.g.
    // IsItemHovered() will return true when hovering the title bar. Here we
    // create a context menu only available from the title bar.

    ImGui::TextWrapped("Enter 'HELP' for help.");

    // TODO: display items starting from the bottom

    if (ImGui::SmallButton("Clear")) {
        clearLog();
    }
    ImGui::SameLine();
    bool copy_to_clipboard = ImGui::SmallButton("Copy");
    // static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t =
    // ImGui::GetTime(); AddLog("Spam %f", t); }

    ImGui::Separator();

    // Options menu
    if (ImGui::BeginPopup("Options")) {
        ImGui::Checkbox("Auto-scroll", &m_isAutoScrolling);
        ImGui::EndPopup();
    }

    // Options, Filter
    ImGui::SetNextItemShortcut(
        ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_Tooltip);
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();
    m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    ImGui::Separator();

    // Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve =
        ImGui::GetStyle().ItemSpacing.y +
        ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollingRegion",
            ImVec2(0, -footer_height_to_reserve),
            ImGuiChildFlags_NavFlattened,
            ImGuiWindowFlags_HorizontalScrollbar)) {
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::Selectable("Clear"))
                clearLog();
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
        if (copy_to_clipboard) {
            ImGui::LogToClipboard();
        }
        for (int i = 0; i < m_items.size(); i++) {
            auto& item = m_items[i];
            //TODO
            if (!m_filter.PassFilter(item.c_str()))
                continue;
            
            // Normally you would store more information in your item than
            // just a string. (e.g. make Items[] an array of structure,
            // store color/type etc.)
            ImVec4 color;
            bool has_color = false;
            if (item.find("[error]") != std::string::npos) {
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                has_color = true;
            } else if (item.find("# ", 2) == 0) {
                color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
                has_color = true;
            }
            if (has_color)
                ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item.c_str());
            if (has_color)
                ImGui::PopStyleColor();
        }
        if (copy_to_clipboard)
            ImGui::LogFinish();

        // Keep up at the bottom of the scroll region if we were already at
        // the bottom at the beginning of the frame. Using a scrollbar or
        // mouse-wheel will take away from the bottom edge.
        if (m_isScrollingToBottom ||
            (m_isAutoScrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        m_isScrollingToBottom = false;

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::Separator();

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();

    ImGui::End();
}
void LogWindow::clearLog() {
    m_items.clear();
}
void LogWindow::addLog(const char* fmt, ...) IM_FMTARGS(2) {
    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    m_items.push_back(buf);
}
}
