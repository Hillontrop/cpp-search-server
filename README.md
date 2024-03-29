# Поисковый сервер (Search Server)

## 1. Описание
Проект SearchServer представляет собой поисковый движок, который обеспечивает индексацию и обработку документов для выполнения поисковых запросов. Он разработан для упрощения поиска информации в больших объемах текстовых данных.

## 2. Цель проекта
Цель проекта SearchServer заключается в предоставлении эффективного и удобного инструмента для поиска и обработки текстовых данных. Он был разработан с учетом следующих основных задач:

* **Быстрый поиск**: SearchServer оптимизирован для быстрого выполнения поисковых запросов даже в больших объемах данных.

* **Гибкость и простота использования**: Проект предоставляет простой и понятный интерфейс для добавления документов и выполнения поисковых запросов. Он позволяет использовать различные параметры поиска для точного определения результатов.

* **Надежность**: SearchServer разработан с учетом обработки ошибок и обеспечения стабильной работы в различных условиях.

* **Расширяемость**: Проект предоставляет возможность добавления новых функциональностей и настройки для соответствия уникальным потребностям пользователей.

С использованием SearchServer пользователи могут улучшить эффективность поиска информации, повысить качество и точность результатов и сократить время, затраченное на обработку текстовых данных.

## 3. Использование

Пример использования Search Server:
```cpp
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
```

## 4.Описание методов
* **SearchServer::SearchServer _(const std::string& stop_words_text)_** - Конструктор класса. Принимает коллекцию стоп-слов и инициализирует объект SearchServer.

* **SearchServer::AddDocument _(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings)_** Метод для добавления документа в поисковый индекс. Принимает идентификатор документа, текст документа, статус документа и рейтинги документа.

* **SearchServer::RemoveDocument _(int document_id)_** - Метод для удаления документа из поискового сервера. Принимает идентификатор документа.

* **SearchServer::FindTopDocuments _(const std::string_view& raw_query, DocumentStatus status_filter)_** - Выполняет поиск документов по заданному запросу и возвращает список наиболее подходящих документов.

* **SearchServer::GetDocumentCount _()_** - Метод для получения количества документов в поисковом сервере.

## 5. Планы по доработке проекта
1. **Документация и комментарии:**
Впервую очередь дополните код комментариями, чтобы обеспечить ясность и понятность кода для других разработчиков.

2. **Улучшение структуры данных:**
Замена простых массивов или списков на более эффективные структуры данных, такие как хэш-таблицы или деревья, для быстрого доступа к данным

3. **Добавление тестового покрытия:**
   * Создание модульных тестов для проверки отдельных компонентов системы, таких как алгоритмы поиска, индексации и ранжирования.
   * Использование инструментов автоматического тестирования, таких как фреймворки для юнит-тестирования или инструменты для тестирования производительности.



