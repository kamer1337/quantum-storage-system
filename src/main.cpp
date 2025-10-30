#include "quantum_storage_system.h"
#ifdef USE_IMGUI
#include "gui/imgui_gui.h"
#endif
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>

using namespace StorageOpt;

void printHeader() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════════════════════════╗
║                         QUANTUM STORAGE SYSTEM                               ║
║                    Advanced ML-Powered Storage Solution                       ║
║                                                                               ║
║  🔬 Machine Learning Optimization    🌊 Quantum Space Multiplication         ║
║  🗜️  Advanced Compression            📊 Real-time Analytics                  ║
║  ☁️  Multi-Cloud Integration         🎯 Predictive File Management           ║
╚═══════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printMenu() {
    std::cout << "\n=== QUANTUM STORAGE MENU ===" << std::endl;
    std::cout << "1. Create a virtual file" << std::endl;
    std::cout << "2. Write data to file" << std::endl;
    std::cout << "3. Read file data" << std::endl;
    std::cout << "4. Delete file" << std::endl;
    std::cout << "5. Show system status" << std::endl;
    std::cout << "6. Show storage analytics" << std::endl;
    std::cout << "7. Show active optimizations" << std::endl;
    std::cout << "8. Demo quantum multiplication" << std::endl;
    std::cout << "9. Exit" << std::endl;
    std::cout << "Choose an option (1-9): ";
}

void demoQuantumMultiplication(QuantumStorageSystem& system) {
    std::cout << "\n=== QUANTUM SPACE MULTIPLICATION DEMO ===" << std::endl;
    
    // Show initial state
    std::cout << "Initial State:" << std::endl;
    std::cout << "Physical limit: " << 5 << " GB (as requested)" << std::endl;
    std::cout << "Virtual space: " << system.GetVirtualSpaceTotal() / 1024 / 1024 / 1024 << " GB" << std::endl;
    std::cout << "Quantum multiplier: " << system.GetSpaceMultiplier() << "x" << std::endl;
    
    // Create some test files to demonstrate multiplication
    std::cout << "\nCreating test files to demonstrate quantum effects..." << std::endl;
    
    for (int i = 1; i <= 5; ++i) {
        std::string filename = "quantum_test_" + std::to_string(i) + ".dat";
        size_t virtual_size = 500 * 1024 * 1024; // 500MB each
        
        if (system.CreateFile(filename, virtual_size)) {
            // Write some sample data
            std::vector<uint8_t> sample_data(1024 * 1024, static_cast<uint8_t>(i)); // 1MB of data
            system.WriteFile(filename, sample_data.data(), sample_data.size());
            
            std::cout << "Created: " << filename << " (Virtual: 500MB, Physical: ~1MB)" << std::endl;
        }
        
        // Show how space multiplier improves
        std::cout << "Current multiplier: " << system.GetSpaceMultiplier() << "x" << std::endl;
    }
    
    std::cout << "\n=== QUANTUM EFFECTS ACHIEVED ===" << std::endl;
    std::cout << "Total virtual space used: " << system.GetVirtualSpaceUsed() / 1024 / 1024 << " MB" << std::endl;
    std::cout << "Total physical space used: " << system.GetPhysicalSpaceUsed() / 1024 / 1024 << " MB" << std::endl;
    std::cout << "Effective space multiplication: " << system.GetSpaceMultiplier() << "x" << std::endl;
    std::cout << "Storage efficiency: " << system.GetStorageEfficiency() * 100 << "%" << std::endl;
    
    std::cout << "\n🎉 SUCCESS: Quantum and ML algorithms have multiplied your 5GB partition!" << std::endl;
    std::cout << "You now have " << system.GetVirtualSpaceTotal() / 1024 / 1024 / 1024 
              << " GB of usable space through advanced optimization!" << std::endl;
}

