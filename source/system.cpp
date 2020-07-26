// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "system.h"
#include "path.h"
#include "splitter.h"

namespace mimeapps
{
    std::string findExecutable(const std::string& fileName)
    {
        if (!isBaseName(fileName)) {
            if (::access(fileName.c_str(), X_OK) == 0) {
                return fileName;
            } else {
                return std::string();
            }
        }

        const char* envPath = std::getenv("PATH");
        if (!envPath) {
            return std::string();
        }

        typedef Splitter<const char*> SplitterType;
        SplitterType splitter(envPath, envPath + std::strlen(envPath), ':');
        for (SplitterType::iterator it = splitter.begin(); it != splitter.end(); ++it) {
            std::string basePath(it->first, it->second);
            if (basePath.size()) {
                std::string filePath = buildPath(basePath, fileName);
                if (::access(filePath.c_str(), X_OK) == 0) {
                    return filePath;
                }
            }
        }
        return std::string();
    }

    static std::string getDETerminal() {
        const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
        if (!desktop) {
            return std::string();
        }
        if (std::strcmp(desktop, "GNOME")==0 || std::strcmp(desktop, "X-Cinnamon")==0) {
            return "gnome-terminal";
        } else if (std::strcmp(desktop, "LXDE")==0) {
            return "lxterminal";
        } else if (std::strcmp(desktop, "XFCE")==0) {
            return "xfce4-terminal";
        } else if (std::strcmp(desktop, "MATE")==0) {
            return "mate-terminal";
        } else if (std::strcmp(desktop, "KDE")==0) {
            return "konsole";
        }
        return std::string();
    }

    std::string getTerminal()
    {
        std::string term = findExecutable("x-terminal-emulator");
        if (!term.empty()) {
            return term;
        }
        term = findExecutable(getDETerminal());
        if (!term.empty()) {
            return term;
        }
        return findExecutable("xterm");
    }

    static void ignorePipeErrors()
    {
        struct sigaction ignoreAction;
        memset(&ignoreAction, 0, sizeof(ignoreAction));
        ignoreAction.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &ignoreAction, NULL);
    }

    enum InternalError
    {
        NOERROR,
        DOUBLEFORK,
        EXEC,
        CHDIR,
        GETRLIMIT,
        ENVIRONMENT
    };

    static int safePipe(int* pipefds)
    {
        int result = ::pipe(pipefds);
        if (result != 0) {
            return result;
        }
        if (::fcntl(pipefds[0], F_SETFD, FD_CLOEXEC) == -1 || ::fcntl(pipefds[1], F_SETFD, FD_CLOEXEC) == -1) {
            ::close(pipefds[0]);
            ::close(pipefds[1]);
            return -1;
        }
        return result;
    }

    static void abortOnError(int execPipeOut, InternalError errorType, int error) {
        error = error ? error : EINVAL;
        ::write(execPipeOut, &errorType, sizeof(errorType));
        ::write(execPipeOut, &error, sizeof(error));
        ::close(execPipeOut);
        ::_exit(1);
    }

    SystemError spawnDetached(char** args, const char* workingDirectory, unsigned int* pid)
    {
        int execPipe[2];
        int pidPipe[2];

        if (safePipe(execPipe) != 0) {
            return SystemError(errno, "Could not create pipe to check startup of child");
        }

        if (safePipe(pidPipe) != 0) {
            ::close(execPipe[0]);
            ::close(execPipe[1]);
            return SystemError(errno, "Could not create pipe to check startup of child");
        }

        pid_t firstFork = ::fork();
        int lastError = errno;
        if (firstFork == 0) {
            ::close(execPipe[0]);
            ::close(pidPipe[0]);

            ignorePipeErrors();

            int execPipeOut = execPipe[1];
            int pidPipeOut = pidPipe[1];

            pid_t secondFork = ::fork();
            if (secondFork == 0) {
                //detach from process group of parent
                ::setsid();

                ::close(pidPipeOut);
                ignorePipeErrors();

                if (workingDirectory && workingDirectory[0]) {
                    if (::chdir(workingDirectory) == -1) {
                        abortOnError(execPipeOut, CHDIR, errno);
                    }
                }

                //close all file descriptors
                //Possible optimization: use poll and close only valid descriptors.
                struct rlimit r;
                if (getrlimit(RLIMIT_NOFILE, &r) != 0) {
                    int maxDescriptors = (int)r.rlim_cur;
                    int i;
                    for (i=STDERR_FILENO+1; i<maxDescriptors; ++i) {
                        ::close(i);
                    }
                }

                //set standard streams to /dev/null
                int devNull = ::open("/dev/null", O_RDWR);
                if (devNull > 0) {
                    ::dup2(devNull, STDIN_FILENO);
                    ::dup2(devNull, STDOUT_FILENO);
                    ::dup2(devNull, STDERR_FILENO);
                }
                ::close(devNull);

                ::execv(args[0], args);
                abortOnError(execPipeOut, EXEC, errno);
            }

            ::write(pidPipeOut, &secondFork, sizeof(pid_t));
            ::close(pidPipeOut);

            if (secondFork == -1) {
                abortOnError(execPipeOut, DOUBLEFORK, errno);
            } else {
                ::close(execPipeOut);
                ::_exit(0);
            }

        } else if (firstFork < 0) {
            ::close(execPipe[0]);
            ::close(execPipe[1]);
            ::close(pidPipe[0]);
            ::close(pidPipe[1]);
            return SystemError(lastError, "Could not fork");
        }

        ::close(execPipe[1]);
        ::close(pidPipe[1]);

        InternalError status = NOERROR;
        int readExecResult = ::read(execPipe[0], &status, sizeof(status));
        lastError = errno;

        int waitResult;
        waitpid(firstFork, &waitResult, 0);

        if (readExecResult == -1) {
            return SystemError(lastError, "Could not read from pipe to get child status");
        }

        if (status == NOERROR) {
            ::close(execPipe[0]);
            if (pid != NULL) {
                pid_t actualPid = 0;
                if (::read(pidPipe[0], &actualPid, sizeof(pid_t)) >= 0) {
                    *pid = actualPid;
                } else {
                    *pid = 0;
                }
            }
            ::close(pidPipe[0]);
            return SystemError(0, "");
        } else {
            ::close(pidPipe[0]);
            int error;
            readExecResult = ::read(execPipe[0], &error, sizeof(error));
            lastError = errno;
            ::close(execPipe[0]);
            if (readExecResult == -1) {
                return SystemError(errno, "Error occured but could not read exec errno from pipe");
            }
            switch(status) {
                case DOUBLEFORK: return SystemError(error, "Could not fork twice");
                case EXEC: return SystemError(error, "Could not exec");
                case CHDIR: return SystemError(error, "Could not set working directory");
                case GETRLIMIT: return SystemError(error, "getrlimit");
                case ENVIRONMENT: return SystemError(error, "Could not set environment variables");
                default:return SystemError(error, "Unknown error occured");
            }
        }
    }
}
