#include "test_example_functions.h"


void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint)
{
    if (!value)
    {
        std::cout << file << "(" << line << "): " << func << ": ";
        std::cout << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty())
        {
            std::cout << " Hint: " << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings)
{
    search_server.AddDocument(document_id, document, status, ratings);
}