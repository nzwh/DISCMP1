#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>

std::mutex output_mutex;

bool check_if_prime(int number) {
  if (number <= 1) return false;
  if (number <= 3) return true;
  if (number % 2 == 0 || number % 3 == 0) return false;
    
  for (int i = 5; i * i <= number; i += 6) {
    if (number % i == 0 || number % (i + 2) == 0) return false;
  }
  
  return true;
}

std::string get_current_timestamp() {
  auto current_time = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(current_time);
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()) % 1000;
    
  std::stringstream timestamp_stream;
  
  struct tm time_info;
  localtime_r(&time_t_now, &time_info);
  
  timestamp_stream << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S");
  timestamp_stream << "." << std::setfill('0') << std::setw(3) << milliseconds.count();
  
  return timestamp_stream.str();
}

void calculate_primes_in_range(int worker_id, int range_start, int range_end) {
  for (int num = range_start; num <= range_end; num++) {
    if (check_if_prime(num)) {
      std::lock_guard<std::mutex> lock(output_mutex);
      std::cout << "[Worker " << worker_id << "] " 
                << "[" << get_current_timestamp() << "] "
                << "Found prime: " << num << std::endl;
    }
  }
}

int main() {
  std::ifstream config_file("../config.txt");
  int thread_count = 0;
  int search_limit = 0;
  std::string config_line;

  if (!config_file.is_open()) {
    std::cerr << "ERROR: Failed to open config.txt file" << std::endl;
    return 1;
  }

  while (std::getline(config_file, config_line)) {
    if (config_line.empty()) continue;
        
    size_t delimiter_pos = config_line.find('=');
    
    if (delimiter_pos != std::string::npos) {
      std::string param_key = config_line.substr(0, delimiter_pos);
      std::string param_value = config_line.substr(delimiter_pos + 1);
            
      if (param_key == "threads") {
        thread_count = std::stoi(param_value);
      } else if (param_key == "limit") {
        search_limit = std::stoi(param_value);
      }
    }
  }
    
  config_file.close();
    
  std::cout << "\n========================================" << std::endl;
  std::cout << "PRIME NUMBER FINDER - VARIANT 1" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Configuration loaded:" << std::endl;
  std::cout << "  * Thread count: " << thread_count << std::endl; 
  std::cout << "  * Search limit: " << search_limit << std::endl;
  std::cout << "Started at: " << get_current_timestamp() << std::endl;
  std::cout << "========================================\n" << std::endl;
    
  auto execution_start = std::chrono::high_resolution_clock::now();
    
  int numbers_per_thread = search_limit / thread_count;
  std::vector<std::thread> worker_threads;
    
  for (int i = 0; i < thread_count; i++) {
    int start_num = i * numbers_per_thread + 1;
    int end_num = (i == thread_count - 1) ? search_limit : (i + 1) * numbers_per_thread;
        
    worker_threads.emplace_back(calculate_primes_in_range, i, start_num, end_num);
  }
    
  for (auto& worker : worker_threads) {
    worker.join();
  }
    
  auto execution_end = std::chrono::high_resolution_clock::now();
    
  std::cout << "\n========================================" << std::endl;
  std::cout << "Finished at: " << get_current_timestamp() << std::endl;
  std::cout << "========================================" << std::endl;
    
  return 0;
}