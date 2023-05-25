#include "string_processing.h"


std::vector<std::string_view> SplitIntoWords(std::string_view str)
{
    std::vector<std::string_view> words_view;
    str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ")));
    const int64_t pos_end = str.npos;
    while (!str.empty())
    {
        int64_t space = str.find(' ');
        words_view.push_back(space == pos_end ? str.substr(0, pos_end) : str.substr(0, space));
        str.remove_prefix(space == pos_end ? str.size() : space + 1);
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ")));
    }
    return words_view;
}