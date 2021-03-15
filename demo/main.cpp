#include <csignal>
#include "Worker.hpp"



std::atomic_int sig = 0;
void Interupt(int i) {
  sig = i;
}

int main(int argc, char** argv) {
  signal(SIGTERM, Interupt);
  signal(SIGINT, Interupt);
  if (argc == 1)
    Worker work(sig);
  else if (argc == 2)
    Worker work(sig, boost::lexical_cast<unsigned>(argv[1]));
  else if (argc == 3)
    Worker work(sig, boost::lexical_cast<unsigned>(argv[1]), argv[2]);

}