#include "string_processing.h"

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

std::vector<std::string> SplitIntoWords(const std::string& text)
{
    std::vector<std::string> words;
    std::string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

std::vector<int> ReadLineWithRatings()
{
    std::vector<int> ratings;
    std::string str;
    std::getline(std::cin, str);

    for (size_t i = 1; i < SplitIntoWords(str).size(); i++)
    {
        ratings.push_back(stoi(SplitIntoWords(str)[i]));
    }
    return ratings;
}