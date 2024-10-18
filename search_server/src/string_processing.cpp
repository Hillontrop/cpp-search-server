#include "search_server/string_processing.h"


std::vector<std::string_view> SplitIntoWords(std::string_view str)
{
    std::vector<std::string_view> wordsView;
    str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ")));
    const int64_t posEnd = str.npos;
    
    while (!str.empty()) {
        int64_t space = str.find(' ');
        wordsView.push_back(space == posEnd ? str.substr(0, posEnd) : str.substr(0, space));
        str.remove_prefix(space == posEnd ? str.size() : space + 1);
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ")));
    }
    return wordsView;
}