//#include <algorithm>
//#include <cmath>
//#include <iostream>
//#include <map>
//#include <numeric>
//#include <set>
//#include <string>
//#include <tuple>
//#include <utility>
//#include <vector>
//#include <cstdlib>
//#include <iomanip>
//#include <sstream>
//
//using namespace std;
//
//const int MAX_RESULT_DOCUMENT_COUNT = 5;
//
//template <typename T>
//ostream& operator<<(ostream& os, const vector<T>& vec)
//{
//    os << "["s;
//    for (const auto& v : vec)
//    {
//        if (v != vec[vec.size() - 1])
//        {
//            os << v << ", "s;
//        }
//        else
//        {
//            os << v;
//        }
//    }
//    os << "]"s;
//    return os;
//}
//
//template <typename T>
//ostream& operator<<(ostream& os, const set<T>& se)
//{
//    os << "{"s;
//    for (const auto& s : se)
//    {
//        auto it = se.end();
//        --it;
//        if (s != *it)
//        {
//            os << s << ", "s;
//        }
//        else
//        {
//            os << s;
//        }
//    }
//    os << "}"s;
//    return os;
//}
//
//template <typename T1, typename T2>
//ostream& operator<<(ostream& os, const map<T1, T2>& ma)
//{
//    os << "{"s;
//    for (const auto& m : ma)
//    {
//        auto it = ma.end();
//        --it;
//        if (m != *it)
//        {
//            os << m.first << ": "s << m.second << ", "s;
//        }
//        else
//        {
//            os << m.first << ": "s << m.second;
//        }
//    }
//    os << "}"s;
//    return os;
//}
//
//template <typename T, typename U>
//void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
//    const string& func, unsigned line, const string& hint)
//{
//    if (t != u)
//    {
//        cout << boolalpha;
//        cout << file << "("s << line << "): "s << func << ": "s;
//        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
//        cout << t << " != "s << u << "."s;
//        if (!hint.empty())
//        {
//            cout << " Hint: "s << hint;
//        }
//        cout << endl;
//        abort();
//    }
//}
//
//#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
//
//#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
//
//void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
//    const string& hint)
//{
//    if (!value)
//    {
//        cout << file << "("s << line << "): "s << func << ": "s;
//        cout << "ASSERT("s << expr_str << ") failed."s;
//        if (!hint.empty())
//        {
//            cout << " Hint: "s << hint;
//        }
//        cout << endl;
//        abort();
//    }
//}
//
//#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
//
//#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))
//
//template <typename T>
//void RunTestImpl(T& func, const string& func_name)
//{
//    func();
//    cerr << func_name << " OK"s << endl;
//}
//
//#define RUN_TEST(func) RunTestImpl((func), #func)
//
//string ReadLine()
//{
//    string s;
//    getline(cin, s);
//    return s;
//}
//
//int ReadLineWithNumber()
//{
//    int result = 0;
//    cin >> result;
//    ReadLine();
//    return result;
//}
//
//vector<string> SplitIntoWords(const string& text)
//{
//    vector<string> words;
//    string word;
//    for (const char c : text)
//    {
//        if (c == ' ')
//        {
//            if (!word.empty())
//            {
//                words.push_back(word);
//                word.clear();
//            }
//        }
//        else
//        {
//            word += c;
//        }
//    }
//    if (!word.empty())
//    {
//        words.push_back(word);
//    }
//
//    return words;
//}
//
//vector<int> ReadLineWithRatings()
//{
//    vector<int> ratings;
//    string str;
//    getline(cin, str);
//
//    for (size_t i = 1; i < SplitIntoWords(str).size(); i++)
//    {
//        ratings.push_back(stoi(SplitIntoWords(str)[i]));
//    }
//
//    return ratings;
//}
//
//struct Document
//{
//    int id;
//    double relevance;
//    int rating;
//};
//
//enum class DocumentStatus
//{
//    ACTUAL,
//    IRRELEVANT,
//    BANNED,
//    REMOVED,
//};
//
//class SearchServer
//{
//public:
//    void SetStopWords(const string& text)
//    {
//        for (const string& word : SplitIntoWords(text))
//        {
//            stop_words_.insert(word);
//        }
//    }
//
//    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
//    {
//        const vector<string> words = SplitIntoWordsNoStop(document);
//        const double TF = 1.0 / static_cast<double>(words.size());
//
//        for (const string& word : words)
//        {
//            word_to_document_freqs_[word][document_id] += TF;
//        }
//        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
//    }
//
//    template <typename DocumentPredicate>
//    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const
//    {
//        const WordsPlusMinus query = ParseQuery(raw_query);
//        auto matched_documents = FindAllDocuments(query, document_predicate);
//
//        sort(matched_documents.begin(), matched_documents.end(),
//            [](const Document& lhs, const Document& rhs)
//            {
//                if (abs(lhs.relevance - rhs.relevance) < 1e-6)
//                {
//                    return lhs.rating > rhs.rating;
//                }
//                else
//                {
//                    return lhs.relevance > rhs.relevance;
//                }
//            });
//        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
//        {
//            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
//        }
//        return matched_documents;
//    }
//
//    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status_filter) const
//    {
//        return FindTopDocuments(raw_query, [&status_filter](int document_id, DocumentStatus status, int rating) { return status == status_filter; });
//    }
//
//    vector<Document> FindTopDocuments(const string& raw_query) const
//    {
//        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
//    }
//
//    int GetDocumentCount() const
//    {
//        return static_cast <int> (documents_.size());
//    }
//
//    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
//    {
//        vector<string> words;
//
//        const WordsPlusMinus query_words = ParseQuery(raw_query);
//
//        for (const string& word : query_words.words_plus)
//        {
//            if (word_to_document_freqs_.count(word) != 0)
//            {
//                if (word_to_document_freqs_.at(word).count(document_id))
//                {
//                    words.push_back(word);
//                }
//            }
//        }
//        for (const string& word : query_words.words_minus)
//        {
//            if (word_to_document_freqs_.count(word) != 0)
//            {
//                if (word_to_document_freqs_.at(word).count(document_id))
//                {
//                    words.clear();
//                    break;
//                }
//            }
//        }
//
//        sort(begin(words), end(words), [](const string& str_1, const string& str_2) {return str_1 < str_2;});
//
//        return tuple(words, documents_.at(document_id).status);
//    }
//
//private:
//
//    struct DocumentData
//    {
//        int rating;
//        DocumentStatus status;
//    };
//
//    map<string, map<int, double>> word_to_document_freqs_;
//
//    set<string> stop_words_;
//
//    map<int, DocumentData> documents_;
//
//    bool IsStopWord(const string& word) const
//    {
//        return stop_words_.count(word) > 0;
//    }
//
//    vector<string> SplitIntoWordsNoStop(const string& text) const
//    {
//        vector<string> words;
//        for (const string& word : SplitIntoWords(text))
//        {
//            if (!IsStopWord(word))
//            {
//                words.push_back(word);
//            }
//        }
//        return words;
//    }
//
//    struct QueryWord
//    {
//        string data;
//        bool is_minus;
//        bool is_stop;
//    };
//
//    QueryWord ParseQueryWord(string text) const
//    {
//        bool is_minus = false;
//
//        if (text[0] == '-')
//        {
//            is_minus = true;
//            text = text.substr(1);
//        }
//        return
//        {
//            text,
//            is_minus,
//            IsStopWord(text)
//        };
//    }
//
//    struct WordsPlusMinus
//    {
//    public:
//        set<string> words_plus;
//        set<string> words_minus;
//    };
//
//    WordsPlusMinus ParseQuery(const string& text) const
//    {
//        WordsPlusMinus word_plus_minus;
//        for (const string& word : SplitIntoWords(text))
//        {
//            const QueryWord query_word = ParseQueryWord(word);
//            if (!query_word.is_stop)
//            {
//                if (query_word.is_minus)
//                {
//                    word_plus_minus.words_minus.insert(query_word.data);
//                }
//                else
//                {
//                    word_plus_minus.words_plus.insert(query_word.data);
//                }
//            }
//        }
//        return word_plus_minus;
//    }
//
//    double InverseDocumentFrequency(const string& word) const
//    {
//        return log(static_cast<double>(documents_.size()) / static_cast <double> (word_to_document_freqs_.at(word).size()));
//    }
//
//    template <typename DocumentPredicate>
//    vector<Document> FindAllDocuments(const WordsPlusMinus& query, DocumentPredicate document_predicate) const
//    {
//        map<int, double> document_to_relevance;
//        for (const string& word : query.words_plus)
//        {
//            if (word_to_document_freqs_.count(word) == 0)
//            {
//                continue;
//            }
//            const double i_d_f = InverseDocumentFrequency(word);
//            for (const auto [id, t_f] : word_to_document_freqs_.at(word))
//            {
//                const auto& document_data = documents_.at(id);
//                if (document_predicate(id, document_data.status, document_data.rating))
//                {
//                    document_to_relevance[id] += t_f * i_d_f;
//                }
//            }
//        }
//
//        for (const string& word : query.words_minus)
//        {
//            if (word_to_document_freqs_.count(word) == 0)
//            {
//                continue;
//            }
//            for (const auto [document_id, _] : word_to_document_freqs_.at(word))
//            {
//                document_to_relevance.erase(document_id);
//            }
//        }
//
//        vector<Document> matched_documents;
//        for (const auto [document_id, relevance] : document_to_relevance)
//        {
//            matched_documents.push_back(
//                {
//                    document_id,
//                    relevance,
//                    documents_.at(document_id).rating
//                });
//        }
//        return matched_documents;
//    }
//
//    static int ComputeAverageRating(const vector<int>& ratings)
//    {
//        if (ratings.empty())
//        {
//            return 0;
//        }
//        int number_of_ratings = ratings.size();
//        return (accumulate(begin(ratings), end(ratings), 0) / number_of_ratings);
//    }
//};
//
//SearchServer CreateSearchServer()
//{
//    SearchServer search_server;
//    search_server.SetStopWords(ReadLine());
//
//    const int document_count = ReadLineWithNumber();
//    for (int document_id = 0; document_id < document_count; ++document_id)
//    {
//        string text = ReadLine();
//        vector<int> rating = ReadLineWithRatings();
//        search_server.AddDocument(document_id, text, DocumentStatus::ACTUAL, rating);
//    }
//
//    return search_server;
//}
//
//void PrintDocument(const Document& document)
//{
//    cout << "{ "s << "document_id = "s << document.id << ", "s
//        << "relevance = "s << document.relevance << ", "s
//        << "rating = "s << document.rating << " }"s << endl;
//}
//
//void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status)
//{
//    cout << "{ "s
//        << "document_id = "s << document_id << ", "s
//        << "status = "s << static_cast<int>(status) << ", "s
//        << "words ="s;
//    for (const string& word : words)
//    {
//        cout << ' ' << word;
//    }
//    cout << "}"s << endl;
//}

