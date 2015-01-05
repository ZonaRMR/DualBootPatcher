/*
 * Copyright (C) 2014  Xiao-Long Chen <chenxiaolong@cxl.epac.to>
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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int mb_run_shell_command(const char *command)
{
    // If /sbin/sh exists (eg. in recovery), then fork and run that. Otherwise,
    // just call system().

    struct stat sb;
    if (stat("/sbin/sh", &sb) == 0) {
        int status;
        pid_t pid;
        if ((pid = fork()) >= 0) {
            if (pid == 0) {
                execlp("/sbin/sh", "sh", "-c", command, NULL);
                exit(127);
            } else {
                pid = waitpid(pid, &status, 0);
            }
        }

        return pid == -1 ? -1 : status;
    } else {
        return system(command);
    }
}

int mb_run_command(char * const argv[])
{
    if (!argv) {
        return -1;
    }
    if (!argv[0]) {
        return -1;
    }

    int status;
    pid_t pid;
    if ((pid = fork()) >= 0) {
        if (pid == 0) {
            execvp(argv[0], argv);
            exit(127);
        } else {
            pid = waitpid(pid, &status, 0);
        }
    }

    return pid == -1 ? -1 : status;
}
