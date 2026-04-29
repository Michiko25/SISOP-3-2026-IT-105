#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BUF 1024
#define MAX_NAME 50

// client - server
typedef struct {
char sender [MAX_NAME];
char content [MAX_BUF];
int type; // login, chat, command, exit.
} Message;

// simpan info dari config file
typedef struct {
char ip[20];
int port;
} Config;

static inline void get_config (Config *cfg, const char *filename) {
FILE *f = fopen(filename, "r");
if (!f) {
strcpy(cfg->ip, "127.0.0.1");
cfg->port = 8080;
return;
}

if (fscanf(f, "%[^:]:%d", cfg->ip, &cfg->port) != 2) {
strcpy(cfg->ip, "127.0.0.1");
cfg->port = 8080;
}
fclose(f);
}

static inline void log_event (const char *tag, const char *msg) {
FILE *f = fopen ("history.log", "a");
if (!f) {
return;
}

time_t now = time(NULL);
struct tm *t = localtime(&now);
char ts[25];
strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

fprintf(f, "[%s] %s %s\n", ts, tag, msg);
fclose(f);
}

#endif

