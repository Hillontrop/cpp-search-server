#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;   // Глобальная константа для сравнение чисел с плавающей точкой 

// -------- Вспомогательные функции и перегрузки для модульных тестов поисковой системы ---------- //
template <typename T>
ostream& operator<<(ostream& os, const vector<T>& vec)
{
    os << "["s;
    for (const auto& v : vec)
    {
        if (v != vec[vec.size() - 1])
        {
            os << v << ", "s;
        }
        else
        {
            os << v;
        }
    }
    os << "]"s;
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const set<T>& se)
{
    os << "{"s;
    for (const auto& s : se)
    {
        auto it = se.end();
        --it;
        if (s != *it)
        {
            os << s << ", "s;
        }
        else
        {
            os << s;
        }
    }
    os << "}"s;
    return os;
}

template <typename T1, typename T2>
ostream& operator<<(ostream& os, const map<T1, T2>& ma)
{
    os << "{"s;
    for (const auto& m : ma)
    {
        auto it = ma.end();
        --it;
        if (m != *it)
        {
            os << m.first << ": "s << m.second << ", "s;
        }
        else
        {
            os << m.first << ": "s << m.second;
        }
    }
    os << "}"s;
    return os;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint)
{
    if (t != u)
    {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty())
        {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint)
{
    if (!value)
    {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty())
        {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(T& func, const string& func_name)
{
    func();
    cerr << func_name << " OK"s << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)
// -------- Вспомогательные функции и перегрузки для модульных тестов поисковой системы ---------- //


string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
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

vector<int> ReadLineWithRatings()
{
    vector<int> ratings;
    string str;
    getline(cin, str);

    for (size_t i = 1; i < SplitIntoWords(str).size(); i++)
    {
        ratings.push_back(stoi(SplitIntoWords(str)[i]));
    }

    return ratings;
}

struct Document
{
    Document() : id{ 0 }, relevance{ 0.0 }, rating{ 0 }    //+ Конструктор изменен на более лаконичный
    {
    }
    Document(int id_, double relevance_, int rating_)
    {
        id = id_;                   //+ Убран избыточный this->
        relevance = relevance_;     //+ Убран избыточный this->
        rating = rating_;           //+ Убран избыточный this->
    }

    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer
{
public:
    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words)
    {
        for (const auto& word : stop_words)
        {
            if (!IsValidWord(word))
            {
                throw invalid_argument("invalid symbols in stop-word -> "s + word);
            }
            if (stop_words_.count(word) == 0)
            {
                stop_words_.insert(word);
            }
        }
    }

    explicit SearchServer(const string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text))    //+ Делегированиев другой конструктор
    {
    }


    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
    {
        if (document_id < 0)
        {
            throw invalid_argument("Negative_id");
        }
        if (documents_.count(document_id) != 0)
        {
            throw invalid_argument("id already exists");
        }
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double t_f = 1.0 / static_cast<double>(words.size());  //+ Выполнена замена TF на t_f

        for (const string& word : words)
        {
            word_to_document_freqs_[word][document_id] += t_f;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        order_of_adding_documents.push_back(document_id);       //+ Добавление в vector<int> для сохранения порядка добавляемых документов
    }

    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const
    {
        const auto query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                if (abs(lhs.relevance - rhs.relevance) < EPSILON)   // Используется глобальная переменная для сравнение чисел с плавающей точкой
                {
                    return lhs.rating > rhs.rating;
                }
                else
                {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status_filter) const
    {
        return FindTopDocuments(raw_query, [&status_filter](int document_id, DocumentStatus status, int rating) { return status == status_filter; });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int GetDocumentCount() const
    {
        return static_cast <int> (documents_.size());
    }

    int GetDocumentId(int index) const   //+ Метод исправлен. Теперь возвращать id документа добавленного на сервер под номером "index"
    {
        return order_of_adding_documents.at(--index);
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
    {
        vector<string> words;

        auto query_words = ParseQuery(raw_query);

        for (const string& word : query_words.words_plus)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                if (word_to_document_freqs_.at(word).count(document_id))
                {
                    words.push_back(word);
                }
            }
        }
        for (const string& word : query_words.words_minus)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                if (word_to_document_freqs_.at(word).count(document_id))
                {
                    words.clear();
                    break;
                }
            }
        }
        sort(begin(words), end(words), [](const string& str_1, const string& str_2) {return str_1 < str_2;});
        return tuple(words, documents_.at(document_id).status);
    }

private:

    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };

    map<string, map<int, double>> word_to_document_freqs_;

    set<string> stop_words_;

    map<int, DocumentData> documents_;

    vector<int> order_of_adding_documents;      //+ Порядок добавления документов

    bool IsStopWord(const string& word) const
    {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text))
        {
            if (!IsValidWord(word))     //+ Вынесено из AddDocument
            {
                throw invalid_argument("invalid symbols in word -> "s + word);
            }
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }

    struct QueryWord
    {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string word) const
    {
        if (!IsValidWord(word))     //+ Убрано вложеное ветвление
        {
            throw invalid_argument("invalid characters in query word -> " + word);
        }
        bool is_minus = false;

        if (word.at(0) == '-')
        {
            if (word.size() == 1)
            {
                throw invalid_argument("no text after - in the query");
            }
            if (word.at(1) == '-')
            {
                throw invalid_argument("more than one minus before minus-word in the query");
            }

            is_minus = true;
            word = word.substr(1);
        }
        const QueryWord query_word = { word, is_minus, IsStopWord(word) };
        return query_word;
    }

    struct Query            //+ WordsPlusMinus заменен на Query
    {                       //+ Убрано public:
        set<string> words_plus;
        set<string> words_minus;
    };

    Query ParseQuery(const string& text) const
    {
        Query word_plus_minus;
        for (const string& word : SplitIntoWords(text))
        {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop)
            {
                if (query_word.is_minus)
                {
                    word_plus_minus.words_minus.insert(query_word.data);
                }
                else
                {
                    word_plus_minus.words_plus.insert(query_word.data);
                }
            }
        }
        return word_plus_minus;
    }

    double InverseDocumentFrequency(const string& word) const
    {
        return log(static_cast<double>(documents_.size()) / static_cast <double> (word_to_document_freqs_.at(word).size()));
    }

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const
    {
        map<int, double> document_to_relevance;
        for (const string& word : query.words_plus)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            const double i_d_f = InverseDocumentFrequency(word);
            for (const auto [id, t_f] : word_to_document_freqs_.at(word))
            {
                const auto& document_data = documents_.at(id);
                if (document_predicate(id, document_data.status, document_data.rating))
                {
                    document_to_relevance[id] += t_f * i_d_f;
                }
            }
        }
        for (const string& word : query.words_minus)
        {
            if (word_to_document_freqs_.count(word) == 0)
            {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word))
            {
                document_to_relevance.erase(document_id);
            }
        }
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance)
        {
            matched_documents.push_back(
                {
                    document_id,
                    relevance,
                    documents_.at(document_id).rating
                });
        }
        return matched_documents;
    }

    static int ComputeAverageRating(const vector<int>& ratings)
    {
        if (ratings.empty())
        {
            return 0;
        }
        int number_of_ratings = static_cast<int> (ratings.size());
        return (accumulate(begin(ratings), end(ratings), 0) / number_of_ratings);
    }

    static bool IsValidWord(const string& word)
    {
        return none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; }); // true при отсутствии спец-символов
    }
};