// -------- Начало модульных тестов поисковой системы ---------- //

// 1. Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent()
{
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

// 2. Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestAddDocument()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };
    const int doc_id_2 = 20;
    const string content_2 = "cat and dog old town"s;
    const vector<int> ratings_2 = { 4, 5, 6 };
    const int doc_id_3 = 30;
    const string content_3 = "old white dog"s;
    const vector<int> ratings_3 = { 10, 20, 30 };

    SearchServer server;
    ASSERT_EQUAL(server.GetDocumentCount(), 0);    // Добавил проверку есть ли документы в сервере
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);   // Добавили документ
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);   // Добавили документ
    server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);   // Добавили документ
    ASSERT_EQUAL(server.GetDocumentCount(), 3);    // Добавил проверку сколько документов добавлено
    // Проверяем слово которое есть в документах
    {
        const auto found_docs = server.FindTopDocuments("cat"s); // Ищем слово
        ASSERT_EQUAL(found_docs.size(), 2);     // Проверили, что слово "cat" есть в 2 документах
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, doc_id_1);        // Проверили, что найден первый документ
        ASSERT_EQUAL(doc1.id, doc_id_2);        // Проверили, что найден второй документ
    }
    // Проверяем слово которого нет в документах
    {
        const auto found_docs = server.FindTopDocuments("bird"s); // Ищем слово
        ASSERT_EQUAL(found_docs.size(), 0); // Проверили, что слово "bird" нет в документах
    }
}

