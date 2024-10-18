#pragma once

#include "search_server.h"

#include <algorithm>
#include <execution>
#include <list>
#include <set>
#include <string>
#include <vector>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& searchServer, const std::vector<std::string>& queries);

std::list<Document> ProcessQueriesJoined(const SearchServer& searchServer, const std::vector<std::string>& queries);