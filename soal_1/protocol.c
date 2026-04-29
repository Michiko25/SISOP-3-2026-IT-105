#include "protocol.h"

// ambi ip n port dari file 
void get_config (Config *config, const char *filename) {
FILE *file = fopen (filename, "r");
if (file !=  NULL) {
fscanf (file, "%[^:]:%d", config->ip, &config->port);
fclose (file);
}
else {
strcpy (config->ip, "127.0.0.1");
config->port = 8080;
}
}

// catat semua ke history.log
void log_event (const char *tag, const char *message) {
FILE *fp = fopen ("history.log", "a");
if (fp == NULL) {
return;
}

time_t now = time(NULL);
struct tm *t = localtime(&now);
char timestamp[25];

strftime (timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

fprintf (fp, "[%s] %s %s\n",timestamp, tag, message);

fclose(fp);
}
