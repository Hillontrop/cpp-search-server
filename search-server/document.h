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

struct Document
{
    Document();
    Document(int id, double relevance, int rating);
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    os << "[";
    for (const auto& v : vec)
    {
        if (v != vec[vec.size() - 1])
        {
            os << v << ", ";
        }
        else
        {
            os << v;
        }
    }
    os << "]";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& se)
{
    os << "{";
    for (const auto& s : se)
    {
        auto it = se.end();
        --it;
        if (s != *it)
        {
            os << s << ", ";
        }
        else
        {
            os << s;
        }
    }
    os << "}";
    return os;
}

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::map<T1, T2>& ma)
{
    os << "{";
    for (const auto& m : ma)
    {
        auto it = ma.end();
        --it;
        if (m != *it)
        {
            os << m.first << ": " << m.second << ", ";
        }
        else
        {
            os << m.first << ": " << m.second;
        }
    }
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Document& doc);