// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <algorithm>
#include <stdexcept>
#include <cstddef>

#include "inilike.h"

namespace mimeapps
{
    std::string unescapeValue(const std::string& value) {
        return unescapeValue(value.begin(), value.end());
    }

    bool isTrue(const std::string& str) {
        return str == "true" || str == "1";
    }

    SearchRequest::Value::Value() : _found(false) {}

    bool SearchRequest::Value::found() const {
        return _found;
    }

    std::string SearchRequest::Value::value() const {
        return _value;
    }

    void SearchRequest::Value::setValue(const std::string& value) {
        _value = value;
        _found = true;
    }

    static void trimRight(std::string& str)
    {
        std::string::size_type found = str.find_last_not_of(" \t\n\r\v\f");
        if (found == std::string::npos) {
            str.clear();
        } else {
            str.erase(found+1);
        }
    }

    void SearchRequest::addRequest(const std::string& group, const std::string& key)
    {
        _impl[group][key] = Value();
    }

    SearchRequest::Value SearchRequest::getValue(const std::string& group, const std::string& key)
    {
        SearchRequest::Impl::iterator groupIt = _impl.find(group);
        if (groupIt != _impl.end()) {
            std::map<std::string, Value>::iterator searchIt = groupIt->second.find(key);
            if (searchIt != groupIt->second.end()) {
                return searchIt->second;
            }
        }
        return Value();
    }

    void SearchRequest::searchKeyValues(std::istream& stream)
    {
        std::string line;
        std::string currentGroup;
        while(getline(stream, line)) {
            trimRight(line);
            if (line.empty() || line[0] == '#') {
                continue;
            }

            if (line[0] == '[') {
                std::string::iterator closeBracketIt = std::find(line.begin(), line.end(), ']');
                if (closeBracketIt == line.end()) {
                    throw std::runtime_error("No closing ']' found");
                }
                currentGroup = std::string(line.begin() + 1, closeBracketIt);
                if (currentGroup.empty()) {
                    throw std::runtime_error("Empty group name");
                }
            } else {
                std::string::iterator equalIt = std::find(line.begin(), line.end(), '=');
                if (equalIt == line.end()) {
                    throw std::runtime_error("No '=' found");
                }
                if (currentGroup.empty()) {
                    throw std::runtime_error("Key-value pair outside of group");
                }
                SearchRequest::Impl::iterator groupIt = _impl.find(currentGroup);
                if (groupIt != _impl.end()) {
                    std::string key(line.begin(), equalIt);
                    std::map<std::string, Value>::iterator searchIt = groupIt->second.find(key);
                    if (searchIt != groupIt->second.end()) {
                        searchIt->second.setValue(unescapeValue(equalIt+1, line.end()));
                    }
                }
            }
        }
    }
}
