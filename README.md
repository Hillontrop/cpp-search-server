# Поисковый сервер (Search Server)

## 1. Использование

Пример использования Search Server:

    #include "search_server.h"
      
    int main()
    {
        // Создание экземпляра Search Server
        SearchServer server("stop_words.txt");
    
        // Добавление документов
        server.AddDocument(1, "This is the first document", DocumentStatus::ACTUAL, {5, 2, 3});
        server.AddDocument(2, "This document is about programming", DocumentStatus::ACTUAL, {4, 5});
        server.AddDocument(3, "Another document", DocumentStatus::BANNED, {1, 2, 3, 4});
    
        // Поиск документов по запросу
        std::vector<Document> results = server.FindTopDocuments("document");
    
        // Вывод результатов
        for (const Document& doc : results)
        {
            PrintDocument(doc);
        }
        return 0;
    }

## 2.Описание методов
### - SearchServer::SearchServer(const std::string& stop_words_text)
Конструктор класса. Принимает коллекцию стоп-слов и инициализирует объект SearchServer.

### - SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings)
Метод для добавления документа в поисковый индекс. Принимает идентификатор документа, текст документа, статус документа и рейтинги документа.

### - SearchServer::RemoveDocument(int document_id)
Метод для удаления документа из поискового сервера. Принимает идентификатор документа.

### - SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentStatus status_filter)
Выполняет поиск документов по заданному запросу и возвращает список наиболее подходящих документов.

### - SearchServer::GetDocumentCount()
Метод для получения количества документов в поисковом сервере.


