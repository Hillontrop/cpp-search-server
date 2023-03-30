#pragma once

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "search_server.h"

bool Duplicates(std::vector<std::string> first, std::vector<std::string> second);

void RemoveDuplicates(SearchServer& search_server);