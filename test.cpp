#include <iostream>
#include <chrono>
#include <thread>

void performTask() {
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

int main() {
    std::cout << "Starting task..." << std::endl;

    performTask();

    std::cout << "Task completed!" << std::endl;

    return 0;
}

