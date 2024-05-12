#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <cstring>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// Function to get hardware information
std::string getHardwareInfo() {
    std::string hardwareInfo;

    // Get CPU model
    FILE* cpuInfoFile = popen("cat /proc/cpuinfo | grep 'model name' | uniq | cut -d ':' -f 2", "r");
    if (cpuInfoFile) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), cpuInfoFile) != nullptr) {
            hardwareInfo += "CPU Model: ";
            hardwareInfo += buffer;
        }
        pclose(cpuInfoFile);
    }

    // Get number of CPU cores
    FILE* cpuCoresFile = popen("cat /proc/cpuinfo | grep 'cpu cores' | uniq | cut -d ':' -f 2", "r");
    if (cpuCoresFile) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), cpuCoresFile) != nullptr) {
            hardwareInfo += "CPU Cores: ";
            hardwareInfo += buffer;
        }
        pclose(cpuCoresFile);
    }

    // Get total RAM
    FILE* ramInfoFile = popen("grep MemTotal /proc/meminfo", "r");
    if (ramInfoFile) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), ramInfoFile) != nullptr) {
            hardwareInfo += "Total RAM: ";
            hardwareInfo += buffer;
        }
        pclose(ramInfoFile);
    }

    // Get operating system version
    const char* osVersion = std::getenv("PRETTY_NAME");
    if (osVersion) {
        hardwareInfo += "OS Version: ";
        hardwareInfo += osVersion;
    }

    return hardwareInfo;
}

// Function to run the target program and collect performance data
void runTargetProgram(const char* programName, const char* outputFile) {
    // Record the start time
    auto startTime = std::chrono::steady_clock::now();

    // Fork a new process
    pid_t pid = fork();

    if (pid == -1) {
        std::cerr << "Failed to fork" << std::endl;
        return;
    }

    if (pid == 0) { // Child process
        // Execute the target program
        execl(programName, programName, NULL);
    } else { // Parent process
        // Wait for the child process to exit
        int status;
        waitpid(pid, &status, 0);

        // Record the end time
        auto endTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

        // Get hardware information
        std::string hardwareInfo = getHardwareInfo();

        // Output the target program name
        std::cout << "Target Program Name: " << programName << std::endl;

        // Output the current date
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "Current Date: " << std::ctime(&now);

        // Output hardware information
        std::cout << hardwareInfo << std::endl;

        // Output the elapsed time
        std::cout << "Elapsed Time (seconds): " << elapsedTime << std::endl;

        // Get RAM usage information
        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);
        std::cout << "RAM Usage (KB): " << usage.ru_maxrss << std::endl;

        // Output the CPU usage
        std::cout << "CPU Usage (seconds): " << usage.ru_utime.tv_sec + usage.ru_stime.tv_sec +
                                                  (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1e6 << std::endl;

        // Output the memory usage
        long memory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024; // in KB
        std::cout << "Memory Usage (KB): " << memory << std::endl;

        // Write the performance data to the output file
        std::ofstream out(outputFile);
        if (!out.is_open()) {
            std::cerr << "Failed to open output file" << std::endl;
            return;
        }
        out << "Target Program Name: " << programName << std::endl;
        out << "Current Date: " << std::ctime(&now);
        out << "Hardware Information "<<"\n" << hardwareInfo << std::endl;
        out << "Elapsed Time (seconds): " << elapsedTime << std::endl;
        out << "RAM Usage (KB): " << usage.ru_maxrss << std::endl;
        out << "CPU Usage (seconds): " << usage.ru_utime.tv_sec + usage.ru_stime.tv_sec +
                                                  (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1e6 << std::endl;
        out << "Memory Usage (KB): " << memory << std::endl;

        // Close the output file
        out.close();
    }
}

// Main function
int main(int argc, char* argv[]) {
    // Check if the correct number of command line arguments is provided
    if (argc != 3) {
        std::cerr << "Error: Incorrect number of arguments" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <program_name> <output_file>" << std::endl;
        return 1;
    }

    // Extract the program name and output file name from the command line arguments
    const char* programName = argv[1];
    const char* outputFile = argv[2];

    // Run the target program and collect performance data
    runTargetProgram(programName, outputFile);

    return 0;
}