// 3. Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestFindTopDocumentsMinusWord()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };
    const int doc_id_2 = 20;
    const string content_2 = "cat and dog old town"s;
    const vector<int> ratings_2 = { 4, 5, 6 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

    {
        const auto found_docs = server.FindTopDocuments("cat"s);
        const auto found_docs_with_minus_word = server.FindTopDocuments("cat -dog"s);
        ASSERT_EQUAL(found_docs.size(), 2);                      // Проверили, что слово "cat" есть в 2 документах
        ASSERT_EQUAL(found_docs_with_minus_word.size(), 1);      // Проверили, что если есть минус-словом "-dog" то выдается всего один документ
        const Document& doc0 = found_docs_with_minus_word[0];
        ASSERT(doc0.id != doc_id_2);                             // Проверили, что документа с минус-словом исключен
    }
}

// 4. Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestFindTopDocumentsSortRelevance()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };
    const int doc_id_2 = 20;
    const string content_2 = "cat and dog old town"s;
    const vector<int> ratings_2 = { 4, 5, 6 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

    {
        const auto found_docs = server.FindTopDocuments("cat"s);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT(doc0.relevance >= doc1.relevance);         // Проверили, что релевантность первого документа больше релевантности второго
    }
}

// 5. Корректное вычисление релевантности найденных документов.
void TestFindTopDocumentsRelevance()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };
    const int doc_id_2 = 20;
    const string content_2 = "cat and dog old town"s;
    const vector<int> ratings_2 = { 4, 5, 6 };
    const int doc_id_3 = 30;
    const string content_3 = "old white dog"s;
    const vector<int> ratings_3 = { 1, 2, 3 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
    server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);

    {
        const auto found_docs = server.FindTopDocuments("cat"s);

        double IDF_cat = log(static_cast<double>(server.GetDocumentCount()) / 2.0);    // IDF слова "cat"
        double TF_dok0_cat = 1.0 / 4.0;                 // TF слова "cat" в первом документе
        double TF_dok1_cat = 1.0 / 5.0;                 // TF слова "cat" во втором документе
        double relevance_doc0 = IDF_cat * TF_dok0_cat;  // Релевантность первого документа
        double relevance_doc1 = IDF_cat * TF_dok1_cat;  // Релевантность второго документа
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.relevance, relevance_doc0);     // Проверили, что релевантность первого документа вычислена правильно
        ASSERT_EQUAL(doc1.relevance, relevance_doc1);     // Проверили, что релевантность второгодокумента вычислена правильно
    }
}

