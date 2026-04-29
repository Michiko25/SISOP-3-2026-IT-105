#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "protocol.h"

#define MAX_CLIENTS 100

typedef struct {
int socket;
char name [MAX_NAME];
} Client;

Client *clients [MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t start_time;

// kirim pesan ke semua kecuali pengirim
void broadcast (Message msg, int sender_fd) {
pthread_mutex_lock (&clients_mutex);
for (int i = 0; i < MAX_CLIENTS; i++) {
if (clients[i] != NULL && clients[i]->socket != sender_fd) {
send (clients[i]->socket, &msg, sizeof(msg), 0);
}
}
pthread_mutex_unlock (&clients_mutex);
}

// handle client khusus
void *handle_client (void *arg) {
int client_fd = *((int *)arg);
free(arg);
Message msg;
char log_buf[200];
char name [MAX_NAME];
int index = -1;

// login
if (recv (client_fd, &msg, sizeof(msg), 0) > 0) {
strcpy (name, msg.sender);

if (strcmp(name, "The Knights") != 0) {
pthread_mutex_lock(&clients_mutex);
for (int i = 0; i < MAX_CLIENTS; i++) {
if (clients[i] == NULL) {
clients[i] = malloc(sizeof(Client));
clients[i]->socket = client_fd;
strcpy (clients[i]->name, name);
index = i;
break;
}
}

pthread_mutex_unlock(&clients_mutex);

sprintf (log_buf, "User '%s' connected", name);
}
else {
sprintf(log_buf, "Admin 'The Knights' connected");
}

log_event ("[System]", log_buf);
printf ("%s\n", log_buf);
}

// rutinitas chat
while (recv (client_fd, &msg, sizeof(msg), 0) > 0) {
if (msg.type == 2) {
char res[100];
switch (msg.content[0]) {
case '1': ; // cek usr aktif or no
int count = 0;
for (int i = 0; i<MAX_CLIENTS; i++) {
if (clients[i]) {
count++;
}
}

sprintf (res, "Active Users: %d", count);
break;

case '2': ; // uptime
sprintf (res, "Server Uptime: %.0f seconds", difftime(time(NULL), start_time));
break;

case '3': ; // shutdown
log_event("[System]", "[EMERGENCY SHUTDOWN INITIATED]");
printf("Emergency Shutdown by Admin!\n");
exit (0);
break;

default: 
strcpy(res, "Command ignored.");
}

strcpy(msg.content, res);
send (client_fd, &msg, sizeof(msg), 0);
log_event("[Admin]", res);
continue;
}

if (msg.type == 3 || strcmp (msg.content, "/exit") == 0) {
break;
}

printf ("[%s]: %s\n", msg.sender, msg.content);
broadcast (msg, client_fd);

sprintf (log_buf, "[%s]: %s", msg.sender, msg.content);
log_event ("[User]", log_buf);
}

// clean up
close (client_fd);

pthread_mutex_lock(&clients_mutex);
if (index != -1) {
free (clients[index]);
clients[index] = NULL;
}

pthread_mutex_unlock(&clients_mutex);

sprintf (log_buf, "User '%s' disconnected", name);
log_event ("[System]", log_buf);
printf ("%s\n", log_buf);

pthread_detach (pthread_self());
return NULL;
}

int main() {
int server_fd, new_fd;
struct sockaddr_in server_addr;
Config cfg;

start_time = time(NULL);
get_config (&cfg, "wired.conf");

server_fd = socket (AF_INET, SOCK_STREAM, 0);
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = inet_addr(cfg.ip);
server_addr.sin_port = htons (cfg.port);

bind (server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
listen (server_fd, 10);

log_event ("[System]", "[SERVER ONLINE]");
printf ("The Wired is running on %s:%d...\n", cfg.ip, cfg.port);

while (1) {
new_fd = accept (server_fd, NULL, NULL);
pthread_t tid;
int *new_sock = malloc(sizeof(int));
*new_sock = new_fd;
pthread_create (&tid, NULL, &handle_client, new_sock);
}

return 0;
}
