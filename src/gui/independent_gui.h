#pragma once

// Independent GUI System
// =====================
// This is a lightweight, independent GUI system created as a replacement for ImGui.
// It provides the basic architecture and API for GUI rendering using OpenGL and GLFW.
//
// Current Status: Functional Architecture with Basic Text Rendering
// - Provides GUI layout and windowing system
// - Supports buttons, text, input fields, sliders, progress bars, and menus
// - Uses immediate mode rendering with OpenGL
// - Includes built-in 8x13 bitmap font for text rendering (ASCII 32-126)
// 
// Known Limitations (for production use):
// 1. Text rendering: Uses basic 8x13 bitmap font
//    - For enhanced typography: Integrate FreeType, stb_truetype, or texture atlases
// 2. Legacy OpenGL: Uses immediate mode (glBegin/glEnd)
//    - Solution: Migrate to VBOs, VAOs, and modern shaders
// 3. Input handling: Visual display works but keyboard/mouse interaction is minimal
//    - Solution: Implement GLFW input callbacks for full interactivity
//
// The system successfully demonstrates independent GUI architecture without
// external GUI library dependencies (beyond OpenGL/GLFW).

#include <string>
#include <functional>
#include <vector>
#include <memory>

struct GLFWwindow;

namespace IndependentGUI {

// Color structure
struct Color {
    float r, g, b, a;
    Color(float r_ = 1.0f, float g_ = 1.0f, float b_ = 1.0f, float a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}
};

// Vec2 structure
struct Vec2 {
    float x, y;
    Vec2(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}
};

// GUI Context
class Context {
public:
    Context();
    ~Context();
    
    bool Initialize(GLFWwindow* window);
    void Shutdown();
    
    void NewFrame();
    void Render();
    
    // Window management
    bool BeginWindow(const char* title, bool* open = nullptr, Vec2 pos = Vec2(), Vec2 size = Vec2());
    void EndWindow();
    
    // Menu bar
    bool BeginMainMenuBar();
    void EndMainMenuBar();
    bool BeginMenu(const char* label);
    void EndMenu();
    bool MenuItem(const char* label, const char* shortcut = nullptr, bool* selected = nullptr);
    
    // Widgets
    void Text(const char* text);
    void TextColored(Color color, const char* text);
    void Separator();
    void Spacing();
    bool Button(const char* label);
    bool InputText(const char* label, char* buffer, size_t buffer_size);
    bool InputTextMultiline(const char* label, char* buffer, size_t buffer_size, Vec2 size = Vec2());
    bool SliderInt(const char* label, int* value, int min, int max);
    void ProgressBar(float fraction, Vec2 size = Vec2(), const char* overlay = nullptr);
    bool CollapsingHeader(const char* label, bool default_open = false);
    
    // Layout
    void SameLine();
    bool BeginChild(const char* id, Vec2 size = Vec2(), bool border = false);
    void EndChild();
    
    // Style
    void SetNextWindowSize(Vec2 size, int condition = 0);
    void SetNextWindowPos(Vec2 pos, int condition = 0);
    
    // Utility
    void BulletText(const char* text);
    void TextWrapped(const char* text);
    
    // Demo
    void ShowDemoWindow(bool* open);
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Condition flags for SetNextWindow* functions
enum Condition {
    Always = 0,
    Once = 1,
    FirstUseEver = 2,
    Appearing = 3
};

// Tree node flags
enum TreeNodeFlags {
    None = 0,
    DefaultOpen = 1 << 0
};

} // namespace IndependentGUI
