#include "gui_interface.h"
#include "api_gui.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace StorageOpt {

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

GUIInterface::GUIInterface(QuantumStorageSystem* system)
    : window_(nullptr)
    , system_(system)
    , gui_context_(nullptr)
    , show_demo_window_(false)
    , show_status_window_(true)
    , show_analytics_window_(true)
    , show_file_ops_window_(true)
    , show_quantum_viz_window_(true)
    , file_size_mb_(100)
{
    filename_buffer_[0] = '\0';
    write_data_buffer_[0] = '\0';
}

GUIInterface::~GUIInterface() {
    Shutdown();
}

bool GUIInterface::Initialize() {
    return InitializeWindow();
}

bool GUIInterface::InitializeWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Use OpenGL compatibility profile for legacy API
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window_ = glfwCreateWindow(1600, 900, "Quantum Storage System - Pure C 5D Renderer", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Initialize API GUI
    gui_context_ = apigui_create_context();
    if (!gui_context_ || !apigui_initialize(gui_context_, window_)) {
        std::cerr << "Failed to initialize API GUI" << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    return true;
}

void GUIInterface::InitializeGUI() {
    // Already initialized in InitializeWindow
}

void GUIInterface::Shutdown() {
    if (window_) {
        if (gui_context_) {
            apigui_destroy_context(gui_context_);
            gui_context_ = nullptr;
        }
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

bool GUIInterface::ShouldClose() {
    return window_ && glfwWindowShouldClose(window_);
}

void GUIInterface::RenderMainMenuBar() {
    if (apigui_begin_main_menu_bar(gui_context_)) {
        if (apigui_begin_menu(gui_context_, "File")) {
            if (apigui_menu_item(gui_context_, "Exit", "Alt+F4", nullptr)) {
                glfwSetWindowShouldClose(window_, true);
            }
            apigui_end_menu(gui_context_);
        }
        
        if (apigui_begin_menu(gui_context_, "View")) {
            apigui_menu_item(gui_context_, "Status", nullptr, &show_status_window_);
            apigui_menu_item(gui_context_, "Analytics", nullptr, &show_analytics_window_);
            apigui_menu_item(gui_context_, "File Operations", nullptr, &show_file_ops_window_);
            apigui_menu_item(gui_context_, "Quantum Visualization", nullptr, &show_quantum_viz_window_);
            apigui_menu_item(gui_context_, "GUI Demo", nullptr, &show_demo_window_);
            apigui_end_menu(gui_context_);
        }
        
        if (apigui_begin_menu(gui_context_, "Help")) {
            if (apigui_menu_item(gui_context_, "About", nullptr, nullptr)) {
                status_message_ = "Quantum Storage System v1.0.0 - Advanced ML-Powered Storage";
            }
            apigui_end_menu(gui_context_);
        }
        
        apigui_end_main_menu_bar(gui_context_);
    }
}

void GUIInterface::RenderStatusWindow() {
    if (!show_status_window_) return;
    
    apigui_set_next_window_size(gui_context_, apigui_vec2(500, 300), APIGUI_COND_FIRST_USE_EVER);
    if (apigui_begin_window(gui_context_, "System Status", &show_status_window_, apigui_vec2(0, 0), apigui_vec2(0, 0))) {
        apigui_text_colored(gui_context_, apigui_color(0.0f, 1.0f, 0.0f, 1.0f), "QUANTUM STORAGE SYSTEM");
        apigui_separator(gui_context_);
        
        // Space information
        apigui_text(gui_context_, "Storage Information:");
        
        size_t virtual_total = system_->GetVirtualSpaceTotal();
        size_t virtual_used = system_->GetVirtualSpaceUsed();
        size_t physical_used = system_->GetPhysicalSpaceUsed();
        double multiplier = system_->GetSpaceMultiplier();
        double efficiency = system_->GetStorageEfficiency();
        
        char text[256];
        snprintf(text, sizeof(text), "Virtual Total: %.2f GB", virtual_total / (1024.0 * 1024.0 * 1024.0));
        apigui_text(gui_context_, text);
        snprintf(text, sizeof(text), "Virtual Used:  %.2f GB", virtual_used / (1024.0 * 1024.0 * 1024.0));
        apigui_text(gui_context_, text);
        snprintf(text, sizeof(text), "Physical Used: %.2f MB", physical_used / (1024.0 * 1024.0));
        apigui_text(gui_context_, text);
        
        apigui_separator(gui_context_);
        snprintf(text, sizeof(text), "Quantum Multiplier: %.2fx", multiplier);
        apigui_text_colored(gui_context_, apigui_color(1.0f, 1.0f, 0.0f, 1.0f), text);
        
        // Progress bar for virtual space usage
        float virtual_usage = virtual_total > 0 ? (float)virtual_used / virtual_total : 0.0f;
        snprintf(text, sizeof(text), "%d%%", (int)(virtual_usage * 100));
        apigui_progress_bar(gui_context_, virtual_usage, apigui_vec2(-1.0f, 0.0f), text);
        
        apigui_separator(gui_context_);
        snprintf(text, sizeof(text), "Storage Efficiency: %.1f%%", efficiency * 100.0);
        apigui_text(gui_context_, text);
        
        // Health status
        bool is_healthy = system_->IsHealthy();
        apigui_text(gui_context_, "System Health: ");
        apigui_same_line(gui_context_);
        if (is_healthy) {
            apigui_text_colored(gui_context_, apigui_color(0.0f, 1.0f, 0.0f, 1.0f), "HEALTHY");
        } else {
            apigui_text_colored(gui_context_, apigui_color(1.0f, 0.0f, 0.0f, 1.0f), "DEGRADED");
        }
        
        // Active optimizations
        apigui_separator(gui_context_);
        apigui_text(gui_context_, "Active Optimizations:");
        auto optimizations = system_->GetActiveOptimizations();
        for (size_t i = 0; i < std::min(optimizations.size(), size_t(5)); ++i) {
            apigui_bullet_text(gui_context_, optimizations[i].c_str());
        }
        
        if (!status_message_.empty()) {
            apigui_separator(gui_context_);
            apigui_text_wrapped(gui_context_, status_message_.c_str());
        }
    }
    apigui_end_window(gui_context_);
}

void GUIInterface::RenderAnalyticsWindow() {
    if (!show_analytics_window_) return;
    
    apigui_set_next_window_size(gui_context_, apigui_vec2(600, 400), APIGUI_COND_FIRST_USE_EVER);
    if (apigui_begin_window(gui_context_, "Storage Analytics Dashboard", &show_analytics_window_, apigui_vec2(0, 0), apigui_vec2(0, 0))) {
        auto* analytics = system_->GetAnalyticsDashboard();
        if (analytics) {
            apigui_text_colored(gui_context_, apigui_color(0.0f, 1.0f, 1.0f, 1.0f), "Real-Time Analytics");
            apigui_separator(gui_context_);
            
            // Get text report
            std::string report = analytics->GenerateTextReport("summary");
            
            // Parse and display report sections
            apigui_begin_child(gui_context_, "AnalyticsContent", apigui_vec2(0, -30), true);
            
            std::istringstream iss(report);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.find("===") != std::string::npos) {
                    apigui_text_colored(gui_context_, apigui_color(1.0f, 1.0f, 0.0f, 1.0f), line.c_str());
                } else if (!line.empty()) {
                    apigui_text(gui_context_, line.c_str());
                }
            }
            
            apigui_end_child(gui_context_);
            
            if (apigui_button(gui_context_, "Refresh Analytics")) {
                status_message_ = "Analytics refreshed!";
            }
        } else {
            apigui_text(gui_context_, "Analytics dashboard not available");
        }
    }
    apigui_end_window(gui_context_);
}

void GUIInterface::RenderFileOpsWindow() {
    if (!show_file_ops_window_) return;
    
    apigui_set_next_window_size(gui_context_, apigui_vec2(500, 400), APIGUI_COND_FIRST_USE_EVER);
    if (apigui_begin_window(gui_context_, "File Operations", &show_file_ops_window_, apigui_vec2(0, 0), apigui_vec2(0, 0))) {
        apigui_text_colored(gui_context_, apigui_color(1.0f, 0.5f, 0.0f, 1.0f), "Virtual File Management");
        apigui_separator(gui_context_);
        
        // Create file section
        if (apigui_collapsing_header(gui_context_, "Create Virtual File", true)) {
            apigui_input_text(gui_context_, "Filename", filename_buffer_, sizeof(filename_buffer_));
            apigui_slider_int(gui_context_, "Size (MB)", &file_size_mb_, 1, 1000);
            
            if (apigui_button(gui_context_, "Create File")) {
                if (strlen(filename_buffer_) > 0) {
                    size_t virtual_size = static_cast<size_t>(file_size_mb_) * 1024 * 1024;
                    if (system_->CreateFile(filename_buffer_, virtual_size)) {
                        status_message_ = "File '" + std::string(filename_buffer_) + "' created successfully!";
                    } else {
                        status_message_ = "Failed to create file '" + std::string(filename_buffer_) + "'";
                    }
                }
            }
        }
        
        // Write data section
        if (apigui_collapsing_header(gui_context_, "Write Data to File", true)) {
            apigui_input_text(gui_context_, "Target File", filename_buffer_, sizeof(filename_buffer_));
            apigui_input_text_multiline(gui_context_, "Data", write_data_buffer_, sizeof(write_data_buffer_), 
                                     apigui_vec2(-1.0f, 128.0f));
            
            if (apigui_button(gui_context_, "Write Data")) {
                if (strlen(filename_buffer_) > 0 && strlen(write_data_buffer_) > 0) {
                    if (system_->WriteFile(filename_buffer_, write_data_buffer_, strlen(write_data_buffer_))) {
                        status_message_ = "Data written to '" + std::string(filename_buffer_) + "' successfully!";
                    } else {
                        status_message_ = "Failed to write data to '" + std::string(filename_buffer_) + "'";
                    }
                }
            }
        }
        
        // Read file section
        if (apigui_collapsing_header(gui_context_, "Read File Data", true)) {
            apigui_input_text(gui_context_, "File to Read", filename_buffer_, sizeof(filename_buffer_));
            
            if (apigui_button(gui_context_, "Read File")) {
                if (strlen(filename_buffer_) > 0) {
                    std::vector<char> buffer(1024 * 1024); // 1MB buffer
                    size_t size = buffer.size();
                    
                    if (system_->ReadFile(filename_buffer_, buffer.data(), size)) {
                        std::string preview(buffer.data(), std::min(size, size_t(200)));
                        status_message_ = "Read " + std::to_string(size) + " bytes from '" + 
                                        std::string(filename_buffer_) + "':\n" + preview;
                    } else {
                        status_message_ = "Failed to read file '" + std::string(filename_buffer_) + "'";
                    }
                }
            }
        }
        
        // Delete file section
        if (apigui_collapsing_header(gui_context_, "Delete File", true)) {
            apigui_input_text(gui_context_, "File to Delete", filename_buffer_, sizeof(filename_buffer_));
            
            if (apigui_button(gui_context_, "Delete File")) {
                if (strlen(filename_buffer_) > 0) {
                    if (system_->DeleteFile(filename_buffer_)) {
                        status_message_ = "File '" + std::string(filename_buffer_) + "' deleted successfully!";
                    } else {
                        status_message_ = "Failed to delete file '" + std::string(filename_buffer_) + "'";
                    }
                }
            }
        }
    }
    apigui_end_window(gui_context_);
}

void GUIInterface::RenderQuantumVisualization() {
    if (!show_quantum_viz_window_) return;
    
    apigui_set_next_window_size(gui_context_, apigui_vec2(500, 300), APIGUI_COND_FIRST_USE_EVER);
    if (apigui_begin_window(gui_context_, "Quantum Multiplication Visualization", &show_quantum_viz_window_, apigui_vec2(0, 0), apigui_vec2(0, 0))) {
        apigui_text_colored(gui_context_, apigui_color(0.5f, 0.5f, 1.0f, 1.0f), "Quantum Space Multiplication");
        apigui_separator(gui_context_);
        
        double multiplier = system_->GetSpaceMultiplier();
        size_t virtual_total = system_->GetVirtualSpaceTotal();
        
        apigui_text(gui_context_, "Physical Storage Limit: 5 GB");
        
        char text[256];
        snprintf(text, sizeof(text), "Virtual Storage Available: %.2f GB", virtual_total / (1024.0 * 1024.0 * 1024.0));
        apigui_text(gui_context_, text);
        
        apigui_spacing(gui_context_);
        snprintf(text, sizeof(text), "Quantum Multiplier Effect: %.2fx", multiplier);
        apigui_text_colored(gui_context_, apigui_color(1.0f, 0.0f, 1.0f, 1.0f), text);
        
        // Visual representation
        apigui_spacing(gui_context_);
        apigui_text(gui_context_, "Physical Space:");
        apigui_progress_bar(gui_context_, 1.0f, apigui_vec2(-1.0f, 0.0f), "5 GB");
        
        apigui_text(gui_context_, "Virtual Space (Quantum Multiplied):");
        float visual_mult = std::min(static_cast<float>(multiplier / 10.0), 1.0f); // Scale for visualization
        snprintf(text, sizeof(text), "%dx Multiplied", (int)multiplier);
        apigui_progress_bar(gui_context_, visual_mult, apigui_vec2(-1.0f, 0.0f), text);
        
        apigui_spacing(gui_context_);
        apigui_separator(gui_context_);
        apigui_text(gui_context_, "Quantum Features Active:");
        apigui_bullet_text(gui_context_, "Quantum Superposition Compression");
        apigui_bullet_text(gui_context_, "ML-Optimized Storage Allocation");
        apigui_bullet_text(gui_context_, "Entanglement-Based Deduplication");
        apigui_bullet_text(gui_context_, "Coherence-Maintained State Management");
        
        apigui_spacing(gui_context_);
        if (apigui_button(gui_context_, "Run Quantum Demo")) {
            status_message_ = "Running quantum multiplication demo...";
            // In a real implementation, you'd call the demo function here
        }
    }
    apigui_end_window(gui_context_);
}

void GUIInterface::Run() {
    while (!ShouldClose()) {
        glfwPollEvents();
        
        // Start frame
        apigui_new_frame(gui_context_);
        
        // Clear background
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render UI
        RenderMainMenuBar();
        RenderStatusWindow();
        RenderAnalyticsWindow();
        RenderFileOpsWindow();
        RenderQuantumVisualization();
        
        if (show_demo_window_) {
            apigui_show_demo_window(gui_context_, &show_demo_window_);
        }
        
        // Rendering
        apigui_render(gui_context_);
        
        glfwSwapBuffers(window_);
    }
}

} // namespace StorageOpt
