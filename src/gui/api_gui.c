/**
 * API GUI - Pure C 5D Renderer Implementation
 * ============================================
 */

#include "api_gui.h"
#include <GLFW/glfw3.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================================
 * Constants and Configuration
 * ============================================================================ */

#define MAX_WINDOWS 32
#define MAX_MENU_ITEMS 64
#define FONT_WIDTH 8
#define FONT_HEIGHT 13
#define MENU_BAR_HEIGHT 20.0f
#define WINDOW_PADDING 10.0f
#define ITEM_SPACING 5.0f
#define BUTTON_PADDING 5.0f

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

typedef struct {
    char title[128];
    float x, y, width, height;
    bool visible;
    bool collapsed;
    bool has_next_size;
    bool has_next_pos;
    APIGUIVec2 next_size;
    APIGUIVec2 next_pos;
    APIGUICondition next_size_cond;
    APIGUICondition next_pos_cond;
    float cursor_x, cursor_y;
    bool same_line_next;
} Window;

typedef struct {
    bool in_menu_bar;
    bool in_menu;
    float menu_x, menu_y;
    int menu_depth;
    char current_menu[64];
} MenuState;

typedef struct APIGUIContext {
    GLFWwindow* window;
    int display_width, display_height;
    
    /* Window management */
    Window windows[MAX_WINDOWS];
    int window_count;
    Window* current_window;
    
    /* Menu state */
    MenuState menu_state;
    
    /* Input state */
    double mouse_x, mouse_y;
    bool mouse_down[3];
    bool mouse_clicked[3];
    
    /* Rendering state */
    bool initialized;
} APIGUIContext;

/* ============================================================================
 * Font Data - Simple 8x13 Bitmap Font
 * ============================================================================ */

/* Simplified 8x13 bitmap font for ASCII 32-126 
 * Each character is 8 pixels wide and 13 pixels tall
 * Each byte represents one row of 8 pixels
 */
