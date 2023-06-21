Поисковый сервер (Search Server)

Использование
Пример использования Search Server:

#include "search_server.h"

int main() {
    // Создание экземпляра Search Server
    SearchServer server("stop_words.txt");

    // Добавление документов
    server.AddDocument(1, "This is the first document", DocumentStatus::ACTUAL, {5, 2, 3});
    server.AddDocument(2, "This document is about programming", DocumentStatus::ACTUAL, {4, 5});
    server.AddDocument(3, "Another document", DocumentStatus::BANNED, {1, 2, 3, 4});

    // Поиск документов по запросу
    std::vector<Document> results = server.FindTopDocuments("document");

    // Вывод результатов
    for (const Document& doc : results) {
        std::cout << "Document ID: " << doc.id << ", Rating: " << doc.rating << std::endl;
    }

    return 0;
}