// 6. Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestComputeAverageRating()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { -20, -10, -30 };
    const int doc_id_2 = 20;
    const string content_2 = "cat blak dog"s;
    const vector<int> ratings_2 = { 20, 10, 30 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

    {
        const auto found_docs = server.FindTopDocuments("cat"s);
        int rating_doc_id_1 = accumulate(ratings_1.begin(), ratings_1.end(), 0) / static_cast<int>(ratings_1.size());   // Рассчитываем рейтинг для первого документа 
        int rating_doc_id_2 = accumulate(ratings_2.begin(), ratings_2.end(), 0) / static_cast<int>(ratings_2.size());   // Рассчитываем рейтинг для второго документа 
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.rating, rating_doc_id_2); // Проверяем рейтинг для получееного документа
        ASSERT_EQUAL(doc1.rating, rating_doc_id_1); // Проверяем рейтинг для получееного документа
    }
}

// 7. Поиск документов, имеющих заданный статус.
void TestFindTopDocumentsStatus()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };
    const int doc_id_2 = 20;
    const string content_2 = "cat and dog old town"s;
    const vector<int> ratings_2 = { 4, 5, 6 };
    const int doc_id_3 = 30;
    const string content_3 = "old white cat"s;
    const vector<int> ratings_3 = { 1, 2, 3 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::IRRELEVANT, ratings_2);
    server.AddDocument(doc_id_3, content_3, DocumentStatus::BANNED, ratings_3);
    // Проверка поиска документов по статусу
    {
        const auto found_docs = server.FindTopDocuments("cat"s, DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id_2);
    }
    // Проверка поиска документов по статусу не соответствующему статусу документа
    {
        const auto found_docs = server.FindTopDocuments("cat"s, DocumentStatus::REMOVED);
        ASSERT_EQUAL(found_docs.size(), 0);
    }
}

