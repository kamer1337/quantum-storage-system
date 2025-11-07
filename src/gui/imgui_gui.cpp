#include "imgui_gui.h"
#include "independent_gui.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace StorageOpt {

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

ImGuiGUI::ImGuiGUI(QuantumStorageSystem* system)
    : window_(nullptr)
    , system_(system)
    , gui_context_(std::make_unique<IndependentGUI::Context>())
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

ImGuiGUI::~ImGuiGUI() {
    Shutdown();
}

bool ImGuiGUI::Initialize() {
    return InitializeWindow();
}

bool ImGuiGUI::InitializeWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // GL 3.3 + GLSL 330
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window_ = glfwCreateWindow(1600, 900, "Quantum Storage System - Independent GUI Interface", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Initialize Independent GUI
    if (!gui_context_->Initialize(window_)) {
        std::cerr << "Failed to initialize Independent GUI" << std::endl;
        glfwDestroyWindow(window_);
        glfwTerminate();
        return false;
    }
    
    return true;
}

void ImGuiGUI::InitializeGUI() {
    // Already initialized in InitializeWindow
}

void ImGuiGUI::Shutdown() {
    if (window_) {
        gui_context_->Shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

bool ImGuiGUI::ShouldClose() {
    return window_ && glfwWindowShouldClose(window_);
}

void ImGuiGUI::RenderMainMenuBar() {
    if (gui_context_->BeginMainMenuBar()) {
        if (gui_context_->BeginMenu("File")) {
            if (gui_context_->MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(window_, true);
            }
            gui_context_->EndMenu();
        }
        
        if (gui_context_->BeginMenu("View")) {
            gui_context_->MenuItem("Status", nullptr, &show_status_window_);
            gui_context_->MenuItem("Analytics", nullptr, &show_analytics_window_);
            gui_context_->MenuItem("File Operations", nullptr, &show_file_ops_window_);
            gui_context_->MenuItem("Quantum Visualization", nullptr, &show_quantum_viz_window_);
            gui_context_->MenuItem("GUI Demo", nullptr, &show_demo_window_);
            gui_context_->EndMenu();
        }
        
        if (gui_context_->BeginMenu("Help")) {
            if (gui_context_->MenuItem("About")) {
                status_message_ = "Quantum Storage System v1.0.0 - Advanced ML-Powered Storage";
            }
            gui_context_->EndMenu();
        }
        
        gui_context_->EndMainMenuBar();
    }
}

void ImGuiGUI::RenderStatusWindow() {
    if (!show_status_window_) return;
    
    gui_context_->SetNextWindowSize(IndependentGUI::Vec2(500, 300), IndependentGUI::FirstUseEver);
    if (gui_context_->BeginWindow("System Status", &show_status_window_)) {
        gui_context_->TextColored(IndependentGUI::Color(0.0f, 1.0f, 0.0f, 1.0f), "QUANTUM STORAGE SYSTEM");
        gui_context_->Separator();
        
        // Space information
        gui_context_->Text("Storage Information:");
        
        size_t virtual_total = system_->GetVirtualSpaceTotal();
        size_t virtual_used = system_->GetVirtualSpaceUsed();
        size_t physical_used = system_->GetPhysicalSpaceUsed();
        double multiplier = system_->GetSpaceMultiplier();
        double efficiency = system_->GetStorageEfficiency();
        
        char text[256];
        snprintf(text, sizeof(text), "Virtual Total: %.2f GB", virtual_total / (1024.0 * 1024.0 * 1024.0));
        gui_context_->Text(text);
        snprintf(text, sizeof(text), "Virtual Used:  %.2f GB", virtual_used / (1024.0 * 1024.0 * 1024.0));
        gui_context_->Text(text);
        snprintf(text, sizeof(text), "Physical Used: %.2f MB", physical_used / (1024.0 * 1024.0));
        gui_context_->Text(text);
        
        gui_context_->Separator();
        snprintf(text, sizeof(text), "Quantum Multiplier: %.2fx", multiplier);
        gui_context_->TextColored(IndependentGUI::Color(1.0f, 1.0f, 0.0f, 1.0f), text);
        
        // Progress bar for virtual space usage
        float virtual_usage = virtual_total > 0 ? (float)virtual_used / virtual_total : 0.0f;
        snprintf(text, sizeof(text), "%d%%", (int)(virtual_usage * 100));
        gui_context_->ProgressBar(virtual_usage, IndependentGUI::Vec2(-1.0f, 0.0f), text);
        
        gui_context_->Separator();
        snprintf(text, sizeof(text), "Storage Efficiency: %.1f%%", efficiency * 100.0);
        gui_context_->Text(text);
        
        // Health status
        bool is_healthy = system_->IsHealthy();
        gui_context_->Text("System Health: ");
        gui_context_->SameLine();
        if (is_healthy) {
            gui_context_->TextColored(IndependentGUI::Color(0.0f, 1.0f, 0.0f, 1.0f), "HEALTHY");
        } else {
            gui_context_->TextColored(IndependentGUI::Color(1.0f, 0.0f, 0.0f, 1.0f), "DEGRADED");
        }
        
        // Active optimizations
        gui_context_->Separator();
        gui_context_->Text("Active Optimizations:");
        auto optimizations = system_->GetActiveOptimizations();
        for (size_t i = 0; i < std::min(optimizations.size(), size_t(5)); ++i) {
            gui_context_->BulletText(optimizations[i].c_str());
        }
        
        if (!status_message_.empty()) {
            gui_context_->Separator();
            gui_context_->TextWrapped(status_message_.c_str());
        }
    }
    gui_context_->EndWindow();
}

void ImGuiGUI::RenderAnalyticsWindow() {
    if (!show_analytics_window_) return;
    
    gui_context_->SetNextWindowSize(IndependentGUI::Vec2(600, 400), IndependentGUI::FirstUseEver);
    if (gui_context_->BeginWindow("Storage Analytics Dashboard", &show_analytics_window_)) {
        auto* analytics = system_->GetAnalyticsDashboard();
        if (analytics) {
            gui_context_->TextColored(IndependentGUI::Color(0.0f, 1.0f, 1.0f, 1.0f), "Real-Time Analytics");
            gui_context_->Separator();
            
            // Get text report
            std::string report = analytics->GenerateTextReport("summary");
            
            // Parse and display report sections
            gui_context_->BeginChild("AnalyticsContent", IndependentGUI::Vec2(0, -30), true);
            
            std::istringstream iss(report);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.find("===") != std::string::npos) {
                    gui_context_->TextColored(IndependentGUI::Color(1.0f, 1.0f, 0.0f, 1.0f), line.c_str());
                } else if (!line.empty()) {
                    gui_context_->Text(line.c_str());
                }
            }
            
            gui_context_->EndChild();
            
            if (gui_context_->Button("Refresh Analytics")) {
                status_message_ = "Analytics refreshed!";
            }
        } else {
            gui_context_->Text("Analytics dashboard not available");
        }
    }
    gui_context_->EndWindow();
}

