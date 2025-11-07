#include "independent_gui.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace IndependentGUI {

// Simple OpenGL immediate mode rendering helpers
static void RenderRect(float x, float y, float w, float h, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void RenderText(float x, float y, const char* text, Color color) {
    // Simple text rendering using GL (limited but functional)
    glColor4f(color.r, color.g, color.b, color.a);
    glRasterPos2f(x, y);
    // Note: In a real implementation, you'd use a proper text rendering solution
    // For now, this is a placeholder that shows the concept
}

// Window state
struct WindowState {
    std::string title;
    Vec2 pos;
    Vec2 size;
    bool is_open;
    float scroll_y;
    float content_y;
};

// Implementation details
struct Context::Impl {
    GLFWwindow* window;
    int window_width;
    int window_height;
    
    // Current state
    std::vector<WindowState> window_stack;
    WindowState* current_window;
    Vec2 cursor_pos;
    Vec2 next_window_pos;
    Vec2 next_window_size;
    int next_window_condition;
    bool has_next_window_pos;
    bool has_next_window_size;
    
    // Input state
    double mouse_x, mouse_y;
    bool mouse_clicked;
    bool mouse_down;
    
    // Style
    float text_height;
    float padding;
    float spacing;
    
    // Menu state
    bool in_menu_bar;
    bool in_menu;
    
    // Child state
    bool in_child;
    Vec2 child_size;
    
    // Layout state
    bool same_line_mode;
    float line_height;
    
    Impl() : window(nullptr), window_width(1600), window_height(900),
             current_window(nullptr), has_next_window_pos(false),
             has_next_window_size(false), mouse_clicked(false),
             mouse_down(false), text_height(16.0f), padding(8.0f),
             spacing(4.0f), in_menu_bar(false), in_menu(false),
             in_child(false), same_line_mode(false), line_height(20.0f) {}
};

Context::Context() : impl_(std::make_unique<Impl>()) {}

Context::~Context() {
    Shutdown();
}

bool Context::Initialize(GLFWwindow* window) {
    impl_->window = window;
    glfwGetFramebufferSize(window, &impl_->window_width, &impl_->window_height);
    return true;
}

void Context::Shutdown() {
    impl_->window_stack.clear();
    impl_->current_window = nullptr;
}

void Context::NewFrame() {
    glfwGetFramebufferSize(impl_->window, &impl_->window_width, &impl_->window_height);
    glfwGetCursorPos(impl_->window, &impl_->mouse_x, &impl_->mouse_y);
    
    int mouse_button = glfwGetMouseButton(impl_->window, GLFW_MOUSE_BUTTON_LEFT);
    bool mouse_down_now = (mouse_button == GLFW_PRESS);
    impl_->mouse_clicked = (!impl_->mouse_down && mouse_down_now);
    impl_->mouse_down = mouse_down_now;
    
    // Setup OpenGL state
    glViewport(0, 0, impl_->window_width, impl_->window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, impl_->window_width, impl_->window_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void Context::Render() {
    // Already rendered in immediate mode
}

bool Context::BeginWindow(const char* title, bool* open, Vec2 pos, Vec2 size) {
    WindowState window;
    window.title = title;
    window.is_open = (open == nullptr || *open);
    
    if (!window.is_open) return false;
    
    // Use provided or default position/size
    if (impl_->has_next_window_pos) {
        window.pos = impl_->next_window_pos;
        impl_->has_next_window_pos = false;
    } else if (pos.x != 0.0f || pos.y != 0.0f) {
        window.pos = pos;
    } else {
        window.pos = Vec2(50.0f + impl_->window_stack.size() * 20.0f,
                         50.0f + impl_->window_stack.size() * 20.0f);
    }
    
    if (impl_->has_next_window_size) {
        window.size = impl_->next_window_size;
        impl_->has_next_window_size = false;
    } else if (size.x != 0.0f || size.y != 0.0f) {
        window.size = size;
    } else {
        window.size = Vec2(400.0f, 300.0f);
    }
    
    window.scroll_y = 0.0f;
    window.content_y = 0.0f;
    
    impl_->window_stack.push_back(window);
    impl_->current_window = &impl_->window_stack.back();
    impl_->cursor_pos = Vec2(window.pos.x + impl_->padding,
                            window.pos.y + 30.0f + impl_->padding);
    impl_->same_line_mode = false;
    
    // Render window background
    RenderRect(window.pos.x, window.pos.y, window.size.x, window.size.y,
              Color(0.15f, 0.15f, 0.2f, 0.95f));
    
    // Render window title bar
    RenderRect(window.pos.x, window.pos.y, window.size.x, 25.0f,
              Color(0.2f, 0.2f, 0.3f, 1.0f));
    RenderText(window.pos.x + impl_->padding, window.pos.y + 18.0f, title,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    // Render window border
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(window.pos.x, window.pos.y);
    glVertex2f(window.pos.x + window.size.x, window.pos.y);
    glVertex2f(window.pos.x + window.size.x, window.pos.y + window.size.y);
    glVertex2f(window.pos.x, window.pos.y + window.size.y);
    glEnd();
    
    return true;
}

void Context::EndWindow() {
    if (!impl_->window_stack.empty()) {
        impl_->window_stack.pop_back();
        impl_->current_window = impl_->window_stack.empty() ? nullptr :
                                &impl_->window_stack.back();
    }
}

bool Context::BeginMainMenuBar() {
    impl_->in_menu_bar = true;
    impl_->cursor_pos = Vec2(0.0f, 0.0f);
    
    // Render menu bar background
    RenderRect(0, 0, impl_->window_width, 25.0f, Color(0.2f, 0.2f, 0.3f, 1.0f));
    
    return true;
}

void Context::EndMainMenuBar() {
    impl_->in_menu_bar = false;
}

bool Context::BeginMenu(const char* label) {
    if (!impl_->in_menu_bar) return false;
    
    float text_width = strlen(label) * 7.0f + impl_->padding * 2;
    
    // Check if mouse is hovering
    bool hovering = (impl_->mouse_x >= impl_->cursor_pos.x &&
                    impl_->mouse_x <= impl_->cursor_pos.x + text_width &&
                    impl_->mouse_y >= impl_->cursor_pos.y &&
                    impl_->mouse_y <= impl_->cursor_pos.y + 25.0f);
    
    if (hovering) {
        RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, text_width, 25.0f,
                  Color(0.3f, 0.3f, 0.4f, 1.0f));
    }
    
    RenderText(impl_->cursor_pos.x + impl_->padding, impl_->cursor_pos.y + 18.0f, label,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.x += text_width;
    impl_->in_menu = hovering && impl_->mouse_clicked;
    
    return impl_->in_menu;
}

void Context::EndMenu() {
    impl_->in_menu = false;
}

bool Context::MenuItem(const char* label, const char* shortcut, bool* selected) {
    if (!impl_->in_menu) return false;
    
    // Simple menu item rendering - in a real implementation, this would show a dropdown
    return false;
}

void Context::Text(const char* text) {
    if (!impl_->current_window) return;
    
    if (impl_->same_line_mode) {
        impl_->cursor_pos.x += 10.0f;
        impl_->same_line_mode = false;
    } else {
        impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    }
    
    RenderText(impl_->cursor_pos.x, impl_->cursor_pos.y, text,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.y += impl_->line_height;
}

void Context::TextColored(Color color, const char* text) {
    if (!impl_->current_window) return;
    
    if (impl_->same_line_mode) {
        impl_->cursor_pos.x += 10.0f;
        impl_->same_line_mode = false;
    } else {
        impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    }
    
    RenderText(impl_->cursor_pos.x, impl_->cursor_pos.y, text, color);
    
    impl_->cursor_pos.y += impl_->line_height;
}

void Context::Separator() {
    if (!impl_->current_window) return;
    
    impl_->cursor_pos.y += impl_->spacing;
    
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(impl_->current_window->pos.x + impl_->padding, impl_->cursor_pos.y);
    glVertex2f(impl_->current_window->pos.x + impl_->current_window->size.x - impl_->padding,
              impl_->cursor_pos.y);
    glEnd();
    
    impl_->cursor_pos.y += impl_->spacing * 2;
}

void Context::Spacing() {
    impl_->cursor_pos.y += impl_->spacing * 2;
}

bool Context::Button(const char* label) {
    if (!impl_->current_window) return false;
    
    if (impl_->same_line_mode) {
        impl_->cursor_pos.x += 10.0f;
        impl_->same_line_mode = false;
    } else {
        impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    }
    
    float button_width = std::max(strlen(label) * 7.0f + impl_->padding * 2, 80.0f);
    float button_height = 25.0f;
    
    bool hovering = (impl_->mouse_x >= impl_->cursor_pos.x &&
                    impl_->mouse_x <= impl_->cursor_pos.x + button_width &&
                    impl_->mouse_y >= impl_->cursor_pos.y &&
                    impl_->mouse_y <= impl_->cursor_pos.y + button_height);
    
    bool clicked = hovering && impl_->mouse_clicked;
    
    Color button_color = hovering ? Color(0.3f, 0.4f, 0.6f, 1.0f) :
                                   Color(0.2f, 0.3f, 0.5f, 1.0f);
    
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, button_width, button_height,
              button_color);
    
    // Button border
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + button_width, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + button_width, impl_->cursor_pos.y + button_height);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y + button_height);
    glEnd();
    
    RenderText(impl_->cursor_pos.x + impl_->padding,
              impl_->cursor_pos.y + button_height - 8.0f, label,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.y += button_height + impl_->spacing;
    
    return clicked;
}

bool Context::InputText(const char* label, char* buffer, size_t buffer_size) {
    if (!impl_->current_window) return false;
    
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    // Label
    RenderText(impl_->cursor_pos.x, impl_->cursor_pos.y, label,
              Color(0.8f, 0.8f, 0.8f, 1.0f));
    impl_->cursor_pos.y += impl_->line_height;
    
    // Input box
    float input_width = impl_->current_window->size.x - impl_->padding * 2;
    float input_height = 25.0f;
    
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, input_width, input_height,
              Color(0.1f, 0.1f, 0.15f, 1.0f));
    
    // Border
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + input_width, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + input_width, impl_->cursor_pos.y + input_height);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y + input_height);
    glEnd();
    
    // Text content
    RenderText(impl_->cursor_pos.x + impl_->padding,
              impl_->cursor_pos.y + input_height - 8.0f, buffer,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.y += input_height + impl_->spacing;
    
    return false; // No actual text input in this simplified version
}

bool Context::InputTextMultiline(const char* label, char* buffer, size_t buffer_size, Vec2 size) {
    if (!impl_->current_window) return false;
    
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    float input_width = size.x > 0 ? size.x : impl_->current_window->size.x - impl_->padding * 2;
    float input_height = size.y > 0 ? size.y : 100.0f;
    
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, input_width, input_height,
              Color(0.1f, 0.1f, 0.15f, 1.0f));
    
    // Border
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + input_width, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + input_width, impl_->cursor_pos.y + input_height);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y + input_height);
    glEnd();
    
    RenderText(impl_->cursor_pos.x + impl_->padding,
              impl_->cursor_pos.y + impl_->text_height, buffer,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.y += input_height + impl_->spacing;
    
    return false;
}

