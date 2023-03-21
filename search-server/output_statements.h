#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "document.h"
#include "iterator_range.h"
#include "paginator.h"

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

template<typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range)
{
    for (auto it = range.begin(); it < range.end(); ++it)
    {
        os << *it;
    }
    return os;
}

template<typename Iterator>
std::ostream& operator<<(std::ostream& os, const Paginator<Iterator>& page)
{
    for (auto it = page.begin(); distance(it, page.end()) > 0; advance(page))
    {
        os << *it;
    }
    return os;
}