#pragma once

template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end), size_(distance(begin_, end_))
    {
    }
    Iterator begin() const
    {
        return begin_;
    }
    Iterator end() const
    {
        return end_;
    }
    size_t size() const
    {
        return size_;
    }
private:
    Iterator begin_, end_;
    size_t size_;
};