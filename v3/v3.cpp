#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cmath>

std::mutex display_mutex;

bool verify_prime(int value) {
  if (value <= 1) return false;
  if (value <= 3) return true;
  if (value % 2 == 0 || value % 3 == 0) return false;
    
  for (int i = 5; i * i <= value; i += 6) {
    if (value % i == 0 || value % (i + 2) == 0) return false;
  }
  
  return true;
}

std::string fetch_timestamp() {
  auto time_now = std::chrono::system_clock::now();
  auto epoch_time = std::chrono::system_clock::to_time_t(time_now);
  auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch()) % 1000;
    
  std::stringstream buffer;
  
  struct tm local_time;
  localtime_r(&epoch_time, &local_time);

  buffer << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
  buffer << "." << std::setfill('0') << std::setw(3) << millisec.count();
  
  return buffer.str();
}

void process_range_for_primes(int worker_num, int lower_bound, int upper_bound) {
  std::stringstream buffered_output;
  
  for (int candidate = lower_bound; candidate <= upper_bound; candidate++) {
    if (verify_prime(candidate)) {
      buffered_output << "[Worker " << worker_num << "] " 
                      << "[" << fetch_timestamp() << "] "
                      << "Located prime: " << candidate << std::endl;
    }
  }
  
  if (buffered_output.str().length() > 0) {
    std::lock_guard<std::mutex> lock(display_mutex);
    std::cout << buffered_output.str();
  }
}

int main() {
  std::ifstream settings_file("../config.txt");
  int thread_count = 0;
  int upper_limit = 0;
  std::string input_line;

  if (!settings_file.is_open()) {
    std::cerr << "ERROR: Could not access config.txt" << std::endl;
    return 1;
  }

  while (std::getline(settings_file, input_line)) {
    if (input_line.empty()) continue;
        
    size_t delimiter = input_line.find('=');
    
    if (delimiter != std::string::npos) {
      std::string property = input_line.substr(0, delimiter);
      std::string setting = input_line.substr(delimiter + 1);
            
      if (property == "threads") {
        thread_count = std::stoi(setting);
      } else if (property == "limit") {
        upper_limit = std::stoi(setting);
      }
    }
  }
    
  settings_file.close();
    
  std::cout << "\n=======================================" << std::endl;
  std::cout << "DIRECT RANGE PARTITIONING METHOD" << std::endl;
  std::cout << "=======================================" << std::endl;
  std::cout << "Parameters:" << std::endl;
  std::cout << "  * Worker threads: " << thread_count << std::endl; 
  std::cout << "  * Maximum value: " << upper_limit << std::endl;
  std::cout << "Launch time: " << fetch_timestamp() << std::endl;
  std::cout << "=======================================\n" << std::endl;
    
  auto execution_begin = std::chrono::high_resolution_clock::now();
    
  std::vector<std::thread> thread_pool;
  int segment_length = upper_limit / thread_count;
  
  for (int idx = 0; idx < thread_count; idx++) {
    int segment_start = idx * segment_length + 1;
    int segment_end = (idx == thread_count - 1) ? upper_limit : (idx + 1) * segment_length;
    
    std::cout << "[Main] Spawning Worker " << idx 
              << " --> Range: " << segment_start << " to " << segment_end << std::endl;
    
    thread_pool.emplace_back(process_range_for_primes, idx, segment_start, segment_end);
  }
    
  for (auto& worker_thread : thread_pool) {
    worker_thread.join();
  }
    
  auto execution_finish = std::chrono::high_resolution_clock::now();
    
  std::cout << "\n=======================================" << std::endl;
  std::cout << "Termination time: " << fetch_timestamp() << std::endl;
  std::cout << "=======================================" << std::endl;
    
  return 0;
}