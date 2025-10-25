#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>

std::mutex collection_mutex;

struct PrimeData {
  int worker_id;
  int prime_number;
  std::string time_found;
};

std::vector<PrimeData> collected_primes;

bool is_number_prime(int num) {
  if (num <= 1) return false;
  if (num <= 3) return true;
  if (num % 2 == 0 || num % 3 == 0) return false;
    
  for (int i = 5; i * i <= num; i += 6) {
    if (num % i == 0 || num % (i + 2) == 0) return false;
  }
  
  return true;
}

std::string generate_timestamp() {
  auto current_moment = std::chrono::system_clock::now();
  auto time_value = std::chrono::system_clock::to_time_t(current_moment);
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(current_moment.time_since_epoch()) % 1000;
    
  std::stringstream stream;
  
  struct tm time_struct;
  localtime_r(&time_value, &time_struct);

  stream << std::put_time(&time_struct, "%Y-%m-%d %H:%M:%S");
  stream << "." << std::setfill('0') << std::setw(3) << millis.count();
  
  return stream.str();
}

void search_primes_in_range(int worker_id, int range_begin, int range_finish) {
  std::vector<PrimeData> worker_primes;
    
  for (int current = range_begin; current <= range_finish; current++) {
    if (is_number_prime(current)) {
      PrimeData data;
      data.worker_id = worker_id;
      data.prime_number = current;
      data.time_found = generate_timestamp();
      worker_primes.push_back(data);
    }
  }
    
  std::lock_guard<std::mutex> lock(collection_mutex);
  collected_primes.insert(collected_primes.end(), worker_primes.begin(), worker_primes.end());
}

int main() {
  std::ifstream config_stream("../config.txt");
  int worker_count = 0;
  int max_number = 0;
  std::string text_line;

  if (!config_stream.is_open()) {
    std::cerr << "ERROR: Cannot open config.txt file" << std::endl;
    return 1;
  }

  while (std::getline(config_stream, text_line)) {
    if (text_line.empty()) continue;
        
    size_t separator_position = text_line.find('=');
    
    if (separator_position != std::string::npos) {
      std::string config_key = text_line.substr(0, separator_position);
      std::string config_val = text_line.substr(separator_position + 1);
            
      if (config_key == "threads") {
        worker_count = std::stoi(config_val);
      } else if (config_key == "limit") {
        max_number = std::stoi(config_val);
      }
    }
  }
    
  config_stream.close();

  if (worker_count == 0 || max_number == 0) {
    std::cerr << "ERROR: Configuration missing 'threads' or 'limit' parameter" << std::endl;
    return 1;
  }
    
  std::cout << "\n========================================" << std::endl;
  std::cout << "PRIME FINDER - BATCH PROCESSING MODE" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Settings:" << std::endl;
  std::cout << "  * Workers: " << worker_count << std::endl; 
  std::cout << "  * Range: 1 to " << max_number << std::endl;
  std::cout << "Initiated: " << generate_timestamp() << std::endl;
  std::cout << "========================================\n" << std::endl;
    
  auto begin_execution = std::chrono::high_resolution_clock::now();
  
  int partition_size = max_number / worker_count;
  std::vector<std::thread> worker_pool;
    
  for (int idx = 0; idx < worker_count; idx++) {
    int partition_start = idx * partition_size + 1;
    int partition_end = (idx == worker_count - 1) ? max_number : (idx + 1) * partition_size;
        
    worker_pool.emplace_back(search_primes_in_range, idx, partition_start, partition_end);
  }
    
  for (auto& worker : worker_pool) {
    worker.join();
  }
    
  auto finish_execution = std::chrono::high_resolution_clock::now();
    
  std::cout << "Processing complete. Displaying results:\n" << std::endl;
  
  for (const auto& prime_entry : collected_primes) {
    std::cout << "[Worker " << prime_entry.worker_id << "] " 
              << "[" << prime_entry.time_found << "] "
              << "Prime discovered: " << prime_entry.prime_number << std::endl;
  }
    
  std::cout << "\n========================================" << std::endl;
  std::cout << "Completed: " << generate_timestamp() << std::endl;
  std::cout << "========================================" << std::endl;
    
  return 0;
}