// 8. Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestFindTopDocumentsPredicate()    // Правка внесена изменено на Predicate
{
    const int doc_id_1 = 10;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };
    const int doc_id_2 = 20;
    const string content_2 = "cat and dog old town"s;
    const vector<int> ratings_2 = { 40, 50, 60, 10 };
    const int doc_id_3 = 30;
    const string content_3 = "old white cat"s;
    const vector<int> ratings_3 = { 10, 20, 30 };
    const int doc_id_4 = 40;
    const string content_4 = "new white cat"s;
    const vector<int> ratings_4 = { 4, 5, 6 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);   // Добавили документ
    server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);   // Добавили документ
    server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);   // Добавили документ
    server.AddDocument(doc_id_4, content_4, DocumentStatus::REMOVED, ratings_4);   // Добавили документ

    {
        const auto found_docs = server.FindTopDocuments("cat"s,
            [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
        ASSERT_EQUAL(found_docs.size(), 3);
    }
    {
        const auto found_docs = server.FindTopDocuments("cat"s,
            [](int document_id, DocumentStatus status, int rating) { return rating % 5 == 0; });

        int rating_doc_id_2 = accumulate(ratings_2.begin(), ratings_2.end(), 0) / static_cast<int>(ratings_2.size());

        ASSERT_EQUAL(found_docs.size(), 3);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.rating, rating_doc_id_2);
    }
    {
        const auto found_docs = server.FindTopDocuments("cat"s,
            [](int document_id, DocumentStatus status, int rating) { return document_id % 20 == 0; });
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id_2);
    }
}

// 9. При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе.
//    Если есть соответствие хотя бы по одному минус - слову, должен возвращаться пустой список слов.
void TestMatchDocument()
{
    const int doc_id_1 = 10;
    const string content_1 = "cat and dog in the old town"s;
    const vector<int> ratings_1 = { 1, 2, 3 };

    SearchServer server;
    server.SetStopWords("in the"s);
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);

    // Проверка на стоп-слова
    {
        const auto& [words, status] = server.MatchDocument("cat in the old town"s, doc_id_1);
        const vector<string> expected_words = { "cat", "old", "town" };
        ASSERT_EQUAL(words, expected_words);        // Проверяем вектор слов
        ASSERT(status == DocumentStatus::ACTUAL);   // Поверяем статус
    }
    // Проверка на минус-слова
    {
        const auto& [words, status] = server.MatchDocument("cat in the old town -dog"s, doc_id_1);
        ASSERT(words.empty()); 
    }
}


void TestSearchServer()
{
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestFindTopDocumentsMinusWord);
    RUN_TEST(TestFindTopDocumentsSortRelevance);
    RUN_TEST(TestFindTopDocumentsRelevance);
    RUN_TEST(TestComputeAverageRating);
    RUN_TEST(TestFindTopDocumentsStatus);
    RUN_TEST(TestFindTopDocumentsPredicate);
    RUN_TEST(TestMatchDocument);
}

// --------- Окончание модульных тестов поисковой системы ----------- //

//int main()
//{
//    TestSearchServer(); // Тестирование
//    cout << "Search server testing finished"s << endl;    // Если вы видите эту строку, значит все тесты прошли успешно
//
//    SearchServer search_server;
//    search_server.SetStopWords("и в на"s);
//    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
//    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
//    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
//    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
//    cout << "ACTUAL by default:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s))
//    {
//        PrintDocument(document);
//    }
//    cout << "ACTUAL:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
//        [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; }))
//    {
//        PrintDocument(document);
//    }
//    cout << "BANNED:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED))
//    {
//        PrintDocument(document);
//    }
//    cout << "Even ids:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
//        [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; }))
//    {
//        PrintDocument(document);
//    }
//    return 0;
//}