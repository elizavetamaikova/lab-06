//
// Created by lizer on 14.03.2021.
//

#ifndef THREADING_WORKER_HPP
#define THREADING_WORKER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include "picosha2.h"
#include <nlohmann/json.hpp>
#include <atomic>
#include <string>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions/keyword.hpp>

class Worker
{
 public:
  explicit Worker(std::atomic_int& sig);
  Worker(std::atomic_int& sig, uint count);
  Worker(std::atomic_int& sig, uint count, std::string file_name);
  void HashFinder();
  void LoggingInit();
  bool Matching(std::string hash);
  void Stop();

 private:
  std::vector<std::thread> _threads;
  nlohmann::json _result;
  std::string _fileName;
  volatile bool _work;
  std::atomic_int& _sig;
};


#endif  // THREADING_WORKER_HPP
