#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "protocol.h"

int sock_fd;
char username[MAX_NAME];

//thread dengerin broadcast dri server
void *receive_handler (void *arg) {
Message msg;
while (recv (sock_fd, &msg, sizeof(msg), 0) > 0) {
printf ("\r[%s]: %s\n> ", msg.sender, msg.content);
fflush (stdout);
}

return NULL;
}

int main() {
struct sockaddr_in server_addr;
Config cfg;
pthread_t tid;

get_config(&cfg, "wired.conf");

sock_fd = socket(AF_INET, SOCK_STREAM, 0);

server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = inet_addr(cfg.ip);
server_addr.sin_port = htons (cfg.port);

if (connect (sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
printf ("Gagal konek ke The Wired.\n");
return 1;
}

// inisialisasi nama
printf ("Enter your name: ");
fgets (username, MAX_NAME, stdin);
username[strcspn(username, "\n")] = 0;

if (strcmp(username, "The Knights") == 0) {
char pass [50];
printf ("Enter Password: ");
system ("stty -echo"); // hide ketikan pw
scanf ("%s", pass);
system ("stty echo"); // kembalikan ke normal
printf ("\n");

if (strcmp(pass, "protocol7") != 0) {
printf ("[System] Wrong Password!\n");
close(sock_fd);
return 0;
}

// admin
Message admin_msg;
strcpy(admin_msg.sender, username);
admin_msg.type = 0;
send (sock_fd, &admin_msg, sizeof(admin_msg), 0);

while (1) {
printf ("\n--- THE KNIGHTS CONSOLE ---\n");
printf ("1. Check Entities\n2. Uptime\n3. Shutdown\n4. Disconnect\n>> ");
char op[10];
scanf ("%s", op);
getchar();

strcpy(admin_msg.content, op);
admin_msg.type = 2; // tanda pesan admin 
send (sock_fd, &admin_msg, sizeof(admin_msg), 0);

if (op[0] == '4') {
break;
}
if (op[0] == '3') {
exit(0);
}

recv(sock_fd, &admin_msg, sizeof(admin_msg), 0);
printf ("[Result] %s\n", admin_msg.content);
}

close(sock_fd);
return 0;

}

// user
Message login_msg;
strcpy (login_msg.sender, username);
login_msg.type = 0;
send (sock_fd, &login_msg, sizeof(login_msg), 0);

printf ("--- Welcome to The Wired, %s ---\n", username);

pthread_create (&tid, NULL, &receive_handler, NULL);

Message chat_msg;
strcpy (chat_msg.sender, username);
chat_msg.type = 1;

while (1) {
printf ("> ");
fgets (chat_msg.content, MAX_BUF, stdin);
chat_msg.content[strcspn(chat_msg.content, "\n")] = 0;

if (strcmp(chat_msg.content, "/exit") == 0) {
chat_msg.type = 3;
send (sock_fd, &chat_msg, sizeof(chat_msg), 0);
break;
}

send (sock_fd, &chat_msg, sizeof(chat_msg), 0);
}

printf ("[System] Disconnecting from the Wired...\n");
close (sock_fd);
return 0;
}
