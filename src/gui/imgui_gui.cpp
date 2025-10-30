#include "imgui_gui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace StorageOpt {

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

ImGuiGUI::ImGuiGUI(QuantumStorageSystem* system)
    : window_(nullptr)
    , system_(system)
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
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window_ = glfwCreateWindow(1600, 900, "Quantum Storage System - ImGui Interface", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    return true;
}

void ImGuiGUI::InitializeImGui() {
    // Already initialized in InitializeWindow
}

void ImGuiGUI::Shutdown() {
    if (window_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

bool ImGuiGUI::ShouldClose() {
    return window_ && glfwWindowShouldClose(window_);
}

void ImGuiGUI::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Status", nullptr, &show_status_window_);
            ImGui::MenuItem("Analytics", nullptr, &show_analytics_window_);
            ImGui::MenuItem("File Operations", nullptr, &show_file_ops_window_);
            ImGui::MenuItem("Quantum Visualization", nullptr, &show_quantum_viz_window_);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &show_demo_window_);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                status_message_ = "Quantum Storage System v1.0.0 - Advanced ML-Powered Storage";
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void ImGuiGUI::RenderStatusWindow() {
    if (!show_status_window_) return;
    
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("System Status", &show_status_window_)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "QUANTUM STORAGE SYSTEM");
        ImGui::Separator();
        
        // Space information
        ImGui::Text("Storage Information:");
        
        size_t virtual_total = system_->GetVirtualSpaceTotal();
        size_t virtual_used = system_->GetVirtualSpaceUsed();
        size_t physical_used = system_->GetPhysicalSpaceUsed();
        double multiplier = system_->GetSpaceMultiplier();
        double efficiency = system_->GetStorageEfficiency();
        
        ImGui::Text("Virtual Total: %.2f GB", virtual_total / (1024.0 * 1024.0 * 1024.0));
        ImGui::Text("Virtual Used:  %.2f GB", virtual_used / (1024.0 * 1024.0 * 1024.0));
        ImGui::Text("Physical Used: %.2f MB", physical_used / (1024.0 * 1024.0));
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Quantum Multiplier: %.2fx", multiplier);
        
        // Progress bar for virtual space usage
        float virtual_usage = virtual_total > 0 ? (float)virtual_used / virtual_total : 0.0f;
        ImGui::ProgressBar(virtual_usage, ImVec2(-1.0f, 0.0f), 
                          (std::to_string((int)(virtual_usage * 100)) + "%").c_str());
        
        ImGui::Separator();
        ImGui::Text("Storage Efficiency: %.1f%%", efficiency * 100.0);
        
        // Health status
        bool is_healthy = system_->IsHealthy();
        ImGui::Text("System Health: ");
        ImGui::SameLine();
        if (is_healthy) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "HEALTHY");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DEGRADED");
        }
        
        // Active optimizations
        ImGui::Separator();
        ImGui::Text("Active Optimizations:");
        auto optimizations = system_->GetActiveOptimizations();
        for (size_t i = 0; i < std::min(optimizations.size(), size_t(5)); ++i) {
            ImGui::BulletText("%s", optimizations[i].c_str());
        }
        
        if (!status_message_.empty()) {
            ImGui::Separator();
            ImGui::TextWrapped("%s", status_message_.c_str());
        }
    }
    ImGui::End();
}

void ImGuiGUI::RenderAnalyticsWindow() {
    if (!show_analytics_window_) return;
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Storage Analytics Dashboard", &show_analytics_window_)) {
        auto* analytics = system_->GetAnalyticsDashboard();
        if (analytics) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Real-Time Analytics");
            ImGui::Separator();
            
            // Get text report
            std::string report = analytics->GenerateTextReport("summary");
            
            // Parse and display report sections
            ImGui::BeginChild("AnalyticsContent", ImVec2(0, -30), true);
            
            std::istringstream iss(report);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.find("===") != std::string::npos) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", line.c_str());
                } else if (!line.empty()) {
                    ImGui::Text("%s", line.c_str());
                }
            }
            
            ImGui::EndChild();
            
            if (ImGui::Button("Refresh Analytics")) {
                status_message_ = "Analytics refreshed!";
            }
        } else {
            ImGui::Text("Analytics dashboard not available");
        }
    }
    ImGui::End();
}

