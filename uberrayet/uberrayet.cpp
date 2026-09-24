#include "code/engine/engine.hpp"

int main(int /*argc*/, char ** /*argv*/) {
  Engine engine(1280, 720, 60, 0.0f, true);
  engine.Run();
  return 0;
}