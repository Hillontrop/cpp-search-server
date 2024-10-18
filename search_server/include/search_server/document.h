#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>


enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document();
    Document(const int id, const double relevance, const int rating);

    int Id = 0;
    double Relevance = 0.0;
    int Rating = 0;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "[";
    for (const auto& v : vec) {
        if (v != vec[vec.size() - 1]) {
            os << v << ", ";
            continue;
        }
        os << v;
    }
    os << "]";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& se) {
    os << "{";
    for (const auto& s : se) {
        auto it = se.end();
        --it;
        if (s != *it) {
            os << s << ", ";
            continue;
        }
        os << s;
    }
    os << "}";
    return os;
}

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::map<T1, T2>& ma)
{
    os << "{";
    for (const auto& m : ma) {
        auto it = ma.end();
        --it;
        if (m != *it) {
            os << m.first << ": " << m.second << ", ";
            continue;
        }
        os << m.first << ": " << m.second;
    }
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Document& doc);