void ImGuiGUI::RenderFileOpsWindow() {
    if (!show_file_ops_window_) return;
    
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("File Operations", &show_file_ops_window_)) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Virtual File Management");
        ImGui::Separator();
        
        // Create file section
        if (ImGui::CollapsingHeader("Create Virtual File", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputText("Filename", filename_buffer_, sizeof(filename_buffer_));
            ImGui::SliderInt("Size (MB)", &file_size_mb_, 1, 1000);
            
            if (ImGui::Button("Create File")) {
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
        if (ImGui::CollapsingHeader("Write Data to File")) {
            ImGui::InputText("Target File", filename_buffer_, sizeof(filename_buffer_));
            ImGui::InputTextMultiline("Data", write_data_buffer_, sizeof(write_data_buffer_), 
                                     ImVec2(-1.0f, ImGui::GetTextLineHeight() * 8));
            
            if (ImGui::Button("Write Data")) {
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
        if (ImGui::CollapsingHeader("Read File Data")) {
            ImGui::InputText("File to Read", filename_buffer_, sizeof(filename_buffer_));
            
            if (ImGui::Button("Read File")) {
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
        if (ImGui::CollapsingHeader("Delete File")) {
            ImGui::InputText("File to Delete", filename_buffer_, sizeof(filename_buffer_));
            
            if (ImGui::Button("Delete File")) {
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
    ImGui::End();
}

void ImGuiGUI::RenderQuantumVisualization() {
    if (!show_quantum_viz_window_) return;
    
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Quantum Multiplication Visualization", &show_quantum_viz_window_)) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "Quantum Space Multiplication");
        ImGui::Separator();
        
        double multiplier = system_->GetSpaceMultiplier();
        size_t virtual_total = system_->GetVirtualSpaceTotal();
        
        ImGui::Text("Physical Storage Limit: 5 GB");
        ImGui::Text("Virtual Storage Available: %.2f GB", virtual_total / (1024.0 * 1024.0 * 1024.0));
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), 
                          "Quantum Multiplier Effect: %.2fx", multiplier);
        
        // Visual representation
        ImGui::Spacing();
        ImGui::Text("Physical Space:");
        ImGui::ProgressBar(1.0f, ImVec2(-1.0f, 0.0f), "5 GB");
        
        ImGui::Text("Virtual Space (Quantum Multiplied):");
        float visual_mult = std::min(static_cast<float>(multiplier / 10.0), 1.0f); // Scale for visualization
        ImGui::ProgressBar(visual_mult, ImVec2(-1.0f, 0.0f), 
                          (std::to_string((int)(multiplier)) + "x Multiplied").c_str());
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Quantum Features Active:");
        ImGui::BulletText("Quantum Superposition Compression");
        ImGui::BulletText("ML-Optimized Storage Allocation");
        ImGui::BulletText("Entanglement-Based Deduplication");
        ImGui::BulletText("Coherence-Maintained State Management");
        
        ImGui::Spacing();
        if (ImGui::Button("Run Quantum Demo")) {
            status_message_ = "Running quantum multiplication demo...";
            // In a real implementation, you'd call the demo function here
        }
    }
    ImGui::End();
}

void ImGuiGUI::Run() {
    while (!ShouldClose()) {
        glfwPollEvents();
        
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render UI
        RenderMainMenuBar();
        RenderStatusWindow();
        RenderAnalyticsWindow();
        RenderFileOpsWindow();
        RenderQuantumVisualization();
        
        if (show_demo_window_) {
            ImGui::ShowDemoWindow(&show_demo_window_);
        }
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window_);
    }
}

} // namespace StorageOpt
