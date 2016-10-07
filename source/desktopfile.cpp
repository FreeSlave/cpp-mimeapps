// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <fstream>
#include <cstddef>
#include <stdexcept>
#include "desktopfile.h"
#include "system.h"

namespace mimeapps
{
    void details::expand(const std::string& token, std::string& expanded, std::string::size_type& restPos, std::string::size_type& i, const std::string& insert)
    {
        if (token.size() == 2) {
            expanded = insert;
        } else {
            expanded.append(token.begin() + restPos, token.begin()+i).append(insert);
        }
        restPos = i+2;
        ++i;
    }
    
    bool isValidDesktopFileKey(const std::string& str) {
        return isValidDesktopFileKey(str.begin(), str.end());
    }
    
    DesktopFile::DesktopFile() {
        init();
    }
    
    DesktopFile::DesktopFile(std::istream& stream, const std::string& fileName) : _fileName(fileName) {
        init(stream);
    }
    DesktopFile::DesktopFile(const std::string& fileName) : _fileName(fileName) {
        init();
        std::ifstream file(fileName.c_str(), std::ifstream::binary);
        if(file.is_open()) {
            init(file);
        }
    }
    
    void DesktopFile::init() {
        _type = Unknown;
        _terminal = false;
    }
    
    void DesktopFile::init(std::istream& stream) {
        init();
        
        SearchRequest request;
        const std::string desktopEntry = "Desktop Entry";
        request.addRequest(desktopEntry, "Type");
        request.addRequest(desktopEntry, "Exec");
        request.addRequest(desktopEntry, "Name");
        request.addRequest(desktopEntry, "GenericName");
        request.addRequest(desktopEntry, "Comment");
        request.addRequest(desktopEntry, "Icon");
        request.addRequest(desktopEntry, "Path");
        request.addRequest(desktopEntry, "Terminal");
        
        try {
            request.searchKeyValues(stream);
            
            const std::string typeStr = request.getValue(desktopEntry, "Type").value();
            if (typeStr.size()) {
                if (typeStr == "Application") {
                    _type = Application;
                } else if (typeStr == "Link") {
                    _type = Link;
                } else if (typeStr == "Directory") {
                    _type = Directory;
                } else {
                    _type = Other;
                }
            }
            
            _execValue = request.getValue(desktopEntry, "Exec").value();
            _name = request.getValue(desktopEntry, "Name").value();
            _genericName = request.getValue(desktopEntry, "GenericName").value();
            _comment = request.getValue(desktopEntry, "Comment").value();
            _icon = request.getValue(desktopEntry, "Icon").value();
            _workingDirectory = request.getValue(desktopEntry, "Path").value();
            _terminal = isTrue(request.getValue(desktopEntry, "Terminal").value());
        } catch(std::exception& e) {
            _type = Unknown;
        }
    }
    
    bool DesktopFile::isValid() const {
        return _type != Unknown;
    }
    
    DesktopFile::Type DesktopFile::type() const {
        return _type;
    }
    std::string DesktopFile::execValue() const {
        return _execValue;
    }
    std::string DesktopFile::name() const {
        return _name;
    }
    std::string DesktopFile::genericName() const {
        return _genericName;
    }
    std::string DesktopFile::comment() const {
        return _comment;
    }
    std::string DesktopFile::icon() const {
        return _icon;
    }
    std::string DesktopFile::workingDirectory() const {
        return _workingDirectory;
    }
    bool DesktopFile::terminal() const {
        return _terminal;
    }
    std::string DesktopFile::fileName() const {
        return _fileName;
    }
    
    void DesktopFile::spawnApplication(const std::string& toOpen) const {
        std::vector<std::string> argv;
        if (terminal()) {
            std::string term = getTerminal();
            if (term.empty()) {
                throw std::runtime_error("Could not find terminal emulator required to run this application");
            }
            argv.push_back(term);
            argv.push_back("-e");
        }
        std::size_t executablePos = argv.size();
        expandExecValue(toOpen, std::back_inserter(argv));
        
        if (argv.size() == executablePos) {
            throw std::runtime_error("Incorrect Exec entry");
        }
        std::string executable = findExecutable(argv[executablePos]);
        if (executable.empty()) {
            throw std::runtime_error("Could not find executable to run");
        }
        argv[executablePos] = executable;
        
        SystemError result = spawnDetached(argv.begin(), argv.end());
        if (result.status != 0) {
            throw std::runtime_error(result.errorMsg);
        }
    }
}
