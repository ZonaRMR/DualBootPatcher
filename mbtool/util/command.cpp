/*
 * Copyright (C) 2014  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of MultiBootPatcher
 *
 * MultiBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MultiBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util/command.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util/logging.h"

namespace MB {

int run_shell_command(const std::string &command)
{
    // If /sbin/sh exists (eg. in recovery), then fork and run that. Otherwise,
    // just call system().

    struct stat sb;
    if (stat("/sbin/sh", &sb) == 0) {
        int status;
        pid_t pid;
        if ((pid = fork()) >= 0) {
            if (pid == 0) {
                execlp("/sbin/sh", "sh", "-c", command.c_str(), NULL);
                exit(127);
            } else {
                pid = waitpid(pid, &status, 0);
            }
        }

        return pid == -1 ? -1 : status;
    } else {
        return system(command.c_str());
    }
}

int run_command(const std::vector<std::string> &argv)
{
    if (argv.empty()) {
        errno = EINVAL;
        return -1;
    }

    std::vector<const char *> argv_c;
    for (const std::string &arg : argv) {
        argv_c.push_back(arg.c_str());
    }
    argv_c.push_back(nullptr);

    int status;
    pid_t pid;

    if ((pid = fork()) >= 0) {
        if (pid == 0) {
            execvp(argv_c[0], const_cast<char * const *>(argv_c.data()));
            exit(127);
        } else {
            pid = waitpid(pid, &status, 0);
        }
    }

    return pid == -1 ? -1 : status;
}

int run_command_chroot(const std::string &dir,
                       const std::vector<std::string> &argv)
{
    if (dir.empty() || argv.empty()) {
        errno = EINVAL;
        return -1;
    }

    std::vector<const char *> argv_c;
    for (const std::string &arg : argv) {
        argv_c.push_back(arg.c_str());
    }
    argv_c.push_back(nullptr);

    int status;
    pid_t pid;

    if ((pid = fork()) >= 0) {
        if (pid == 0) {
            if (chdir(dir.c_str()) < 0) {
                LOGE("%s: Failed to chdir: %s", dir, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (chroot(dir.c_str()) < 0) {
                LOGE("%s: Failed to chroot: %s", dir, strerror(errno));
                exit(EXIT_FAILURE);
            }

            execvp(argv_c[0], const_cast<char * const *>(argv_c.data()));
            exit(127);
        } else {
            pid = waitpid(pid, &status, 0);
        }
    }

    return pid == -1 ? -1 : status;
}

}