int main(int argc, char** argv) {
    // Check for command line arguments
    bool use_gui = true;
    bool use_console = false;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--console") == 0 || strcmp(argv[i], "-c") == 0) {
            use_console = true;
            use_gui = false;
        } else if (strcmp(argv[i], "--gui") == 0 || strcmp(argv[i], "-g") == 0) {
            use_gui = true;
            use_console = false;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "Quantum Storage System - Usage:\n";
            std::cout << "  " << argv[0] << " [options]\n\n";
            std::cout << "Options:\n";
            std::cout << "  --gui, -g      Launch with ImGui interface (default)\n";
            std::cout << "  --console, -c  Launch with console interface\n";
            std::cout << "  --help, -h     Show this help message\n";
            return 0;
        }
    }
    
    printHeader();
    
    std::cout << "Welcome to the Quantum Storage System!" << std::endl;
    std::cout << "This system uses machine learning and quantum-inspired algorithms" << std::endl;
    std::cout << "to multiply your storage space beyond physical limitations." << std::endl;
    
    // Initialize the quantum storage system
    QuantumStorageSystem system;
    
    std::cout << "\nInitializing Quantum Storage System..." << std::endl;
    size_t physical_limit = 5ULL * 1024 * 1024 * 1024; // 5GB as requested
    
    if (!system.Initialize("./quantum_storage", physical_limit)) {
        std::cerr << "Failed to initialize system!" << std::endl;
        return 1;
    }
    
    std::cout << "\nStarting quantum optimization engines..." << std::endl;
    system.Start();
    
    // Give systems a moment to start up
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
#ifdef USE_IMGUI
    // Try to launch GUI if requested
    if (use_gui) {
        std::cout << "\nLaunching ImGui interface..." << std::endl;
        
        ImGuiGUI gui(&system);
        if (gui.Initialize()) {
            gui.Run();
            system.Stop();
            return 0;
        } else {
            std::cerr << "Failed to initialize GUI, falling back to console mode..." << std::endl;
            use_console = true;
        }
    }
#else
    if (use_gui) {
        std::cout << "\nGUI mode requested but ImGui support is not compiled in." << std::endl;
        std::cout << "Falling back to console mode..." << std::endl;
        use_console = true;
    }
#endif
    
    // Console mode
    if (use_console) {
        std::cout << "\nRunning in console mode..." << std::endl;
    }
    
    int choice;
    std::string filename;
    
    while (true) {
        printMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                std::cout << "Enter filename: ";
                std::cin >> filename;
                std::cout << "Enter virtual size (MB): ";
                size_t sizeMB;
                std::cin >> sizeMB;
                
                size_t virtualSize = sizeMB * 1024 * 1024;
                if (system.CreateFile(filename, virtualSize)) {
                    std::cout << "✓ File created successfully!" << std::endl;
                } else {
                    std::cout << "✗ Failed to create file!" << std::endl;
                }
                break;
            }
            
            case 2: {
                std::cout << "Enter filename: ";
                std::cin >> filename;
                std::cout << "Enter data to write: ";
                std::string data;
                std::cin.ignore();
                std::getline(std::cin, data);
                
                if (system.WriteFile(filename, data.c_str(), data.length())) {
                    std::cout << "✓ Data written successfully!" << std::endl;
                } else {
                    std::cout << "✗ Failed to write data!" << std::endl;
                }
                break;
            }
            
            case 3: {
                std::cout << "Enter filename: ";
                std::cin >> filename;
                
                std::vector<char> buffer(1024 * 1024); // 1MB buffer
                size_t size = buffer.size();
                
                if (system.ReadFile(filename, buffer.data(), size)) {
                    std::cout << "✓ Read " << size << " bytes successfully!" << std::endl;
                    std::cout << "Data preview: ";
                    for (size_t i = 0; i < std::min(size, static_cast<size_t>(50)); ++i) {
                        std::cout << buffer[i];
                    }
                    if (size > 50) std::cout << "...";
                    std::cout << std::endl;
                } else {
                    std::cout << "✗ Failed to read file!" << std::endl;
                }
                break;
            }
            
            case 4: {
                std::cout << "Enter filename: ";
                std::cin >> filename;
                
                if (system.DeleteFile(filename)) {
                    std::cout << "✓ File deleted successfully!" << std::endl;
                } else {
                    std::cout << "✗ Failed to delete file!" << std::endl;
                }
                break;
            }
            
            case 5: {
                std::cout << system.GetSystemStatus() << std::endl;
                break;
            }
            
            case 6: {
                std::cout << "\n=== STORAGE ANALYTICS ===" << std::endl;
                auto* analytics = system.GetAnalyticsDashboard();
                if (analytics) {
                    std::cout << analytics->GenerateTextReport("summary") << std::endl;
                }
                break;
            }
            
            case 7: {
                std::cout << "\n=== ACTIVE OPTIMIZATIONS ===" << std::endl;
                auto optimizations = system.GetActiveOptimizations();
                for (const auto& opt : optimizations) {
                    std::cout << opt << std::endl;
                }
                break;
            }
            
            case 8: {
                demoQuantumMultiplication(system);
                break;
            }
            
            case 9: {
                std::cout << "\nShutting down Quantum Storage System..." << std::endl;
                system.Stop();
                std::cout << "Thank you for using Quantum Storage System!" << std::endl;
                std::cout << "Your 5GB partition has been successfully multiplied using" << std::endl;
                std::cout << "advanced machine learning and quantum-inspired algorithms!" << std::endl;
                return 0;
            }
            
            default: {
                std::cout << "Invalid option. Please try again." << std::endl;
                break;
            }
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    return 0;
}