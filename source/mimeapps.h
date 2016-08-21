// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef MIMEAPPS_H
#define MIMEAPPS_H

#include <algorithm>
#include <iterator>
#include <vector>
#include <fstream>
#include <cstdlib>

#include <sys/stat.h>

#include "basedir.h"
#include "inilike.h"
#include "desktopfile.h"
#include "path.h"
#include "splitter.h"
#include "system.h"

namespace mimeapps
{
    template<typename OutputIterator>
    void getMimeAppsListPaths(OutputIterator out)
    {
        *out = buildPath(configHome(), "mimeapps.list");
        *out = buildPath(dataHome(), "applications/mimeapps.list");
        configDirs(out, "mimeapps.list");
        dataDirs(out, "applications/mimeapps.list");
    }
    
    template<typename OutputIterator>
    void getMimeInfoCachePaths(OutputIterator out)
    {
        *out = buildPath(dataHome(), "applications/mimeinfo.cache");
        dataDirs(out, "applications/mimeinfo.cache");
    }
    
    template<typename OutputIterator>
    void getApplicationsPaths(OutputIterator out)
    {
        *out = buildPath(dataHome(), "applications");
        dataDirs(out, "applications");
    }
    
    template<typename Iterator>
    std::string findDesktopFile(const Iterator& first, const Iterator& last, const std::string& desktopId) {
        if (!isBaseName(desktopId)) {
            return std::string();
        }
        
        struct stat st;
        for (Iterator it = first; it != last; ++it) {
            std::string appPath = buildPath(*it, desktopId);
            if (::stat(appPath.c_str(), &st) == 0) {
                return appPath;
            }
        }
        
        std::string::size_type i = desktopId.rfind('-');
        if (i != std::string::npos) {
            std::string copy = desktopId;
            copy[i] = '/';
            for (Iterator it = first; it != last; ++it) {
                std::string appPath = buildPath(*it, copy);
                if (::stat(appPath.c_str(), &st) == 0) {
                    return appPath;
                }
            }
        }
        return std::string();
    }
    
    template<typename OutputIterator>
    void listAssociatedApplications(const std::string& mimeType, OutputIterator out)
    {
        std::vector<std::string> removed, mimeAppsListPaths, mimeInfoCachePaths, desktopIds;
        getMimeAppsListPaths(std::back_inserter(mimeAppsListPaths));
        getMimeInfoCachePaths(std::back_inserter(mimeInfoCachePaths));
        
        typedef Splitter<std::string::const_iterator> SplitterType;
        
        for (std::vector<std::string>::iterator it = mimeAppsListPaths.begin(); it != mimeAppsListPaths.end(); ++it) {
            const std::string mimeApps = *it;
            try {
                std::ifstream stream(mimeApps.c_str());
                if (stream.is_open()) {
                    SearchRequest request;
                    request.addRequest("Added Associations", mimeType);
                    request.addRequest("Removed Associations", mimeType);
                    request.searchKeyValues(stream);
                    
                    const std::string removedAppsStr = request.getValue("Removed Associations", mimeType).value();
                    SplitterType removedAppsSplitter(removedAppsStr.begin(), removedAppsStr.end(), ';');
                    for (SplitterType::iterator it = removedAppsSplitter.begin(); it != removedAppsSplitter.end(); ++it) {
                        removed.push_back(std::string(it->first, it->second));
                    }
                    
                    const std::string addedAppsStr = request.getValue("Added Associations", mimeType).value();
                    SplitterType addedAppsSplitter(addedAppsStr.begin(), addedAppsStr.end(), ';');
                    for (SplitterType::iterator it = addedAppsSplitter.begin(); it != addedAppsSplitter.end(); ++it) {
                        const std::string desktopId(it->first, it->second);
                        if (!desktopId.empty() && std::find(removed.begin(), removed.end(), desktopId) == removed.end() && 
                            std::find(desktopIds.begin(), desktopIds.end(), desktopId) == desktopIds.end()) {
                            desktopIds.push_back(desktopId);
                            *out = desktopId;
                        }
                    }
                }
            } catch(std::exception& e) {
                
            }
        }
        
        for (std::vector<std::string>::iterator it = mimeInfoCachePaths.begin(); it != mimeInfoCachePaths.end(); ++it) {
            const std::string mimeCache = *it;
            try {
                std::ifstream stream(mimeCache.c_str());
                if (stream.is_open()) {
                    SearchRequest request;
                    request.addRequest("MIME Cache", mimeType);
                    request.searchKeyValues(stream);
                    
                    const std::string mimeAppsStr = request.getValue("MIME Cache", mimeType).value();
                    SplitterType splitter(mimeAppsStr.begin(), mimeAppsStr.end(), ';');
                    for (SplitterType::iterator it = splitter.begin(); it != splitter.end(); ++it) {
                        const std::string desktopId(it->first, it->second);
                        if (!desktopId.empty() && std::find(removed.begin(), removed.end(), desktopId) == removed.end() && 
                            std::find(desktopIds.begin(), desktopIds.end(), desktopId) == desktopIds.end()) {
                            desktopIds.push_back(desktopId);
                            *out = desktopId;
                        }
                    }
                }
                
            } catch(std::exception& e) {
                
            }
        }
    }
    
