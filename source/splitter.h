// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef MIMEAPPS_SPLITTER_H
#define MIMEAPPS_SPLITTER_H

#include <algorithm>
#include <utility>
#include <iterator>

template<typename SourceIterator>
struct Splitter
{
    typedef typename std::iterator_traits<SourceIterator>::value_type SourceValueType;
    
    Splitter(SourceIterator begin, SourceIterator end, const SourceValueType& delim) 
    : _begin(begin), _end(end), _delim(delim) {}
    
    typedef std::pair<SourceIterator, SourceIterator> value_type;
    
    
    struct iterator : public std::iterator<std::forward_iterator_tag, value_type>
    {
        friend struct Splitter;
    private:
        iterator(SourceIterator begin, SourceIterator end, const Splitter* splitter)
        : _range(begin, end), _splitter(splitter) {
            _atEnd = begin == end && end == splitter->_end;
        }
    public:
        const value_type& operator*() const {
            return _range;
        }
        const value_type* operator->() const {
            return &_range;
        }
        iterator& operator++() {
            forward();
            return *this;
        }
        iterator operator++(int) {
            iterator toReturn = *this;
            forward();
            return toReturn;
        }
        
        bool operator==(const iterator& other) const {
            return equal(other);
        }
        bool operator!=(const iterator& other) const {
            return !equal(other);
        }
    private:
        void forward() {
            if (_atEnd) {
                return;
            }
            
            if (_range.second != _splitter->_end) {
                _range.first = ++_range.second;
                _range.second = std::find(_range.first, _splitter->_end, _splitter->_delim);
            } else {
                _range.first = _range.second;
                _atEnd = true;
            }
        }
        
        bool equal(const iterator& other) const {
            return this->_range == other._range && this->_atEnd == other._atEnd;
        }
        
        std::pair<SourceIterator, SourceIterator> _range;
        const Splitter* _splitter;
        bool _atEnd;
    };
    
    iterator begin() const {
        return iterator(_begin, std::find(_begin, _end, _delim), this);
    }
    
    iterator end() const {
        return iterator(_end, _end, this);
    }
    
private:
    SourceIterator _begin;
    SourceIterator _end;
    SourceValueType _delim;
};

#endif
