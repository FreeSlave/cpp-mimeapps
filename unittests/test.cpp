// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#define BOOST_TEST_MODULE Unittests
#include <boost/test/unit_test.hpp>

#include <vector>
#include <cstring>
#include <sstream>
#include <iostream>

#include "splitter.h"
#include "path.h"
#include "inilike.h"
#include "desktopfile.h"
#include "mimeapps.h"
#include "basedir.h"

using namespace mimeapps;

BOOST_AUTO_TEST_SUITE(splitter_test)

template<typename SourceIterator>
std::vector<std::string> applySplitter(SourceIterator begin, SourceIterator end)
{
    typedef Splitter<SourceIterator> SplitterType;
    SplitterType splitter(begin, end, ':');
    typename SplitterType::iterator it = splitter.begin();

    std::vector<std::string> toReturn;
    while(it != splitter.end()) {
        toReturn.push_back(std::string(it->first, it->second));
        ++it;
    }
    return toReturn;
}

std::vector<std::string> applySplitter(const char* str)
{
    return applySplitter(str, str + std::strlen(str));
}

std::vector<std::string> applySplitter(const std::string& str)
{
    return applySplitter(str.begin(), str.end());
}

BOOST_AUTO_TEST_CASE(Splitter_test)
{
    std::vector<std::string> expected, result;

    expected.push_back("one");
    expected.push_back("two");
    expected.push_back("three");
    result = applySplitter("one:two:three");
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    result = applySplitter(std::string("one:two:three"));
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    expected.clear();

    expected.push_back("one");
    result = applySplitter("one");
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    result = applySplitter(std::string("one"));
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    expected.clear();

    result = applySplitter("");
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    result = applySplitter(std::string(""));
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    expected.clear();

    std::vector<std::string> vecWithSpaces;
    expected.push_back("");
    expected.push_back("one");
    expected.push_back("");
    expected.push_back("two");
    expected.push_back("");

    result = applySplitter(":one::two:");
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    result = applySplitter(std::string(":one::two:"));
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
    expected.clear();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(path_test)

BOOST_AUTO_TEST_CASE(isBaseName_test)
{
    BOOST_CHECK(isBaseName("name"));
    BOOST_CHECK(isBaseName(".name"));
    BOOST_CHECK(!isBaseName(""));
    BOOST_CHECK(!isBaseName("/name"));
    BOOST_CHECK(!isBaseName("name/"));
    BOOST_CHECK(!isBaseName("name/sub"));

    BOOST_CHECK(isBaseName(std::string("name")));
    BOOST_CHECK(!isBaseName(std::string("name/")));
}

BOOST_AUTO_TEST_CASE(isAbsolutePath_test)
{
    BOOST_CHECK(isAbsolutePath("/name"));
    BOOST_CHECK(isAbsolutePath("/"));

    BOOST_CHECK(!isAbsolutePath(""));
    BOOST_CHECK(!isAbsolutePath(".name"));
    BOOST_CHECK(!isAbsolutePath("./name"));
    BOOST_CHECK(!isAbsolutePath("../name"));
    BOOST_CHECK(!isAbsolutePath("name"));

    BOOST_CHECK(isAbsolutePath(std::string("/name")));
    BOOST_CHECK(!isAbsolutePath(std::string("name")));
}

BOOST_AUTO_TEST_CASE(buildPath_test)
{
    BOOST_CHECK_EQUAL(buildPath("name", "sub"), "name/sub");
    BOOST_CHECK_EQUAL(buildPath("name", ""), "name");
    BOOST_CHECK_EQUAL(buildPath("/", "name"), "/name");
    BOOST_CHECK_EQUAL(buildPath("name/", "sub"), "name/sub");
    BOOST_CHECK_EQUAL(buildPath("", "sub"), "sub");
    BOOST_CHECK_EQUAL(buildPath("name", "/sub"), "/sub");
    BOOST_CHECK_EQUAL(buildPath("name/", "/sub"), "/sub");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(inilike_test)

BOOST_AUTO_TEST_CASE(unescapeValue_test)
{
    BOOST_CHECK_EQUAL(unescapeValue("a\\\\next\\nline\\top"), "a\\next\nline\top");
    BOOST_CHECK_EQUAL(unescapeValue("\\\\next\\nline\\top"), "\\next\nline\top");
    BOOST_CHECK_EQUAL(unescapeValue("noescape"), "noescape");
}

BOOST_AUTO_TEST_CASE(isTrue_test)
{
    BOOST_CHECK(isTrue("true"));
    BOOST_CHECK(isTrue("1"));
    BOOST_CHECK(!isTrue("false"));
    BOOST_CHECK(!isTrue("0"));
}

BOOST_AUTO_TEST_CASE(searchKeyValues_test)
{
    SearchRequest request;
    const std::string desktopEntry = "Desktop Entry";
    request.addRequest(desktopEntry, "Name");
    request.addRequest(desktopEntry, "Icon");
    request.addRequest(desktopEntry, "GenericName");
    request.addRequest(desktopEntry, "Comment");
    request.addRequest(desktopEntry, "Exec");
    request.addRequest(desktopEntry, "Type");
    request.addRequest(desktopEntry, "Terminal");

    std::string contents =
        "#Comment\n"
        "\n"
        "[Desktop Action Example]\n"
        "Exec=program --action\n"
        "Name=Action\n"
        "[Desktop Entry]\n"
        "Exec=program \\\\\n"
        "Name=Program\n"
        "GenericName=Software   \n"
        "Comment=Utility\n"
        "Type=Application\n"
        "Icon=application-generic\n";

    std::istringstream stream(contents);
    request.searchKeyValues(stream);

    BOOST_CHECK_EQUAL(request.getValue(desktopEntry, "Exec").value(), "program \\");
    BOOST_CHECK_EQUAL(request.getValue(desktopEntry, "Name").value(), "Program");
    BOOST_CHECK_EQUAL(request.getValue(desktopEntry, "GenericName").value(), "Software");
    BOOST_CHECK_EQUAL(request.getValue(desktopEntry, "Comment").value(), "Utility");
    BOOST_CHECK_EQUAL(request.getValue(desktopEntry, "Type").value(), "Application");
    BOOST_CHECK_EQUAL(request.getValue(desktopEntry, "Icon").value(), "application-generic");
    BOOST_CHECK(!request.getValue(desktopEntry, "Terminal").found());
    BOOST_CHECK(!request.getValue(desktopEntry, "MimeType").found());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(desktopfile_test)

BOOST_AUTO_TEST_CASE(unquoteExec_test)
{
    std::vector<std::string> vec, expected;

    unquoteExec("", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    unquoteExec("    ", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());

    expected.push_back("");
    expected.push_back("   ");
    unquoteExec("\"\"  \"   \"", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("cmd");
    expected.push_back("arg1");
    expected.push_back("arg2");
    expected.push_back("arg3");
    unquoteExec("cmd arg1  arg2   arg3   ", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("cmd");
    expected.push_back("arg1");
    expected.push_back("arg2");
    unquoteExec("\"cmd\" arg1 arg2  ", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("quoted cmd");
    expected.push_back("arg1");
    expected.push_back("quoted arg");
    unquoteExec("\"quoted cmd\"   arg1  \"quoted arg\"  ", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("quoted \"cmd\"");
    expected.push_back("arg1");
    expected.push_back("quoted \"arg\"");
    unquoteExec("\"quoted \\\"cmd\\\"\" arg1 \"quoted \\\"arg\\\"\"", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("\\$");
    unquoteExec("\"\\\\\\$\"", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("\\$");
    unquoteExec("\"\\\\$\"", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("$");
    unquoteExec("\"\\$\"", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("$");
    unquoteExec("\"$\"", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("quoted cmd");
    expected.push_back("arg");
    unquoteExec("'quoted cmd' arg", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();

    expected.push_back("test onetwo more  test");
    unquoteExec("test\\ \"one\"\"two\"\\ more\\ \\ test ", std::back_inserter(vec));
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
    vec.clear(); expected.clear();
}

BOOST_AUTO_TEST_CASE(expandExecArgs_test)
{
    std::vector<std::string> args, expected, vec;

    args.push_back("program path");
    args.push_back("%%f");
    args.push_back("%%i");
    args.push_back("%D");
    args.push_back("--deprecated=%d");
    args.push_back("%n");
    args.push_back("%N");
    args.push_back("%m");
    args.push_back("%v");
    args.push_back("--file=%f");
    args.push_back("%i");
    args.push_back("%F");
    args.push_back("--myname=%c");
    args.push_back("--mylocation=%k");
    args.push_back("100%%");

    expected.push_back("program path");
    expected.push_back("%f");
    expected.push_back("%i");
    expected.push_back("--file=one");
    expected.push_back("--icon");
    expected.push_back("folder");
    expected.push_back("one");
    expected.push_back("--myname=program");
    expected.push_back("--mylocation=location");
    expected.push_back("100%");

    expandExecArgs(args.begin(), args.end(), "one", "folder", "program", "location", std::back_inserter(vec));

    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(DesktopFile_test)
{
    std::string contents =
        "[Desktop Entry]\n"
        "Exec=vim %f\n"
        "Name=Vim\n"
        "GenericName=Text editor\n"
        "Comment=Old great thing\n"
        "Type=Application\n"
        "Icon=vim\n"
        "Terminal=true\n"
        "Path=.\n";
    std::istringstream stream(contents);

    DesktopFile file(stream, "file.desktop");

    BOOST_CHECK(file.isValid());
    BOOST_CHECK_EQUAL(file.execValue(), "vim %f");
    BOOST_CHECK_EQUAL(file.name(), "Vim");
    BOOST_CHECK_EQUAL(file.genericName(), "Text editor");
    BOOST_CHECK_EQUAL(file.comment(), "Old great thing");
    BOOST_CHECK_EQUAL(file.type(), DesktopFile::Application);
    BOOST_CHECK_EQUAL(file.workingDirectory(), ".");
    BOOST_CHECK_EQUAL(file.icon(), "vim");
    BOOST_CHECK(file.terminal());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(mimeapps_test)

BOOST_AUTO_TEST_CASE(findAssociatedApplications_test)
{
    std::vector<DesktopFile> desktopFiles;
    findAssociatedApplications("text/plain", std::back_inserter(desktopFiles));
    std::cout << "Desktop files to open text/plain:" << std::endl;
    for (std::vector<DesktopFile>::const_iterator it = desktopFiles.begin(); it != desktopFiles.end(); ++it) {
        std::cout << '\t' << it->fileName() << ": " << it->name() << " : " << it->execValue() << '\n';
    }
    std::cout << std::endl;
    desktopFiles.clear();

    findAssociatedApplications("inode/directory", std::back_inserter(desktopFiles));
    std::cout << "Desktop files to open inode/directory:" << std::endl;
    for (std::vector<DesktopFile>::const_iterator it = desktopFiles.begin(); it != desktopFiles.end(); ++it) {
        std::cout << '\t' << it->fileName() << ": " << it->name() << " : " << it->execValue() << '\n';
    }
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(getMimeAppsListPaths_test)
{
    std::vector<std::string> mimeAppsLists;
    getMimeAppsListPaths(std::back_inserter(mimeAppsLists));

    std::cout << "mimeapps.list files:\n";
    for (std::size_t i=0; i<mimeAppsLists.size(); ++i) {
        std::cout << mimeAppsLists[i] << '\n';
    }
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(getMimeInfoCachePaths_test)
{
    std::vector<std::string> mimeInfoCaches;
    getMimeInfoCachePaths(std::back_inserter(mimeInfoCaches));

    std::cout << "mimeinfo.cache files:\n";
    for (std::size_t i=0; i<mimeInfoCaches.size(); ++i) {
        std::cout << mimeInfoCaches[i] << '\n';
    }
    std::cout << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(basedir_test)

BOOST_AUTO_TEST_CASE(basedir_paths_test)
{
    std::cout << "Config home:\t" << configHome() << '\n';
    std::cout << "Data home:\t" << dataHome() << '\n';

    std::vector<std::string> configs;
    configDirs(std::back_inserter(configs));
    std::vector<std::string> datas;
    dataDirs(std::back_inserter(datas));

    std::cout << "Config directories:\n";
    for (std::size_t i=0; i<configs.size(); ++i) {
        std::cout << configs[i] << '\n';
    }

    std::cout << "Data directories:\n";
    for (std::size_t i=0; i<datas.size(); ++i) {
        std::cout << datas[i] << '\n';
    }
}

BOOST_AUTO_TEST_SUITE_END()
