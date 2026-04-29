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

void get_config (Config *config, const char *filename);
void log_event (const char *tag, const char *message);

#endif
