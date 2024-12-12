#ifndef EMP_LOG_WINDOW
#define EMP_LOG_WINDOW
#include "imgui.h"
#include "debug/log.hpp"
namespace emp {

class LogWindow {
    std::vector<std::string> m_items;
    std::vector<std::string> m_history;
    int m_history_pos; // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter m_filter;
    bool m_isAutoScrolling;
    bool m_isScrollingToBottom;
public:
    bool isOpen = true;
    void draw(const char* title);
    void clearLog();
    void addLog(const char* fmt, ...) IM_FMTARGS(2);
    LogWindow();
    LogWindow(const LogWindow&) = delete;
    LogWindow(LogWindow&&) = delete;
    LogWindow& operator=(const LogWindow&) = delete;
    LogWindow& operator=(LogWindow&&) = delete;
};

struct LogToGUIWindow : public LogOutput {
    LogWindow* console;
    void Output(std::string msg) override {
        console->addLog("%s", msg.c_str());
    }
    LogToGUIWindow(LogWindow* window) : console(window) {}
};
}
#endif
