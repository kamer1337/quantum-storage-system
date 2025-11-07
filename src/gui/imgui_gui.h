#pragma once

#include "../quantum_storage_system.h"
#include <memory>

struct GLFWwindow;

namespace IndependentGUI {
    class Context;
}

namespace StorageOpt {

class ImGuiGUI {
private:
    GLFWwindow* window_;
    QuantumStorageSystem* system_;
    std::unique_ptr<IndependentGUI::Context> gui_context_;
    bool show_demo_window_;
    bool show_status_window_;
    bool show_analytics_window_;
    bool show_file_ops_window_;
    bool show_quantum_viz_window_;
    
    // GUI state
    char filename_buffer_[256];
    int file_size_mb_;
    char write_data_buffer_[1024];
    std::string selected_file_;
    std::string status_message_;
    
    // Initialize GLFW and Independent GUI
    bool InitializeWindow();
    void InitializeGUI();
    void Shutdown();
    
    // Render functions
    void RenderMainMenuBar();
    void RenderStatusWindow();
    void RenderAnalyticsWindow();
    void RenderFileOpsWindow();
    void RenderQuantumVisualization();
    
public:
    ImGuiGUI(QuantumStorageSystem* system);
    ~ImGuiGUI();
    
    bool Initialize();
    void Run();
    bool ShouldClose();
};

} // namespace StorageOpt
