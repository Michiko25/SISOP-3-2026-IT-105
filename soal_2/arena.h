#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define SHM_KEY 0x00001234
#define MSG_KEY 0x00005678
#define SEM_NAME "/eterion_gate"

#define MAX_USERS 50
#define MAX_HISTORY 5

typedef struct {
char time[10];
char opponent[50];
char result[10]; // win or loss
int xp_gain;
} BattleHistory;

typedef struct {
char username[50];
char password[50];
int gold;
int lvl;
int xp;
int weapon_bonus;
int is_logged_in;
int in_match; // flag matchmaking
BattleHistory history[10]; 
int log_count;
} User;

typedef struct {
User players[MAX_USERS];
}GameData;

struct msg_match {
long mtype;
int player_index;
};

void log_event (const char *tag, const char *msg);

#endif
