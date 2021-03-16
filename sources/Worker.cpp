//
// Created by lizer on 14.03.2021.
//
//Copyright 2001 <elizavetamaikova>

#include <Worker.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

std::timed_mutex lock;


Worker::Worker(std::atomic_int& sig)
    : _work(true), _sig(sig)
{
  LoggingInit();
  _fileName = "";
  for (uint32_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    _threads.emplace_back(&Worker::HashFinder, this);
  _threads.emplace_back(&Worker::Stop, this);
  for (auto& worker : _threads)
    if (worker.joinable()) worker.join();
}

Worker::Worker(std::atomic_int& sig, uint count)
    : _work(true), _sig(sig)
{
  LoggingInit();
  _fileName = "";
  for (uint i = 0; i < count; ++i)
    _threads.emplace_back(&Worker::HashFinder, this);
  _threads.emplace_back(&Worker::Stop, this);
  for (auto& worker : _threads)
    if (worker.joinable()) worker.join();
}

Worker::Worker(std::atomic_int& sig, uint count, std::string file_name)
    : _fileName(file_name), _work(true), _sig(sig)
{
  LoggingInit();
  for (uint i = 0; i < count; ++i)
    _threads.emplace_back(&Worker::HashFinder, this);
  _threads.emplace_back(&Worker::Stop, this);
  for (auto& worker : _threads)
    if (worker.joinable()) worker.join();
}
void Worker::HashFinder()
{
  auto start = std::chrono::high_resolution_clock::now();
  auto finish = std::chrono::high_resolution_clock::now();
  while (_work)
  {
    std::string data = std::to_string(std::rand());
    std::string hash = picosha2::hash256_hex_string(data);

    if (Matching(hash)) {
      if (!_fileName.empty())
      {
        finish = std::chrono::high_resolution_clock::now();

        auto js = nlohmann::json::object();
        js["data"] = data;
        js["hash"] = hash;
        js["time"] = std::chrono::duration_cast<std::chrono::microseconds>
            (finish - start).count();
        start = std::chrono::high_resolution_clock::now();
        if (lock.try_lock_for(std::chrono::milliseconds(100))) {
          _result.push_back(js);
          lock.unlock();
        }
      }
      BOOST_LOG_TRIVIAL(info)<< " data: " << data
                             << " hash: " << hash << " id: "
                             <<  std::this_thread::get_id();
    } else {
      BOOST_LOG_TRIVIAL(trace) << " data: " << data << " hash: " << hash
                               << " id: " << std::this_thread::get_id();
    }
  }
}


void Worker::LoggingInit()
{
  logging::add_console_log(
      std::cout,
      keywords::format = "[%TimeStamp%] [%Severity%] %Message%",
      keywords::auto_flush = true,
      keywords::filter = logging::trivial::severity == logging::trivial::info);

  typedef sinks::synchronous_sink<sinks::text_file_backend> file_sink;
  boost::shared_ptr<file_sink> sink(new file_sink(
      keywords::file_name = "../logs/file_%5N.log",
      keywords::rotation_size = 5 * 1024 * 1024,
      keywords::auto_flush = true));

  sink->set_formatter(expr::stream
                          << "["
                          << expr::attr<boost::posix_time::ptime>("TimeStamp")
                          << "] [" << logging::trivial::severity << "] "
                          << expr::smessage);

  logging::core::get()->add_sink(sink);
  logging::add_common_attributes();
}


bool Worker::Matching(std::string hash)
{
  const std::string expected = "0000";
  return hash.substr(hash.size() - expected.size()) == expected;
}

void Worker::Stop()
{
  while (_work) {
    if (_sig == 2 || _sig == 9 || _sig == 15)
    {
      _work = false;
      if (!_fileName.empty() && !_result.empty())
      {
        std::ofstream file(_fileName + ".json");
        if (file.is_open()) {
          file << _result.dump(4);
          _result.clear();
          file.close();
        }
      }
    }
  }
}
