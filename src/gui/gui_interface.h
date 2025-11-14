#pragma once

#include "../quantum_storage_system.h"

struct GLFWwindow;
struct APIGUIContext;

namespace StorageOpt {

/**
 * GUIInterface - C++ wrapper for the pure C API GUI renderer
 * 
 * This class provides a high-level interface to the API GUI system
 * for the Quantum Storage System. It manages the GUI window, rendering,
 * and user interactions.
 */
class GUIInterface {
private:
    GLFWwindow* window_;
    QuantumStorageSystem* system_;
    APIGUIContext* gui_context_;
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
    
    // Initialize GLFW and API GUI
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
    GUIInterface(QuantumStorageSystem* system);
    ~GUIInterface();
    
    bool Initialize();
    void Run();
    bool ShouldClose();
};

} // namespace StorageOpt
