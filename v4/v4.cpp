#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <atomic>
#include <cmath>

std::string capture_current_time() {
  auto moment = std::chrono::system_clock::now();
  auto time_t_val = std::chrono::system_clock::to_time_t(moment);
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(moment.time_since_epoch()) % 1000;
    
  std::stringstream stream;
  
  struct tm time_data;
  localtime_r(&time_t_val, &time_data);
  
  stream << std::put_time(&time_data, "%Y-%m-%d %H:%M:%S");
  stream << "." << std::setfill('0') << std::setw(3) << millis.count();
  
  return stream.str();
}

void test_divisibility_range(int target, int range_start, int range_limit, std::atomic<bool>& still_prime) {
  if (range_start % 6 != 5 && range_start % 6 != 1) {
    range_start = ((range_start / 6) * 6) + 5;
  }
  
  for (int divisor = range_start; divisor <= range_limit && still_prime; divisor += 6) {
    if (target % divisor == 0 || target % (divisor + 2) == 0) {
      still_prime = false;
      break;
    }
  }
}

bool check_primality(int candidate, int worker_count) {
  if (candidate <= 1) return false;
  if (candidate <= 3) return true;
  if (candidate % 2 == 0 || candidate % 3 == 0) return false;
    
  std::atomic<bool> still_prime(true);
  std::vector<std::thread> worker_pool;

  int sqrt_limit = static_cast<int>(std::sqrt(candidate));
  int total_checks = (sqrt_limit - 5) / 6 + 1;
  
  if (total_checks <= 0) return true;
    
  int checks_per_worker = total_checks / worker_count;
  if (checks_per_worker < 1) checks_per_worker = 1;
    
  for (int idx = 0; idx < worker_count; idx++) {
    int partition_start = 5 + (idx * checks_per_worker * 6);
    int partition_end = (idx == worker_count - 1) ? sqrt_limit : 5 + ((idx + 1) * checks_per_worker * 6) - 6;
        
    if (partition_start > sqrt_limit) break;
        
    worker_pool.emplace_back(test_divisibility_range, candidate, partition_start, partition_end, std::ref(still_prime));
  }
    
  for (auto& worker : worker_pool) {
    worker.join();
  }
    
  return still_prime.load();
}

int main() {
  std::ifstream config_input("../config.txt");
  int thread_count = 0;
  int search_max = 0;
  std::string text_line;

  if (!config_input.is_open()) {
    std::cerr << "ERROR: Cannot open config.txt file" << std::endl;
    return 1;
  }

  while (std::getline(config_input, text_line)) {
    if (text_line.empty()) continue;
        
    size_t separator = text_line.find('=');
    if (separator != std::string::npos) {
      std::string param = text_line.substr(0, separator);
      std::string val = text_line.substr(separator + 1);
            
      if (param == "threads") {
        thread_count = std::stoi(val);
      } else if (param == "limit") {
        search_max = std::stoi(val);
      }
    }
  }
  config_input.close();
    
  std::cout << "\n========================================" << std::endl;
  std::cout << "PARALLEL DIVISIBILITY TESTING MODE" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Settings:" << std::endl;
  std::cout << "  * Thread pool size: " << thread_count << std::endl; 
  std::cout << "  * Testing range: 1-" << search_max << std::endl;
  std::cout << "Begin: " << capture_current_time() << std::endl;
  std::cout << "========================================\n" << std::endl;
    
  auto begin_clock = std::chrono::high_resolution_clock::now();
  std::vector<std::pair<int, std::string>> prime_records;
    
  for (int num = 1; num <= search_max; num++) {
    if (check_primality(num, thread_count)) {
      prime_records.push_back({num, capture_current_time()});
    }
  }
    
  auto end_clock = std::chrono::high_resolution_clock::now();
    
  std::cout << "Analysis complete. Displaying discovered primes:\n" << std::endl;

  for (const auto& [prime_val, time_stamp] : prime_records) {
    std::cout << "[" << time_stamp << "] Prime identified: " << prime_val << std::endl;
  }
    
  std::cout << "\n========================================" << std::endl;
  std::cout << "Finish: " << capture_current_time() << std::endl;
  std::cout << "========================================" << std::endl;
    
  return 0;
}