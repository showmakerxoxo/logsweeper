#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <logsweeper.h>

std::atomic_bool running{true};

void signalHandler(int in_signal) {
    if (in_signal == SIGINT) {
        std::cout << "\nReceived termination signal, stopping..." << std::endl;
        running.store(false);
    }
}

int main() {
    const std::string logDir = "./log/";

    std::signal(SIGINT, signalHandler);

    LogSweeper sweeper;
    sweeper.setLimitSize(5);
    sweeper.setWaitTime(5);
    sweeper.setLogPath(logDir);

    std::cout << "Starting LogSweeper on directory: " << logDir << std::endl;
    std::cout << "Press Ctrl+C to exit..." << std::endl;
    sweeper.start();

    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Stopping LogSweeper..." << std::endl;

    std::cout << "Exit program." << std::endl;
    return 0;
}