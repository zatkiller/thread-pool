#include "threadpool.h"

#include <iostream>
#include <future>
#include <fstream>
#include <chrono>
#include <ctime>
#include <time.h>
#include <string.h>

long long fibonacci(int n) {
    if (n <= 1)
        return 1;
    return fibonacci(n-1) + fibonacci(n-2);
}

std::string getTime() {
    time_t rawtime;
    time (&rawtime);
    std::string ctime_no_newline = strtok(ctime(&rawtime), "\n");
    return "[" + ctime_no_newline + "]";
}

void run(std::ofstream &outFile, int &testNum) {
    std::vector<std::future<long long>> futures;
    futures.reserve(testNum);
    
    clock_t start = clock();
    for (int i = 0; i < testNum; i++) 
        futures.emplace_back(std::async(std::launch::async, fibonacci, i % 10));

    for (auto &fut: futures)
        fut.get();

    outFile << "Time taken: " << (double)(clock() - start) / CLOCKS_PER_SEC << std::endl;
}

void runWithThreadPool(std::ofstream &outFile, int &testNum) {
    threadpool::Threadpool pool;
    int numOfThreads = 4;
    pool.init(numOfThreads);
    
    std::cout << "Thread pool size: " << pool.size() << std::endl;
    
    std::vector<std::future<long long>> futures;
    futures.reserve(testNum);
    
    clock_t start = clock();
    for (int i = 0; i < testNum; i++) 
        futures.emplace_back(pool.async(fibonacci, i % 10));

    for (auto &fut: futures)
        fut.get();

    outFile << "Time taken for threadpool: " << (double)(clock() - start) / CLOCKS_PER_SEC << std::endl;
    pool.terminate();
}

int main() {
    std::ofstream outFile("output.txt");
    int testNum = 2048;

    runWithThreadPool(outFile, testNum);
    run(outFile, testNum);
    outFile.close();

    return 0;
}