bool Context::SliderInt(const char* label, int* value, int min, int max) {
    if (!impl_->current_window) return false;
    
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    // Label
    char text[256];
    snprintf(text, sizeof(text), "%s: %d", label, *value);
    RenderText(impl_->cursor_pos.x, impl_->cursor_pos.y, text,
              Color(0.8f, 0.8f, 0.8f, 1.0f));
    impl_->cursor_pos.y += impl_->line_height;
    
    // Slider track
    float slider_width = impl_->current_window->size.x - impl_->padding * 2;
    float slider_height = 20.0f;
    
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, slider_width, slider_height,
              Color(0.2f, 0.2f, 0.25f, 1.0f));
    
    // Slider thumb
    float normalized = static_cast<float>(*value - min) / (max - min);
    float thumb_x = impl_->cursor_pos.x + normalized * (slider_width - 20.0f);
    RenderRect(thumb_x, impl_->cursor_pos.y, 20.0f, slider_height,
              Color(0.3f, 0.4f, 0.6f, 1.0f));
    
    impl_->cursor_pos.y += slider_height + impl_->spacing;
    
    return false;
}

void Context::ProgressBar(float fraction, Vec2 size, const char* overlay) {
    if (!impl_->current_window) return;
    
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    float bar_width = size.x > 0 ? size.x : impl_->current_window->size.x - impl_->padding * 2;
    float bar_height = size.y > 0 ? size.y : 20.0f;
    
    // Background
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, bar_width, bar_height,
              Color(0.2f, 0.2f, 0.25f, 1.0f));
    
    // Progress fill
    float fill_width = bar_width * std::min(std::max(fraction, 0.0f), 1.0f);
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, fill_width, bar_height,
              Color(0.2f, 0.6f, 0.3f, 1.0f));
    
    // Border
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + bar_width, impl_->cursor_pos.y);
    glVertex2f(impl_->cursor_pos.x + bar_width, impl_->cursor_pos.y + bar_height);
    glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y + bar_height);
    glEnd();
    
    // Overlay text
    if (overlay) {
        RenderText(impl_->cursor_pos.x + bar_width / 2 - strlen(overlay) * 3.5f,
                  impl_->cursor_pos.y + bar_height - 8.0f, overlay,
                  Color(1.0f, 1.0f, 1.0f, 1.0f));
    }
    
    impl_->cursor_pos.y += bar_height + impl_->spacing;
}

