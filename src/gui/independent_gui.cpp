#include "independent_gui.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace IndependentGUI {

// Simple OpenGL immediate mode rendering helpers
// NOTE: This implementation uses legacy OpenGL immediate mode for simplicity.
// For production use with OpenGL 3.3+, consider migrating to:
// - Vertex Buffer Objects (VBOs)
// - Vertex Array Objects (VAOs)  
// - Modern shader-based rendering
// The current implementation works but uses deprecated APIs.
static void RenderRect(float x, float y, float w, float h, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// Simple 8x13 bitmap font data for ASCII characters 32-126
// Each character is 8 pixels wide and 13 pixels tall
static const unsigned char bitmap_font_8x13[][13] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
    // " (34)
    {0x00, 0x00, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // # (35)
    {0x00, 0x00, 0x36, 0x36, 0x7F, 0x36, 0x36, 0x7F, 0x36, 0x36, 0x00, 0x00, 0x00},
    // $ (36)
    {0x00, 0x0C, 0x3E, 0x03, 0x03, 0x1E, 0x30, 0x30, 0x1F, 0x0C, 0x00, 0x00, 0x00},
    // % (37)
    {0x00, 0x00, 0x00, 0x23, 0x33, 0x18, 0x0C, 0x06, 0x33, 0x31, 0x00, 0x00, 0x00},
    // & (38)
    {0x00, 0x00, 0x0E, 0x1B, 0x1B, 0x0E, 0x5F, 0x3B, 0x33, 0x6E, 0x00, 0x00, 0x00},
    // ' (39)
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ( (40)
    {0x00, 0x00, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00},
    // ) (41)
    {0x00, 0x00, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00, 0x00, 0x00},
    // * (42)
    {0x00, 0x00, 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    // + (43)
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},
    // , (44)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x0C, 0x00, 0x00},
    // - (45)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // . (46)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},
    // / (47)
    {0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00},
    // 0 (48)
    {0x00, 0x00, 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x63, 0x3E, 0x00, 0x00, 0x00},
    // 1 (49)
    {0x00, 0x00, 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00, 0x00, 0x00},
    // 2 (50)
    {0x00, 0x00, 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x06, 0x33, 0x3F, 0x00, 0x00, 0x00},
    // 3 (51)
    {0x00, 0x00, 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x30, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // 4 (52)
    {0x00, 0x00, 0x18, 0x1C, 0x16, 0x13, 0x7F, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00},
    // 5 (53)
    {0x00, 0x00, 0x3F, 0x03, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // 6 (54)
    {0x00, 0x00, 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // 7 (55)
    {0x00, 0x00, 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00},
    // 8 (56)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // 9 (57)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00, 0x00, 0x00},
    // : (58)
    {0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    // ; (59)
    {0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x0C, 0x00, 0x00, 0x00},
    // < (60)
    {0x00, 0x00, 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00},
    // = (61)
    {0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // > (62)
    {0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00},
    // ? (63)
    {0x00, 0x00, 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x00},
    // @ (64)
    {0x00, 0x00, 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x3B, 0x03, 0x1E, 0x00, 0x00, 0x00},
    // A (65)
    {0x00, 0x00, 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00},
    // B (66)
    {0x00, 0x00, 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00},
    // C (67)
    {0x00, 0x00, 0x3C, 0x66, 0x03, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00, 0x00, 0x00},
    // D (68)
    {0x00, 0x00, 0x1F, 0x36, 0x66, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00, 0x00, 0x00},
    // E (69)
    {0x00, 0x00, 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x46, 0x7F, 0x00, 0x00, 0x00},
    // F (70)
    {0x00, 0x00, 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x06, 0x0F, 0x00, 0x00, 0x00},
    // G (71)
    {0x00, 0x00, 0x3C, 0x66, 0x03, 0x03, 0x73, 0x63, 0x66, 0x5C, 0x00, 0x00, 0x00},
    // H (72)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00},
    // I (73)
    {0x00, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00},
    // J (74)
    {0x00, 0x00, 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // K (75)
    {0x00, 0x00, 0x67, 0x66, 0x36, 0x1E, 0x1E, 0x36, 0x66, 0x67, 0x00, 0x00, 0x00},
    // L (76)
    {0x00, 0x00, 0x0F, 0x06, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00, 0x00, 0x00},
    // M (77)
    {0x00, 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00},
    // N (78)
    {0x00, 0x00, 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00},
    // O (79)
    {0x00, 0x00, 0x1C, 0x36, 0x63, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00, 0x00, 0x00},
    // P (80)
    {0x00, 0x00, 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x0F, 0x00, 0x00, 0x00},
    // Q (81)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00, 0x00, 0x00},
    // R (82)
    {0x00, 0x00, 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x66, 0x67, 0x00, 0x00, 0x00},
    // S (83)
    {0x00, 0x00, 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x70, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // T (84)
    {0x00, 0x00, 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00},
    // U (85)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // V (86)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x00, 0x00, 0x00},
    // W (87)
    {0x00, 0x00, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x63, 0x00, 0x00, 0x00},
    // X (88)
    {0x00, 0x00, 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x63, 0x00, 0x00, 0x00},
    // Y (89)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00},
    // Z (90)
    {0x00, 0x00, 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x46, 0x63, 0x7F, 0x00, 0x00, 0x00},
    // [ (91)
    {0x00, 0x00, 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00, 0x00, 0x00},
    // \ (92)
    {0x00, 0x00, 0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00},
    // ] (93)
    {0x00, 0x00, 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00, 0x00, 0x00},
    // ^ (94)
    {0x00, 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // _ (95)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},
    // ` (96)
    {0x00, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // a (97)
    {0x00, 0x00, 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x33, 0x6E, 0x00, 0x00, 0x00},
    // b (98)
    {0x00, 0x00, 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3B, 0x00, 0x00, 0x00},
    // c (99)
    {0x00, 0x00, 0x00, 0x00, 0x1E, 0x33, 0x03, 0x03, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // d (100)
    {0x00, 0x00, 0x38, 0x30, 0x30, 0x3E, 0x33, 0x33, 0x33, 0x6E, 0x00, 0x00, 0x00},
    // e (101)
    {0x00, 0x00, 0x00, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // f (102)
    {0x00, 0x00, 0x1C, 0x36, 0x06, 0x0F, 0x06, 0x06, 0x06, 0x0F, 0x00, 0x00, 0x00},
    // g (103)
    {0x00, 0x00, 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x33, 0x1E, 0x00, 0x00},
    // h (104)
    {0x00, 0x00, 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x66, 0x67, 0x00, 0x00, 0x00},
    // i (105)
    {0x00, 0x00, 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00},
    // j (106)
    {0x00, 0x00, 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x33, 0x1E, 0x00, 0x00},
    // k (107)
    {0x00, 0x00, 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00, 0x00, 0x00},
    // l (108)
    {0x00, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00},
    // m (109)
    {0x00, 0x00, 0x00, 0x00, 0x3F, 0x6B, 0x6B, 0x6B, 0x6B, 0x63, 0x00, 0x00, 0x00},
    // n (110)
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00},
    // o (111)
    {0x00, 0x00, 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // p (112)
    {0x00, 0x00, 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00, 0x00},
    // q (113)
    {0x00, 0x00, 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x30, 0x78, 0x00, 0x00},
    // r (114)
    {0x00, 0x00, 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x06, 0x0F, 0x00, 0x00, 0x00},
    // s (115)
    {0x00, 0x00, 0x00, 0x00, 0x1E, 0x33, 0x07, 0x38, 0x33, 0x1E, 0x00, 0x00, 0x00},
    // t (116)
    {0x00, 0x00, 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00, 0x00, 0x00},
    // u (117)
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00, 0x00, 0x00},
    // v (118)
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x00, 0x00, 0x00},
    // w (119)
    {0x00, 0x00, 0x00, 0x00, 0x63, 0x6B, 0x6B, 0x6B, 0x3F, 0x36, 0x00, 0x00, 0x00},
    // x (120)
    {0x00, 0x00, 0x00, 0x00, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00},
    // y (121)
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0F, 0x00, 0x00},
    // z (122)
    {0x00, 0x00, 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x06, 0x23, 0x3F, 0x00, 0x00, 0x00},
    // { (123)
    {0x00, 0x00, 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x0C, 0x38, 0x00, 0x00, 0x00},
    // | (124)
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00},
    // } (125)
    {0x00, 0x00, 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x0C, 0x07, 0x00, 0x00, 0x00},
    // ~ (126)
    {0x00, 0x00, 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

static void RenderText(float x, float y, const char* text, Color color) {
    // Simple text rendering using GL bitmap fonts
    glColor4f(color.r, color.g, color.b, color.a);
    glRasterPos2f(x, y);
    
    // Empty bitmap for unsupported characters (just advances position)
    static const unsigned char empty_bitmap[13] = {0};
    
    // Render each character using bitmap font
    for (const char* c = text; *c != '\0'; c++) {
        unsigned char ch = (unsigned char)*c;
        
        // Only render printable ASCII characters (32-126)
        if (ch >= 32 && ch <= 126) {
            const unsigned char* bitmap = bitmap_font_8x13[ch - 32];
            // glBitmap parameters: width=8, height=13, xorig=0, yorig=2, xmove=9, ymove=0
            // xmove=9 provides 1 pixel spacing between characters
            glBitmap(8, 13, 0, 2, 9, 0, bitmap);
        } else {
            // For unsupported characters, just advance the position
            glBitmap(0, 0, 0, 0, 9, 0, empty_bitmap);
        }
    }
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
    
    // NOTE: Actual keyboard input handling not implemented in this simplified version.
    // For full functionality, integrate with GLFW's text input callbacks:
    // - glfwSetCharCallback for text input
    // - glfwSetKeyCallback for special keys
    // - Track focused widget and modify buffer accordingly
    return false;
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
