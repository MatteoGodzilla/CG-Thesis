#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include "universe.h"

void deserializeAll(std::istream& input, Universe* ref);
void serializeAll(std::ostream& output, Universe* ref);
