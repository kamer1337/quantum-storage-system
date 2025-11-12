/**
 * API GUI - Pure C 5D Renderer
 * =============================
 * 
 * A lightweight, pure C implementation of a 5D rendering system for GUI applications.
 * 
 * The "5D" concept refers to:
 * 1-3. Spatial dimensions (X, Y, Z) for positioning and depth
 * 4. Time dimension (animations, transitions, dynamic updates)
 * 5. Interaction dimension (user input states, hover effects, focus)
 * 
 * This renderer provides:
 * - Pure C API (C99 compatible)
 * - No external GUI library dependencies (only GLFW for windowing and OpenGL for rendering)
 * - Immediate mode rendering paradigm
 * - Built-in text rendering with bitmap fonts
 * - Full control over rendering pipeline
 * 
 * Features:
 * - Windows and panels
 * - Buttons, text, input fields, sliders
 * - Progress bars and separators
 * - Menu bars and popup menus
 * - Collapsing headers
 * - Text wrapping and formatting
 * - Color support
 * - Layout management
 */

#ifndef API_GUI_H
#define API_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct GLFWwindow GLFWwindow;
typedef struct APIGUIContext APIGUIContext;

/* Color structure */
typedef struct {
    float r, g, b, a;
} APIGUIColor;

/* Vector2 structure */
typedef struct {
    float x, y;
} APIGUIVec2;

/* Condition flags for SetNextWindow* functions */
typedef enum {
    APIGUI_COND_ALWAYS = 0,
    APIGUI_COND_ONCE = 1,
    APIGUI_COND_FIRST_USE_EVER = 2,
    APIGUI_COND_APPEARING = 3
} APIGUICondition;

/* Tree node flags */
typedef enum {
    APIGUI_TREENODE_NONE = 0,
    APIGUI_TREENODE_DEFAULT_OPEN = 1 << 0
} APIGUITreeNodeFlags;

/* ============================================================================
 * Context Management
 * ============================================================================ */

/**
 * Create a new API GUI context.
 * Must be called before any other API GUI functions.
 */
APIGUIContext* apigui_create_context(void);

/**
 * Destroy the API GUI context and free all resources.
 */
void apigui_destroy_context(APIGUIContext* ctx);

/**
 * Initialize the API GUI with a GLFW window.
 * Returns true on success, false on failure.
 */
bool apigui_initialize(APIGUIContext* ctx, GLFWwindow* window);

/**
 * Shutdown the API GUI and release resources.
 */
void apigui_shutdown(APIGUIContext* ctx);

/* ============================================================================
 * Frame Management
 * ============================================================================ */

/**
 * Begin a new frame. Call this once per frame before rendering any GUI elements.
 */
void apigui_new_frame(APIGUIContext* ctx);

/**
 * Render the GUI. Call this after all GUI elements have been defined.
 */
void apigui_render(APIGUIContext* ctx);

/* ============================================================================
 * Window Management
 * ============================================================================ */

/**
 * Begin a new window. Returns false if the window is collapsed.
 * Must be paired with apigui_end_window().
 */
bool apigui_begin_window(APIGUIContext* ctx, const char* title, bool* open, 
                          APIGUIVec2 pos, APIGUIVec2 size);

/**
 * End the current window.
 */
void apigui_end_window(APIGUIContext* ctx);

/**
 * Set size for next window.
 */
void apigui_set_next_window_size(APIGUIContext* ctx, APIGUIVec2 size, APIGUICondition cond);

/**
 * Set position for next window.
 */
void apigui_set_next_window_pos(APIGUIContext* ctx, APIGUIVec2 pos, APIGUICondition cond);

/* ============================================================================
 * Menu Bar
 * ============================================================================ */

/**
 * Begin the main menu bar. Returns true if the menu bar is visible.
 */
bool apigui_begin_main_menu_bar(APIGUIContext* ctx);

/**
 * End the main menu bar.
 */
void apigui_end_main_menu_bar(APIGUIContext* ctx);

/**
 * Begin a menu. Returns true if the menu is open.
 */
bool apigui_begin_menu(APIGUIContext* ctx, const char* label);

/**
 * End a menu.
 */
void apigui_end_menu(APIGUIContext* ctx);

/**
 * Create a menu item. Returns true if clicked.
 */
bool apigui_menu_item(APIGUIContext* ctx, const char* label, const char* shortcut, bool* selected);

/* ============================================================================
 * Widgets
 * ============================================================================ */

/**
 * Display text.
 */
void apigui_text(APIGUIContext* ctx, const char* text);

/**
 * Display colored text.
 */
void apigui_text_colored(APIGUIContext* ctx, APIGUIColor color, const char* text);

/**
 * Display wrapped text.
 */
void apigui_text_wrapped(APIGUIContext* ctx, const char* text);

/**
 * Display bulleted text.
 */
void apigui_bullet_text(APIGUIContext* ctx, const char* text);

/**
 * Create a button. Returns true if clicked.
 */
bool apigui_button(APIGUIContext* ctx, const char* label);

/**
 * Create an input text field. Returns true if the value changed.
 */
bool apigui_input_text(APIGUIContext* ctx, const char* label, char* buffer, size_t buffer_size);

/**
 * Create a multiline input text field. Returns true if the value changed.
 */
bool apigui_input_text_multiline(APIGUIContext* ctx, const char* label, char* buffer, 
                                   size_t buffer_size, APIGUIVec2 size);

/**
 * Create an integer slider. Returns true if the value changed.
 */
bool apigui_slider_int(APIGUIContext* ctx, const char* label, int* value, int min, int max);

/**
 * Display a progress bar.
 */
void apigui_progress_bar(APIGUIContext* ctx, float fraction, APIGUIVec2 size, const char* overlay);

/**
 * Create a collapsing header. Returns true if expanded.
 */
bool apigui_collapsing_header(APIGUIContext* ctx, const char* label, bool default_open);

/**
 * Add a horizontal separator.
 */
void apigui_separator(APIGUIContext* ctx);

/**
 * Add vertical spacing.
 */
void apigui_spacing(APIGUIContext* ctx);

/**
 * Place next item on same line as previous item.
 */
void apigui_same_line(APIGUIContext* ctx);

/* ============================================================================
 * Child Windows
 * ============================================================================ */

/**
 * Begin a child window. Returns true if the child is visible.
 */
bool apigui_begin_child(APIGUIContext* ctx, const char* id, APIGUIVec2 size, bool border);

/**
 * End a child window.
 */
void apigui_end_child(APIGUIContext* ctx);

/* ============================================================================
 * Demo Window
 * ============================================================================ */

/**
 * Show a demo window with all available widgets and features.
 */
void apigui_show_demo_window(APIGUIContext* ctx, bool* open);

/* ============================================================================
 * Color Helpers
 * ============================================================================ */

static inline APIGUIColor apigui_color(float r, float g, float b, float a) {
    APIGUIColor c = {r, g, b, a};
    return c;
}

static inline APIGUIVec2 apigui_vec2(float x, float y) {
    APIGUIVec2 v = {x, y};
    return v;
}

#ifdef __cplusplus
}
#endif

#endif /* API_GUI_H */