void ImGuiGUI::RenderFileOpsWindow() {
    if (!show_file_ops_window_) return;
    
    gui_context_->SetNextWindowSize(IndependentGUI::Vec2(500, 400), IndependentGUI::FirstUseEver);
    if (gui_context_->BeginWindow("File Operations", &show_file_ops_window_)) {
        gui_context_->TextColored(IndependentGUI::Color(1.0f, 0.5f, 0.0f, 1.0f), "Virtual File Management");
        gui_context_->Separator();
        
        // Create file section
        if (gui_context_->CollapsingHeader("Create Virtual File", true)) {
            gui_context_->InputText("Filename", filename_buffer_, sizeof(filename_buffer_));
            gui_context_->SliderInt("Size (MB)", &file_size_mb_, 1, 1000);
            
            if (gui_context_->Button("Create File")) {
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
        if (gui_context_->CollapsingHeader("Write Data to File")) {
            gui_context_->InputText("Target File", filename_buffer_, sizeof(filename_buffer_));
            gui_context_->InputTextMultiline("Data", write_data_buffer_, sizeof(write_data_buffer_), 
                                     IndependentGUI::Vec2(-1.0f, 128.0f));
            
            if (gui_context_->Button("Write Data")) {
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
        if (gui_context_->CollapsingHeader("Read File Data")) {
            gui_context_->InputText("File to Read", filename_buffer_, sizeof(filename_buffer_));
            
            if (gui_context_->Button("Read File")) {
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
        if (gui_context_->CollapsingHeader("Delete File")) {
            gui_context_->InputText("File to Delete", filename_buffer_, sizeof(filename_buffer_));
            
            if (gui_context_->Button("Delete File")) {
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
    gui_context_->EndWindow();
}

void ImGuiGUI::RenderQuantumVisualization() {
    if (!show_quantum_viz_window_) return;
    
    gui_context_->SetNextWindowSize(IndependentGUI::Vec2(500, 300), IndependentGUI::FirstUseEver);
    if (gui_context_->BeginWindow("Quantum Multiplication Visualization", &show_quantum_viz_window_)) {
        gui_context_->TextColored(IndependentGUI::Color(0.5f, 0.5f, 1.0f, 1.0f), "Quantum Space Multiplication");
        gui_context_->Separator();
        
        double multiplier = system_->GetSpaceMultiplier();
        size_t virtual_total = system_->GetVirtualSpaceTotal();
        
        gui_context_->Text("Physical Storage Limit: 5 GB");
        
        char text[256];
        snprintf(text, sizeof(text), "Virtual Storage Available: %.2f GB", virtual_total / (1024.0 * 1024.0 * 1024.0));
        gui_context_->Text(text);
        
        gui_context_->Spacing();
        snprintf(text, sizeof(text), "Quantum Multiplier Effect: %.2fx", multiplier);
        gui_context_->TextColored(IndependentGUI::Color(1.0f, 0.0f, 1.0f, 1.0f), text);
        
        // Visual representation
        gui_context_->Spacing();
        gui_context_->Text("Physical Space:");
        gui_context_->ProgressBar(1.0f, IndependentGUI::Vec2(-1.0f, 0.0f), "5 GB");
        
        gui_context_->Text("Virtual Space (Quantum Multiplied):");
        float visual_mult = std::min(static_cast<float>(multiplier / 10.0), 1.0f); // Scale for visualization
        snprintf(text, sizeof(text), "%dx Multiplied", (int)multiplier);
        gui_context_->ProgressBar(visual_mult, IndependentGUI::Vec2(-1.0f, 0.0f), text);
        
        gui_context_->Spacing();
        gui_context_->Separator();
        gui_context_->Text("Quantum Features Active:");
        gui_context_->BulletText("Quantum Superposition Compression");
        gui_context_->BulletText("ML-Optimized Storage Allocation");
        gui_context_->BulletText("Entanglement-Based Deduplication");
        gui_context_->BulletText("Coherence-Maintained State Management");
        
        gui_context_->Spacing();
        if (gui_context_->Button("Run Quantum Demo")) {
            status_message_ = "Running quantum multiplication demo...";
            // In a real implementation, you'd call the demo function here
        }
    }
    gui_context_->EndWindow();
}

void ImGuiGUI::Run() {
    while (!ShouldClose()) {
        glfwPollEvents();
        
        // Start frame
        gui_context_->NewFrame();
        
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
            gui_context_->ShowDemoWindow(&show_demo_window_);
        }
        
        // Rendering
        gui_context_->Render();
        
        glfwSwapBuffers(window_);
    }
}

} // namespace StorageOpt
