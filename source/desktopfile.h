#ifndef MIMEAPPS_DESKTOPFILE_H
#define MIMEAPPS_DESKTOPFILE_H

#include <algorithm>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "inilike.h"

namespace mimeapps
{
    namespace details {
        template<typename Iterator>
        std::string unescapeQuotedArgument(const Iterator& first, const Iterator& last) {
            const std::pair<char, char> pairs[] = {
                std::make_pair('`', '`'),
                std::make_pair('$', '$'),
                std::make_pair('"', '"'),
                std::make_pair('\\', '\\')
            };
            return doUnescape(first, last, &pairs[0], pairs + 4);
        }
        
        template<typename Iterator>
        std::string parseQuotedPart(Iterator& it, char delimeter, const Iterator& last)
        {
            const Iterator start = ++it;
            bool inQuotes = true;
            bool wasEscapedSlash = false;
            
            while(it != last) {
                if (*it == '\\' && (it+1 < last) && *(it+1) == '\\') {
                    it += 2;
                    wasEscapedSlash = true;
                    continue;
                }
                
                if (*it == delimeter && (*(it-1) != '\\' || (*(it-1) == '\\' && wasEscapedSlash) )) {
                    inQuotes = false;
                    break;
                }
                wasEscapedSlash = false;
                ++it;
            }
            if (inQuotes) {
                throw std::runtime_error("Missing pair quote");
            }
            
            return unescapeQuotedArgument(start, it);
        }
        
        void expand(const std::string& token, std::string& expanded, std::string::size_type& restPos, std::string::size_type& i, const std::string& insert);
    }
    
    template<typename Iterator, typename OutputIterator>
    void unquoteExec(Iterator first, Iterator last, OutputIterator out)
    {
        std::string append;
        bool isNull = true;
        bool wasInQuotes = false;
        
        Iterator it = first;
        
        while(it != last) {
            if (*it == ' ' || *it == '\t') {
                if (!wasInQuotes && append.size() >= 1 && append[append.size()-1] == '\\') {
                    append[append.size()-1] = *it;
                    isNull = false;
                } else {
                    if (!isNull) {
                        *out = append;
                        append.clear();
                        isNull = true;
                    }
                }
                wasInQuotes = false;
            } else if (*it == '"' || *it == '\'') {
                append += details::parseQuotedPart(it, *it, last);
                wasInQuotes = true;
                isNull = false;
            } else {
                append.push_back(*it);
                wasInQuotes = false;
                isNull = false;
            }
            ++it;
        }
        
        if (!isNull) {
            *out = append;
        }
    }

    template<typename OutputIterator>
    void unquoteExec(const std::string& value, OutputIterator out) {
        unquoteExec(value.begin(), value.end(), out);
    }

    template<typename Iterator>
    bool isValidDesktopFileKey(Iterator first, Iterator last)
    {
        last = std::find(first, last, '['); //separate from locale
        
        if (first == last) {
            return false;
        }
        for (Iterator it = first; it != last; ++it) {
            typename Iterator::value_type c = *it;
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-')) {
                return false;
            }
        }
        return true;
    }

    bool isValidDesktopFileKey(const std::string& str);

    template<typename Iterator, typename OutputIterator>
    void expandExecArgs(const Iterator& first, const Iterator& last, const std::string& toOpen, 
                        const std::string& iconName, const std::string& displayName, 
                        const std::string& desktopFileName, OutputIterator out)
    {
        for(Iterator it = first; it != last; ++it) {
            const std::string token = *it;
            if (token == "%F") {
                *out = toOpen;
            } else if (token == "%U") {
                *out = toOpen;
            } else if (token == "%i") {
                if (iconName.size()) {
                    *out = "--icon";
                    *out = iconName;
                }
            } else {
                std::string expanded;
                std::string::size_type restPos = 0;
                bool ignore = false;
                for(std::string::size_type i=0; i<token.size(); ++i) {
                    bool shouldBreak = false;
                    if (token[i] == '%' && i+1<token.size()) {
                        switch(token[i+1]) {
                            case 'f': case 'u':
                            {
                                details::expand(token, expanded, restPos, i, toOpen);
                            }
                            break;
                            case 'c':
                            {
                                details::expand(token, expanded, restPos, i, displayName);
                            }
                            break;
                            case 'k':
                            {
                                details::expand(token, expanded, restPos, i, desktopFileName);
                            }
                            break;
                            case 'd': case 'D': case 'n': case 'N': case 'm': case 'v':
                            {
                                ignore = true;
                                shouldBreak = true;
                            }
                            break;
                            case '%':
                            {
                                details::expand(token, expanded, restPos, i, "%");
                            }
                            break;
                            default:
                            {
                                throw std::runtime_error("Unknown or misplaced field code: " + token);
                            }
                        }
                    }
                    if (shouldBreak) {
                        break;
                    }
                }
                
                if (!ignore) {
                    *out = expanded.append(token.begin() + restPos, token.end()); 
                }
            }
        }
    }


    struct DesktopFile
    {
        enum Type {
            Unknown,
            Application,
            Link,
            Directory,
            Other
        };
        
        DesktopFile();
        DesktopFile(const std::string& fileName);
        DesktopFile(std::istream& stream, const std::string& fileName);
        
        bool isValid() const;
        
        Type type() const;
        std::string execValue() const;
        std::string name() const;
        std::string genericName() const;
        std::string comment() const;
        std::string icon() const;
        std::string workingDirectory() const;
        bool terminal() const;
        
        std::string fileName() const;
        
        template<typename OutputIterator>
        void expandExecValue(const std::string& toOpen, OutputIterator out) const {
            std::vector<std::string> unquoted;
            unquoteExec(execValue(), std::back_inserter(unquoted));
            expandExecArgs(unquoted.begin(), unquoted.end(), toOpen, icon(), name(), fileName(), out);
        }
        
        void spawnApplication(const std::string& toOpen) const;
        
    private:
        void init();
        void init(std::istream& stream);
        
        Type _type;
        std::string _execValue;
        std::string _name;
        std::string _genericName;
        std::string _comment;
        std::string _icon;
        std::string _workingDirectory;
        std::string _fileName;
        bool _terminal;
    };

}

#endif