static const unsigned char font_bitmap_8x13[95][13] = {
    /* Space (32) */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ! (33) */
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
    /* " (34) */
    {0x00, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* # (35) */
    {0x00, 0x00, 0x36, 0x36, 0x7F, 0x36, 0x36, 0x7F, 0x36, 0x36, 0x00, 0x00, 0x00},
    /* $ (36) */
    {0x00, 0x18, 0x3E, 0x03, 0x03, 0x1E, 0x30, 0x30, 0x1F, 0x0C, 0x00, 0x00, 0x00},
    /* % (37) */
    {0x00, 0x00, 0x63, 0x33, 0x18, 0x0C, 0x06, 0x33, 0x63, 0x00, 0x00, 0x00, 0x00},
    /* & (38) */
    {0x00, 0x00, 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00, 0x00, 0x00, 0x00},
    /* ' (39) */
    {0x00, 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ( (40) */
    {0x00, 0x18, 0x0C, 0x06, 0x06, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00, 0x00, 0x00},
    /* ) (41) */
    {0x00, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00},
    /* * (42) */
    {0x00, 0x00, 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* + (43) */
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* , (44) */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x0C, 0x00, 0x00},
    /* - (45) */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* . (46) */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},
    /* / (47) */
    {0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 0-9 (48-57) */
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x60, 0x30, 0x18, 0x0C, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x60, 0x38, 0x60, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x38, 0x3C, 0x36, 0x7E, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x06, 0x3E, 0x60, 0x60, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x60, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x7C, 0x60, 0x30, 0x1E, 0x00, 0x00, 0x00, 0x00},
    /* : ; < = > ? @ (58-64) */
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x0C, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x30, 0x18, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x76, 0x6E, 0x6E, 0x06, 0x3C, 0x00, 0x00, 0x00, 0x00},
    /* A-Z (65-90) */
    {0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x1E, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x06, 0x76, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x78, 0x30, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x36, 0x1E, 0x0E, 0x1E, 0x36, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x6E, 0x7E, 0x7E, 0x76, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x60, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x1E, 0x36, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x66, 0x06, 0x3C, 0x60, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x7E, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x7E, 0x00, 0x00, 0x00, 0x00},
    /* [ \ ] ^ _ ` (91-96) */
    {0x00, 0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},
    {0x00, 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* a-z (97-122) */
    {0x00, 0x00, 0x00, 0x00, 0x3C, 0x60, 0x7C, 0x66, 0x7C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x06, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x38, 0x0C, 0x0C, 0x3E, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x66, 0x3C, 0x00, 0x00},
    {0x00, 0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x00, 0x1C, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x00, 0x38, 0x30, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00, 0x00},
    {0x00, 0x00, 0x06, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x36, 0x7F, 0x6B, 0x6B, 0x63, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3E, 0x66, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x3C, 0x06, 0x3C, 0x60, 0x3C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x0C, 0x0C, 0x3E, 0x0C, 0x0C, 0x0C, 0x38, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x63, 0x6B, 0x6B, 0x7F, 0x36, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x66, 0x3C, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x7E, 0x30, 0x18, 0x0C, 0x7E, 0x00, 0x00, 0x00, 0x00},
    /* { | } ~ (123-126) */
    {0x00, 0x00, 0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x4C, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

/* ============================================================================
 * Rendering Helpers
 * ============================================================================ */

static void render_rect(float x, float y, float w, float h, APIGUIColor color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void render_rect_outline(float x, float y, float w, float h, APIGUIColor color, float thickness) {
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void render_text(float x, float y, const char* text, APIGUIColor color) {
    glColor4f(color.r, color.g, color.b, color.a);
    
    float cx = x;
    float cy = y;
    
    for (const char* p = text; *p; p++) {
        if (*p == '\n') {
            cx = x;
            cy += FONT_HEIGHT + 2;
            continue;
        }
        
        /* Get character index (ASCII 32-126) */
        int char_index = (int)(*p) - 32;
        if (char_index < 0 || char_index >= 95) {
            cx += FONT_WIDTH;
            continue;
        }
        
        /* Render character bitmap */
        for (int row = 0; row < 13; row++) {
            unsigned char bitmap_row = font_bitmap_8x13[char_index][row];
            for (int bit = 0; bit < 8; bit++) {
                if (bitmap_row & (1 << (7 - bit))) {
                    /* Draw pixel */
                    render_rect(cx + bit, cy + row, 1.0f, 1.0f, color);
                }
            }
        }
        
        cx += FONT_WIDTH;
    }
}

static int text_width(const char* text) {
    int width = 0;
    for (const char* p = text; *p; p++) {
        if (*p != '\n') {
            width++;
        }
    }
    return width * FONT_WIDTH;
}

static bool point_in_rect(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

/* ============================================================================
 * Context Management
 * ============================================================================ */

APIGUIContext* apigui_create_context(void) {
    APIGUIContext* ctx = (APIGUIContext*)calloc(1, sizeof(APIGUIContext));
    if (!ctx) return NULL;
    
    ctx->window_count = 0;
    ctx->current_window = NULL;
    ctx->initialized = false;
    
    memset(&ctx->menu_state, 0, sizeof(MenuState));
    memset(ctx->mouse_down, 0, sizeof(ctx->mouse_down));
    memset(ctx->mouse_clicked, 0, sizeof(ctx->mouse_clicked));
    
    return ctx;
}

void apigui_destroy_context(APIGUIContext* ctx) {
    if (ctx) {
        apigui_shutdown(ctx);
        free(ctx);
    }
}

bool apigui_initialize(APIGUIContext* ctx, GLFWwindow* window) {
    if (!ctx || !window) return false;
    
    ctx->window = window;
    ctx->initialized = true;
    
    /* Get framebuffer size */
    glfwGetFramebufferSize(window, &ctx->display_width, &ctx->display_height);
    
    /* Setup OpenGL state */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    return true;
}

void apigui_shutdown(APIGUIContext* ctx) {
    if (ctx) {
        ctx->initialized = false;
        ctx->window = NULL;
    }
}

/* ============================================================================
 * Frame Management
 * ============================================================================ */

void apigui_new_frame(APIGUIContext* ctx) {
    if (!ctx || !ctx->initialized) return;
    
    /* Get window size */
    glfwGetFramebufferSize(ctx->window, &ctx->display_width, &ctx->display_height);
    
    /* Get mouse state */
    glfwGetCursorPos(ctx->window, &ctx->mouse_x, &ctx->mouse_y);
    
    for (int i = 0; i < 3; i++) {
        bool down = glfwGetMouseButton(ctx->window, GLFW_MOUSE_BUTTON_1 + i) == GLFW_PRESS;
        ctx->mouse_clicked[i] = down && !ctx->mouse_down[i];
        ctx->mouse_down[i] = down;
    }
    
    /* Setup projection */
    glViewport(0, 0, ctx->display_width, ctx->display_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, ctx->display_width, ctx->display_height, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Reset state for new frame */
    ctx->menu_state.in_menu_bar = false;
    ctx->menu_state.in_menu = false;
    ctx->menu_state.menu_depth = 0;
}

void apigui_render(APIGUIContext* ctx) {
    if (!ctx || !ctx->initialized) return;
    
    /* Rendering is done immediately in each widget call */
    /* This function is kept for API consistency */
}

/* ============================================================================
 * Window Management
 * ============================================================================ */

bool apigui_begin_window(APIGUIContext* ctx, const char* title, bool* open,
                         APIGUIVec2 pos, APIGUIVec2 size) {
    if (!ctx) return false;
    
    /* Find or create window */
    Window* win = NULL;
    for (int i = 0; i < ctx->window_count; i++) {
        if (strcmp(ctx->windows[i].title, title) == 0) {
            win = &ctx->windows[i];
            break;
        }
    }
    
    if (!win && ctx->window_count < MAX_WINDOWS) {
        win = &ctx->windows[ctx->window_count++];
        strncpy(win->title, title, sizeof(win->title) - 1);
        win->visible = true;
        win->collapsed = false;
        win->x = pos.x > 0 ? pos.x : 100.0f;
        win->y = pos.y > 0 ? pos.y : 100.0f;
        win->width = size.x > 0 ? size.x : 400.0f;
        win->height = size.y > 0 ? size.y : 300.0f;
    }
    
    if (!win) return false;
    
    /* Apply next window settings */
    if (win->has_next_pos) {
        win->x = win->next_pos.x;
        win->y = win->next_pos.y;
        win->has_next_pos = false;
    }
    
    if (win->has_next_size) {
        win->width = win->next_size.x;
        win->height = win->next_size.y;
        win->has_next_size = false;
    }
    
    ctx->current_window = win;
    
    /* Initialize cursor position */
    win->cursor_x = win->x + WINDOW_PADDING;
    win->cursor_y = win->y + 30.0f; /* Leave space for title bar */
    win->same_line_next = false;
    
    if (!win->visible || (open && !*open)) {
        ctx->current_window = NULL;
        return false;
    }
    
    /* Render window background */
    render_rect(win->x, win->y, win->width, win->height, apigui_color(0.15f, 0.15f, 0.15f, 0.95f));
    
    /* Render window title bar */
    render_rect(win->x, win->y, win->width, 25.0f, apigui_color(0.2f, 0.2f, 0.3f, 1.0f));
    render_text(win->x + 5, win->y + 6, title, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    /* Render window border */
    render_rect_outline(win->x, win->y, win->width, win->height, apigui_color(0.4f, 0.4f, 0.4f, 1.0f), 1.0f);
    
    return true;
}

void apigui_end_window(APIGUIContext* ctx) {
    if (ctx) {
        ctx->current_window = NULL;
    }
}

void apigui_set_next_window_size(APIGUIContext* ctx, APIGUIVec2 size, APIGUICondition cond) {
    if (!ctx) return;
    
    /* Store next window size - will be applied in begin_window */
    /* For simplicity, we'll apply to the next window created or accessed */
    if (ctx->window_count > 0) {
        Window* win = &ctx->windows[ctx->window_count - 1];
        win->next_size = size;
        win->next_size_cond = cond;
        win->has_next_size = true;
    }
}

void apigui_set_next_window_pos(APIGUIContext* ctx, APIGUIVec2 pos, APIGUICondition cond) {
    if (!ctx) return;
    
    /* Store next window position - will be applied in begin_window */
    if (ctx->window_count > 0) {
        Window* win = &ctx->windows[ctx->window_count - 1];
        win->next_pos = pos;
        win->next_pos_cond = cond;
        win->has_next_pos = true;
    }
}

/* ============================================================================
 * Menu Bar
 * ============================================================================ */

bool apigui_begin_main_menu_bar(APIGUIContext* ctx) {
    if (!ctx) return false;
    
    ctx->menu_state.in_menu_bar = true;
    ctx->menu_state.menu_x = 0.0f;
    ctx->menu_state.menu_y = 0.0f;
    
    /* Render menu bar background */
    render_rect(0, 0, (float)ctx->display_width, MENU_BAR_HEIGHT,
                apigui_color(0.2f, 0.2f, 0.25f, 1.0f));
    
    return true;
}

void apigui_end_main_menu_bar(APIGUIContext* ctx) {
    if (ctx) {
        ctx->menu_state.in_menu_bar = false;
    }
}

bool apigui_begin_menu(APIGUIContext* ctx, const char* label) {
    if (!ctx) return false;
    
    float label_width = text_width(label) + 20.0f;
    float menu_x = ctx->menu_state.menu_x;
    float menu_y = ctx->menu_state.menu_y;
    
    /* Check if mouse is over menu */
    bool hovered = point_in_rect((float)ctx->mouse_x, (float)ctx->mouse_y,
                                 menu_x, menu_y, label_width, MENU_BAR_HEIGHT);
    
    /* Render menu label */
    APIGUIColor bg_color = hovered ? apigui_color(0.3f, 0.3f, 0.4f, 1.0f) :
                                      apigui_color(0.2f, 0.2f, 0.25f, 1.0f);
    render_rect(menu_x, menu_y, label_width, MENU_BAR_HEIGHT, bg_color);
    render_text(menu_x + 10, menu_y + 4, label, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    ctx->menu_state.menu_x += label_width;
    
    /* For simplicity, menus open on click */
    bool is_open = hovered && ctx->mouse_clicked[0];
    
    if (is_open) {
        ctx->menu_state.in_menu = true;
        strncpy(ctx->menu_state.current_menu, label, sizeof(ctx->menu_state.current_menu) - 1);
    }
    
    return is_open;
}

void apigui_end_menu(APIGUIContext* ctx) {
    if (ctx) {
        ctx->menu_state.in_menu = false;
    }
}

bool apigui_menu_item(APIGUIContext* ctx, const char* label, const char* shortcut, bool* selected) {
    if (!ctx || !ctx->menu_state.in_menu) return false;
    
    /* Calculate menu item dimensions */
    float item_width = 200.0f;
    float item_height = 25.0f;
    float menu_x = ctx->menu_state.menu_x - item_width;  /* Position under menu */
    float menu_y = MENU_BAR_HEIGHT + ctx->menu_state.menu_depth * item_height;
    
    /* Check if mouse is over menu item */
    bool hovered = point_in_rect((float)ctx->mouse_x, (float)ctx->mouse_y,
                                 menu_x, menu_y, item_width, item_height);
    bool clicked = hovered && ctx->mouse_clicked[0];
    
    /* Render menu item background */
    APIGUIColor bg_color = hovered ? apigui_color(0.3f, 0.3f, 0.4f, 1.0f) :
                                      apigui_color(0.25f, 0.25f, 0.3f, 1.0f);
    render_rect(menu_x, menu_y, item_width, item_height, bg_color);
    
    /* Render label */
    render_text(menu_x + 10, menu_y + 6, label, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    /* Render shortcut if provided */
    if (shortcut) {
        float shortcut_x = menu_x + item_width - text_width(shortcut) - 10;
        render_text(shortcut_x, menu_y + 6, shortcut, apigui_color(0.7f, 0.7f, 0.7f, 1.0f));
    }
    
    /* Render checkmark if selected */
    if (selected && *selected) {
        render_text(menu_x + 2, menu_y + 6, "*", apigui_color(1.0f, 1.0f, 0.0f, 1.0f));
    }
    
    /* Increment menu depth for next item */
    ctx->menu_state.menu_depth++;
    
    /* Toggle selection if clicked and selected pointer is provided */
    if (clicked && selected) {
        *selected = !(*selected);
    }
    
    return clicked;
}

/* ============================================================================
 * Widgets
 * ============================================================================ */

static void advance_cursor(Window* win, float height) {
    if (win->same_line_next) {
        win->cursor_x += height; /* Use height as width for same line */
        win->same_line_next = false;
    } else {
        win->cursor_y += height + ITEM_SPACING;
        win->cursor_x = win->x + WINDOW_PADDING;
    }
}

void apigui_text(APIGUIContext* ctx, const char* text) {
    apigui_text_colored(ctx, apigui_color(1.0f, 1.0f, 1.0f, 1.0f), text);
}

void apigui_text_colored(APIGUIContext* ctx, APIGUIColor color, const char* text) {
    if (!ctx || !ctx->current_window) return;
    
    Window* win = ctx->current_window;
    render_text(win->cursor_x, win->cursor_y, text, color);
    advance_cursor(win, FONT_HEIGHT);
}

void apigui_text_wrapped(APIGUIContext* ctx, const char* text) {
    apigui_text(ctx, text);
}

void apigui_bullet_text(APIGUIContext* ctx, const char* text) {
    if (!ctx || !ctx->current_window) return;
    
    Window* win = ctx->current_window;
    render_text(win->cursor_x, win->cursor_y, "* ", apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    render_text(win->cursor_x + 16, win->cursor_y, text, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    advance_cursor(win, FONT_HEIGHT);
}

bool apigui_button(APIGUIContext* ctx, const char* label) {
    if (!ctx || !ctx->current_window) return false;
    
    Window* win = ctx->current_window;
    float button_width = text_width(label) + BUTTON_PADDING * 2;
    float button_height = FONT_HEIGHT + BUTTON_PADDING * 2;
    
    bool hovered = point_in_rect((float)ctx->mouse_x, (float)ctx->mouse_y,
                                 win->cursor_x, win->cursor_y, button_width, button_height);
    bool clicked = hovered && ctx->mouse_clicked[0];
    
    /* Render button */
    APIGUIColor bg_color = hovered ? apigui_color(0.4f, 0.4f, 0.5f, 1.0f) :
                                      apigui_color(0.3f, 0.3f, 0.35f, 1.0f);
    render_rect(win->cursor_x, win->cursor_y, button_width, button_height, bg_color);
    render_rect_outline(win->cursor_x, win->cursor_y, button_width, button_height,
                        apigui_color(0.5f, 0.5f, 0.6f, 1.0f), 1.0f);
    render_text(win->cursor_x + BUTTON_PADDING, win->cursor_y + BUTTON_PADDING,
                label, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    advance_cursor(win, button_height);
    
    return clicked;
}

bool apigui_input_text(APIGUIContext* ctx, const char* label, char* buffer, size_t buffer_size) {
    if (!ctx || !ctx->current_window) return false;
    
    Window* win = ctx->current_window;
    
    /* Render label */
    render_text(win->cursor_x, win->cursor_y, label, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    float label_width = text_width(label) + 10.0f;
    
    /* Render input box */
    float box_width = 200.0f;
    float box_height = FONT_HEIGHT + 6.0f;
    render_rect(win->cursor_x + label_width, win->cursor_y - 3, box_width, box_height,
                apigui_color(0.1f, 0.1f, 0.1f, 1.0f));
    render_rect_outline(win->cursor_x + label_width, win->cursor_y - 3, box_width, box_height,
                        apigui_color(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
    render_text(win->cursor_x + label_width + 5, win->cursor_y, buffer,
                apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    advance_cursor(win, box_height);
    
    return false; /* No input handling in this simple implementation */
}

bool apigui_input_text_multiline(APIGUIContext* ctx, const char* label, char* buffer,
                                   size_t buffer_size, APIGUIVec2 size) {
    return apigui_input_text(ctx, label, buffer, buffer_size);
}

bool apigui_slider_int(APIGUIContext* ctx, const char* label, int* value, int min, int max) {
    if (!ctx || !ctx->current_window) return false;
    
    Window* win = ctx->current_window;
    
    /* Render label */
    render_text(win->cursor_x, win->cursor_y, label, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    /* Render slider */
    float slider_width = 200.0f;
    float slider_height = 20.0f;
    float slider_x = win->cursor_x + text_width(label) + 10.0f;
    
    render_rect(slider_x, win->cursor_y - 5, slider_width, slider_height,
                apigui_color(0.2f, 0.2f, 0.2f, 1.0f));
    
    /* Calculate slider position */
    float range = (float)(max - min);
    float pos = (*value - min) / range;
    float knob_x = slider_x + pos * (slider_width - 10.0f);
    
    render_rect(knob_x, win->cursor_y - 5, 10.0f, slider_height,
                apigui_color(0.5f, 0.5f, 0.6f, 1.0f));
    
    /* Render value */
    char value_str[32];
    snprintf(value_str, sizeof(value_str), "%d", *value);
    render_text(slider_x + slider_width + 10, win->cursor_y, value_str,
                apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    advance_cursor(win, slider_height);
    
    return false;
}

void apigui_progress_bar(APIGUIContext* ctx, float fraction, APIGUIVec2 size, const char* overlay) {
    if (!ctx || !ctx->current_window) return;
    
    Window* win = ctx->current_window;
    float bar_width = size.x > 0 ? size.x : win->width - 2 * WINDOW_PADDING;
    float bar_height = size.y > 0 ? size.y : 20.0f;
    
    /* Clamp fraction */
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    
    /* Render background */
    render_rect(win->cursor_x, win->cursor_y, bar_width, bar_height,
                apigui_color(0.2f, 0.2f, 0.2f, 1.0f));
    
    /* Render progress */
    render_rect(win->cursor_x, win->cursor_y, bar_width * fraction, bar_height,
                apigui_color(0.2f, 0.6f, 0.2f, 1.0f));
    
    /* Render overlay text */
    if (overlay) {
        float text_x = win->cursor_x + (bar_width - text_width(overlay)) / 2;
        render_text(text_x, win->cursor_y + 4, overlay, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    }
    
    advance_cursor(win, bar_height);
}

bool apigui_collapsing_header(APIGUIContext* ctx, const char* label, bool default_open) {
    if (!ctx || !ctx->current_window) return default_open;
    
    Window* win = ctx->current_window;
    float header_height = FONT_HEIGHT + 4;
    
    /* Render header background */
    render_rect(win->cursor_x, win->cursor_y, win->width - 2 * WINDOW_PADDING, header_height,
                apigui_color(0.25f, 0.25f, 0.3f, 1.0f));
    
    /* Render arrow and label */
    const char* arrow = default_open ? "v " : "> ";
    render_text(win->cursor_x + 5, win->cursor_y + 2, arrow, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    render_text(win->cursor_x + 20, win->cursor_y + 2, label, apigui_color(1.0f, 1.0f, 1.0f, 1.0f));
    
    advance_cursor(win, header_height);
    
    return default_open;
}

void apigui_separator(APIGUIContext* ctx) {
    if (!ctx || !ctx->current_window) return;
    
    Window* win = ctx->current_window;
    float sep_width = win->width - 2 * WINDOW_PADDING;
    
    render_rect(win->cursor_x, win->cursor_y, sep_width, 1.0f,
                apigui_color(0.4f, 0.4f, 0.4f, 1.0f));
    
    advance_cursor(win, 1.0f);
}

void apigui_spacing(APIGUIContext* ctx) {
    if (!ctx || !ctx->current_window) return;
    
    Window* win = ctx->current_window;
    advance_cursor(win, ITEM_SPACING);
}

void apigui_same_line(APIGUIContext* ctx) {
    if (!ctx || !ctx->current_window) return;
    
    ctx->current_window->same_line_next = true;
}

/* ============================================================================
 * Child Windows
 * ============================================================================ */

bool apigui_begin_child(APIGUIContext* ctx, const char* id, APIGUIVec2 size, bool border) {
    if (!ctx || !ctx->current_window) return false;
    
    Window* win = ctx->current_window;
    float child_width = size.x > 0 ? size.x : win->width - 2 * WINDOW_PADDING;
    float child_height = size.y > 0 ? size.y : 200.0f;
    
    if (border) {
        render_rect_outline(win->cursor_x, win->cursor_y, child_width, child_height,
                            apigui_color(0.4f, 0.4f, 0.4f, 1.0f), 1.0f);
    }
    
    /* For simplicity, just adjust cursor */
    win->cursor_x += 5;
    win->cursor_y += 5;
    
    return true;
}

void apigui_end_child(APIGUIContext* ctx) {
    if (!ctx || !ctx->current_window) return;
    
    Window* win = ctx->current_window;
    win->cursor_x = win->x + WINDOW_PADDING;
}

/* ============================================================================
 * Demo Window
 * ============================================================================ */

void apigui_show_demo_window(APIGUIContext* ctx, bool* open) {
    if (!ctx || !open || !*open) return;
    
    if (apigui_begin_window(ctx, "API GUI Demo", open, apigui_vec2(50, 50), apigui_vec2(500, 400))) {
        apigui_text(ctx, "Welcome to API GUI Demo!");
        apigui_separator(ctx);
        
        apigui_text_colored(ctx, apigui_color(1.0f, 1.0f, 0.0f, 1.0f), "This is colored text");
        
        if (apigui_button(ctx, "Click Me!")) {
            /* Button was clicked */
        }
        
        apigui_separator(ctx);
        
        static int slider_val = 50;
        apigui_slider_int(ctx, "Slider", &slider_val, 0, 100);
        
        apigui_progress_bar(ctx, 0.75f, apigui_vec2(-1, 0), "75%");
        
        if (apigui_collapsing_header(ctx, "Collapsing Section", true)) {
            apigui_bullet_text(ctx, "Item 1");
            apigui_bullet_text(ctx, "Item 2");
            apigui_bullet_text(ctx, "Item 3");
        }
        
        apigui_end_window(ctx);
    }
}
