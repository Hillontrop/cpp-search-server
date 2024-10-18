// #include "test_example_functions.h"


// void AssertImpl(
//     bool value,
//     const std::string& exprStr,
//     const std::string& file,
//     const std::string& func,
//     const unsigned line,
//     const std::string& hint)
// {
//     if (!value) {
//         std::cout << file << "(" << line << "): " << func << ": ";
//         std::cout << "ASSERT(" << exprStr << ") failed.";
//         if (!hint.empty()) {
//             std::cout << " Hint: " << hint;
//         }
//         std::cout << std::endl;
//         abort();
//     }
// }

// void AddDocument(
//     SearchServer& searchServer,
//     const int documentId,
//     const std::string& document,
//     const DocumentStatus status,
//     const std::vector<int>& ratings)
// {
//     searchServer.AddDocument(documentId, document, status, ratings);
// }