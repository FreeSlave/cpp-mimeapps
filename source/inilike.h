// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef MIMEAPPS_INILIKE_H
#define MIMEAPPS_INILIKE_H

#include <string>
#include <map>
#include <istream>
#include <stdexcept>

namespace mimeapps
{
    namespace details {
        template<typename Iterator, typename PairIterator>
        std::string doUnescape(const Iterator& first, const Iterator& last, const PairIterator& firstPair, const PairIterator& lastPair)
        {
            //little optimization to avoid executing the algorithm.
            Iterator it = std::find(first, last, '\\');
            if (it == last) {
                return std::string(first, last);
            }
            
            std::string toReturn(first, it);
            
            for (; it != last; ++it) {
                if (*it == '\\' && (it+1) != last) {
                    const char c = *(it+1);
                    
                    bool shouldContinue;
                    for (PairIterator itPair = firstPair; itPair != lastPair; ++itPair) {
                        if (c == itPair->first) {
                            toReturn.push_back(itPair->second);
                            ++it;
                            shouldContinue = true;
                            break;
                        }
                    }
                    if (shouldContinue) {
                        continue;
                    }
                }
                toReturn.push_back(*it);
            }
            return toReturn;
        }
    }

    template<typename Iterator>
    std::string unescapeValue(const Iterator& first, const Iterator& last) {
        const std::pair<char, char> pairs[] = {
            std::make_pair('s', ' '),
            std::make_pair('n', '\n'),
            std::make_pair('r', '\r'),
            std::make_pair('t', '\t'),
            std::make_pair('\\', '\\')
        };
        return details::doUnescape(first, last, &pairs[0], pairs + 5);
    }
    std::string unescapeValue(const std::string& str);
    
    bool isTrue(const std::string& str);
    
    struct SearchRequest
    {
        struct Value
        {
            Value();
            bool found() const;
            std::string value() const;
            void setValue(const std::string& value);
        private:
            std::string _value;
            bool _found;
        };
        void addRequest(const std::string& group, const std::string& key);
        Value getValue(const std::string& group, const std::string& key);
        
        void searchKeyValues(std::istream& stream);
    private:
        typedef std::map<std::string, std::map<std::string, Value> > Impl;    
        Impl _impl;
    };
}

#endif
