#pragma once

#include <iostream>
#include <string>
#include <vector>

std::string ReadLine();

int ReadLineWithNumber();

std::vector<std::string> SplitIntoWords(const std::string& text);

std::vector<int> ReadLineWithRatings();