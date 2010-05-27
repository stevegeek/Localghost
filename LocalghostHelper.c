/*
 * Copyright 2010 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#define HOSTS_FILE "/etc/hosts"
#define BUFFER_SIZE 1024

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PRINT_LINE \
    fprintf(hosts, "%s\n", line)

#define PRINT_USAGE \
    fprintf(stderr, "Must be called with --enable or --disable and a host name\n"); \
    return 1

static void set_enabled(const char *host, int enabled)
{
    FILE *hosts = NULL;
    char buffer[BUFFER_SIZE];
    char *content = NULL;
    size_t content_size = 0;
    size_t bytes_read = 0;
    char *line = NULL;
    BOOL found = FALSE;

    hosts = fopen(HOSTS_FILE, "r+");

    if(!hosts)
    {
        printf("Could not open %s for writing.\n", HOSTS_FILE);
        fprintf(stderr, "Could not open %s for writing.\n", HOSTS_FILE);
        return;
    }

    flock(fileno(hosts), LOCK_EX);

    while((bytes_read = fread(&buffer[0], sizeof(char), BUFFER_SIZE, hosts)) > 0)
    {
        content = realloc(content, content_size + bytes_read + 1);
        memcpy(&content[content_size], buffer, bytes_read);
        content_size += bytes_read;
        content[content_size] = '\0';
    }

    hosts = freopen(HOSTS_FILE, "w", hosts);

    while((line = strtok(line ? 0 : content, "\n")))
    {
        if(strlen(line) > 0 && line[0] == '#')
        {
            PRINT_LINE;
        }
        else if(strstr(line, host))
        {
            if(strstr(line, "127.0.0.1"))
            {
                if(enabled)
                {
                    found = TRUE;
                    PRINT_LINE;
                }
            }
            else
            {
                fprintf(hosts, "# %s\n", line);
            }
        }
        else
        {
            PRINT_LINE;
        }
    }

    if(enabled && !found)
    {
        fprintf(hosts, "127.0.0.1\t%s\n", host);
    }

    flock(fileno(hosts), LOCK_UN);
    fclose(hosts);
    free(content);
}

int main(int argc, char *argv[])
{
    const char *mode = NULL;
    const char *host = NULL;

    if(argc != 3)
    {
        PRINT_USAGE;
    }

    mode = argv[1];
    host = argv[2];

    if(strcmp(mode, "--enable") == 0)
    {
        set_enabled(host, TRUE);
    }
    else if(strcmp(mode, "--disable") == 0)
    {
        set_enabled(host, FALSE);
    }
    else
    {
        PRINT_USAGE;
    }

    return 0;
}