bool Context::CollapsingHeader(const char* label, bool default_open) {
    if (!impl_->current_window) return false;
    
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    float header_height = 25.0f;
    float header_width = impl_->current_window->size.x - impl_->padding * 2;
    
    bool hovering = (impl_->mouse_x >= impl_->cursor_pos.x &&
                    impl_->mouse_x <= impl_->cursor_pos.x + header_width &&
                    impl_->mouse_y >= impl_->cursor_pos.y &&
                    impl_->mouse_y <= impl_->cursor_pos.y + header_height);
    
    Color bg_color = hovering ? Color(0.25f, 0.25f, 0.35f, 1.0f) :
                               Color(0.2f, 0.2f, 0.3f, 1.0f);
    
    RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, header_width, header_height, bg_color);
    
    // Arrow (simplified - always show as open for now)
    RenderText(impl_->cursor_pos.x + 5.0f, impl_->cursor_pos.y + header_height - 8.0f, "v",
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    RenderText(impl_->cursor_pos.x + 20.0f, impl_->cursor_pos.y + header_height - 8.0f, label,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.y += header_height + impl_->spacing;
    
    return true; // Always open in this simplified version
}

void Context::SameLine() {
    impl_->same_line_mode = true;
}

bool Context::BeginChild(const char* id, Vec2 size, bool border) {
    if (!impl_->current_window) return false;
    
    impl_->in_child = true;
    impl_->child_size = size;
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    float child_width = size.x > 0 ? size.x : impl_->current_window->size.x - impl_->padding * 2;
    float child_height = size.y > 0 ? size.y : 200.0f;
    
    if (border) {
        RenderRect(impl_->cursor_pos.x, impl_->cursor_pos.y, child_width, child_height,
                  Color(0.12f, 0.12f, 0.17f, 1.0f));
        
        glColor4f(0.3f, 0.3f, 0.4f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y);
        glVertex2f(impl_->cursor_pos.x + child_width, impl_->cursor_pos.y);
        glVertex2f(impl_->cursor_pos.x + child_width, impl_->cursor_pos.y + child_height);
        glVertex2f(impl_->cursor_pos.x, impl_->cursor_pos.y + child_height);
        glEnd();
    }
    
    impl_->cursor_pos.y += impl_->padding;
    
    return true;
}

void Context::EndChild() {
    if (impl_->in_child) {
        impl_->cursor_pos.y += impl_->child_size.y + impl_->spacing;
        impl_->in_child = false;
    }
}

void Context::SetNextWindowSize(Vec2 size, int condition) {
    impl_->next_window_size = size;
    impl_->next_window_condition = condition;
    impl_->has_next_window_size = true;
}

void Context::SetNextWindowPos(Vec2 pos, int condition) {
    impl_->next_window_pos = pos;
    impl_->next_window_condition = condition;
    impl_->has_next_window_pos = true;
}

void Context::BulletText(const char* text) {
    if (!impl_->current_window) return;
    
    impl_->cursor_pos.x = impl_->current_window->pos.x + impl_->padding;
    
    // Bullet point
    RenderText(impl_->cursor_pos.x, impl_->cursor_pos.y, "*",
              Color(0.6f, 0.6f, 0.6f, 1.0f));
    
    // Text
    RenderText(impl_->cursor_pos.x + 15.0f, impl_->cursor_pos.y, text,
              Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    impl_->cursor_pos.y += impl_->line_height;
}

void Context::TextWrapped(const char* text) {
    // Simplified - just render as regular text
    Text(text);
}

void Context::ShowDemoWindow(bool* open) {
    if (!open || !*open) return;
    
    SetNextWindowSize(Vec2(550, 680), FirstUseEver);
    
    if (!BeginWindow("Independent GUI Demo", open)) {
        EndWindow();
        return;
    }
    
    Text("This is a demo window for the Independent GUI system.");
    Separator();
    
    if (CollapsingHeader("Basic Elements", true)) {
        Text("This is some text");
        TextColored(Color(1.0f, 0.0f, 0.0f, 1.0f), "This is colored text");
        
        if (Button("Click Me!")) {
            // Button clicked
        }
        
        Spacing();
        ProgressBar(0.6f, Vec2(-1, 0), "60%");
    }
    
    if (CollapsingHeader("Layout")) {
        Text("Same line demo:");
        Button("Button 1"); SameLine();
        Button("Button 2"); SameLine();
        Button("Button 3");
    }
    
    EndWindow();
}

} // namespace IndependentGUI
