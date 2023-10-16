#include "glm/glm.hpp"

#include <iostream>

int main() {
   std::cout << glm::mat4(1.0f)[0][0];
   return 0;
}