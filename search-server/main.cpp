#include <iostream>
#include <string>

#include "document.h"
#include "iterator_range.h"
#include "paginator.h"
#include "request_queue.h"
#include "search_server.h"
#include "string_processing.h"
#include "const.h"
#include "read_input_functions.h"
#include "test_example_functions.h"



using namespace std;

template <typename Container>
auto Paginate(const Container& c, size_t page_size)
{
    return Paginator(begin(c), end(c), page_size);
}


int main()
{
    setlocale(LC_ALL, "ru");

    SearchServer search_server("and with"s);
    search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);

    for (auto page = pages.begin(); page != pages.end(); ++page)
    {
        std::cout << *page << std::endl;
        std::cout << "Page break"s << std::endl;
    }
    return 0;
}