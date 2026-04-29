#include "arena.h"

void log_event (const char *tag, const char *msg) {
FILE *f = fopen ("history.log", "a");
if (!f) {
return;
}

time_t now = time(NULL);
fprintf (f, "[%ld] %s %s\n", now, tag, msg);
fclose(f);
}

int main() {
// 1. shared memory
int shmid= shmget(SHM_KEY, sizeof(GameData), IPC_CREAT | 0666);
GameData *data = (GameData *)shmat(shmid, NULL, 0);

if (data->players[0].gold == 0) {
memset(data, 0, sizeof(GameData));
}

// 2. semaphore
sem_t *gate = sem_open(SEM_NAME, O_CREAT, 0666, 1);

// 3. message q
int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
printf ("Orion is ready (PID: %d)\n", getpid());

log_event("[System]", "SERVER ONLINE");

// orion stay alive agar IPC tidak hilang
while (1) {
sleep(10);
}

shmdt(data);
sem_close(gate);
return 0;
}
