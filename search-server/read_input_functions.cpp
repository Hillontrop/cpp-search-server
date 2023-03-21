#include "read_input_functions.h"

std::string ReadLine()
{
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    std::cin >> result;
    ReadLine();
    return result;
}

std::vector<int> ReadLineWithRatings()
{
    std::vector<int> ratings;
    std::string str;
    std::getline(std::cin, str);

    for (size_t i = 1; i < SplitIntoWords(str).size(); i++)
    {
        ratings.push_back(std::stoi(SplitIntoWords(str)[i]));
    }
    return ratings;
}