    template<typename OutputIterator>
    void listDefaultApplications(const std::string& mimeType, OutputIterator out)
    {
        std::vector<std::string> mimeAppsListPaths, desktopIds;
        getMimeAppsListPaths(std::back_inserter(mimeAppsListPaths));
        
        typedef Splitter<std::string::const_iterator> SplitterType;
        
        for (std::vector<std::string>::iterator it = mimeAppsListPaths.begin(); it != mimeAppsListPaths.end(); ++it) {
            const std::string mimeApps = *it;
            try {
                std::ifstream stream(mimeApps.c_str());
                if (stream.is_open()) {
                    SearchRequest request;
                    request.addRequest("Default Applications", mimeType);
                    request.searchKeyValues(stream);
                    
                    std::string appsStr = request.getValue("Default Applications", mimeType).value();
                    SplitterType splitter(appsStr.begin(), appsStr.end(), ';');
                    for (SplitterType::iterator it = splitter.begin(); it != splitter.end(); ++it) {
                        const std::string desktopId(it->first, it->second);
                        if (!desktopId.empty() && std::find(desktopIds.begin(), desktopIds.end(), desktopId) == desktopIds.end()) {
                            desktopIds.push_back(desktopId);
                            *out = desktopId;
                        }
                    }
                }
            } catch(std::exception& e) {
                
            }
        }
    }
    
    namespace details {
        bool isDesktopFileOk(const DesktopFile& file) {
            if (file.isValid()) {
                std::vector<std::string> args;
                unquoteExec(file.execValue(), std::back_inserter(args));
                if (!args.empty() && !findExecutable(args[0]).empty()) {
                    return true;
                }
            }
            return false;
        }
    }
    
    template<typename OutputIterator>
    void findAssociatedApplications(const std::string& mimeType, OutputIterator out) {
        std::vector<std::string> applicationsPaths, desktopIds;
        getApplicationsPaths(std::back_inserter(applicationsPaths));
        listAssociatedApplications(mimeType, std::back_inserter(desktopIds));
        
        for (std::vector<std::string>::const_iterator it = desktopIds.begin(); it != desktopIds.end(); ++it) {
            try {
                std::string desktopFilePath = findDesktopFile(applicationsPaths.begin(), applicationsPaths.end(), *it);
                if (!desktopFilePath.empty()) {
                    DesktopFile file(desktopFilePath);
                    if (details::isDesktopFileOk(file)) {
                        *out = file;
                    }
                }
            } catch(std::exception& e) {
                
            }
        }
    }
    
    DesktopFile findDefaultApplication(const std::string& mimeType) {
        std::vector<std::string> applicationsPaths, defaultDesktopIds, desktopIds;
        getApplicationsPaths(std::back_inserter(applicationsPaths));
        listDefaultApplications(mimeType, std::back_inserter(defaultDesktopIds));
        
        for (std::vector<std::string>::const_iterator it = defaultDesktopIds.begin(); it != defaultDesktopIds.end(); ++it) {
            try {
                std::string desktopFilePath = findDesktopFile(applicationsPaths.begin(), applicationsPaths.end(), *it);
                if (!desktopFilePath.empty()) {
                    DesktopFile file(desktopFilePath);
                    if (details::isDesktopFileOk(file)) {
                        return file;
                    }
                }
            } catch(std::exception& e) {
                
            }
        }
        
        listAssociatedApplications(mimeType, std::back_inserter(desktopIds));
        
        for (std::vector<std::string>::const_iterator it = desktopIds.begin(); it != desktopIds.end(); ++it) {
            try {
                std::string desktopFilePath = findDesktopFile(applicationsPaths.begin(), applicationsPaths.end(), *it);
                if (!desktopFilePath.empty()) {
                    DesktopFile file(desktopFilePath);
                    if (details::isDesktopFileOk(file)) {
                        return file;
                    }
                }
            } catch(std::exception& e) {
                
            }
        }
        return DesktopFile();
    }
}

#endif
