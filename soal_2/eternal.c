#include "arena.h"
#include <termios.h>
#include <errno.h>

// baca input tanpa enter (real-time battle)
int getch() {
struct termios oldt, newt;
int chara;
tcgetattr(STDIN_FILENO, &oldt);
newt = oldt;
newt.c_lflag &= ~(ICANON | ECHO);
tcsetattr(STDIN_FILENO, TCSANOW, &newt);
chara = getchar();
tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
return chara;
}

// tampilan profile
void tampilkan_profile (User *u) {
printf("\n      PROFILE\n");
printf("Name : %-10s Lvl : %d\n", u->username, u->lvl);
printf("Gold : %-10d XP  : %d\n", u->gold, u->xp);
printf("---------------------------\n");
}

void fase_battle (User *player, User *enemy, int is_bot, sem_t *gate) {
int p_hp = 100, e_hp = 100;
int p_dmg = 10 + (player->xp / 50) + player->weapon_bonus;
int e_dmg = is_bot ? 10 : (enemy->xp / 50) + enemy->weapon_bonus;
time_t last_atk = 0;

printf ("\nBATTLE START against %s!\n", is_bot ? "Monster (Bot)" : enemy->username);

while (p_hp > 0 && e_hp > 0) {
printf ("\r\33[2K[YOU: %d/100] VS [%s: %d/100] | (a) Attack (u) Ulti ", p_hp, is_bot ? "BOT" : enemy->username, e_hp);
fflush(stdout);

char c = getch();
time_t now = time(NULL);

if (c == 'a' && difftime(now, last_atk) >= 1) {
e_hp -= p_dmg;
if (e_hp < 0) {
e_hp = 0;
}
last_atk = now;
p_hp -= e_dmg;
}
else if (c == 'u' && player->weapon_bonus > 0) {
e_hp -= (p_dmg * 3);
p_hp -= e_dmg;
}
}

// print 0/100
printf("\r\33[2K[YOU: %d/100] VS [%s: %d/100] | (a) Attack (u) Ulti", p_hp, is_bot ? "BOT" : enemy->username, e_hp);
fflush(stdout);

sem_wait(gate);
if (p_hp > 0) {
printf ("\nVICTORY!\n");
player->xp += 50; player->gold += 120;
}
else {
printf("\nDEFEAT!\n");
player->xp += 15; player->gold += 30;
}

player->lvl = 1 + (player->xp / 100);

int h_idx = player->log_count % 10;
time_t t = time(NULL);
struct tm *tm_info = localtime(&t);
strftime(player->history[h_idx].time, 10, "%H:%M", tm_info);
strcpy(player->history[h_idx].opponent, is_bot ? "Monster" : enemy->username);
strcpy(player->history[h_idx].result, p_hp > 0 ? "WIN" : "LOSS");

player->history[h_idx].xp_gain = p_hp > 0 ? 50 : 15;
player->log_count++;

sem_post(gate);
}

// matchmaking 35 dtik
void matchmaking (User *u, int msgid, GameData *data, sem_t *gate) {
struct msg_match mm;
mm.mtype = 1;
mm.player_index = -1;

printf ("Searching for an opponent... (35s)\n");
time_t start_m = time(NULL);
int found = 0;

while (difftime(time(NULL), start_m) < 35) {
if (msgrcv(msgid, &mm, sizeof(mm.player_index), 1, IPC_NOWAIT) != -1) { 
User *opponent = &data->players[mm.player_index];
if (!opponent->in_match) {
fase_battle(u, opponent, 0, gate);
found = 1;
break;
}
}

printf (".");
fflush(stdout);
sleep(1);
}

if (!found) {
printf ("\nOpponent not found. Fighting Monster (Bot)...\n");
fase_battle(u, NULL, 1, gate);
}
}

// armory
void armory (User *u, sem_t *gate) {
printf ("\n--- ARMORY ---\n");
printf ("1. Wood Sword (100G) +5 Dmg\n2. Iron Sword (300G) +15 Dmg\n0. Back\nChoice: ");
int c;
scanf ("%d", &c);

sem_wait(gate);

if (c== 1 && u->gold >= 100) {
u->gold -= 100;
u->weapon_bonus = 5;
}
else if (c == 2 && u->gold >= 300) {
u->gold -= 300;
u->weapon_bonus = 15;
}

sem_post(gate);

}

void menu_game (User *u, GameData *data, sem_t *gate, int msgid) {
int choice;
while(1) {
tampilkan_profile(u);
printf ("1. Battle\n2. Armory\n3. History\n4. Logout\n> Choice: ");
scanf ("%d", &choice);

if (choice == 1) {
matchmaking(u, msgid, data, gate);
}
else if (choice == 2) {
armory(u, gate);
}
else if (choice == 3) {
printf ("\n       MATCH HISTORY\n---------------------------------\n");
printf ("Time  | Opponent   | Res  | XP\n---------------------------------\n");
for (int i = 0; i < 10; i++) {
if (u->history[i].time[0] != '\0') {
printf ("%-5s | %-10s | %-4s | +%d XP\n", u->history[i].time, u->history[i].opponent, u->history[i].result, u->history[i].xp_gain);
}
}
printf ("---------------------------------\nPress any key to back...");
getch();
}
else if (choice == 4) {
sem_wait(gate);
u->is_logged_in = 0;
sem_post(gate);
break;
}
}
}

void main_menu (GameData *data, sem_t *gate, int msgid) {
int choice;
User *logged_in_user = NULL;

while (1) {
printf("\n1. Register\n2. Login\n3. Exit\nChoice: ");
scanf("%d", &choice);

if (choice == 1) {
char u[50], p[50];
printf ("\nCREATE ACCOUNT\n");
printf("Username: ");
scanf("%s", u);

printf("Password: "); 
scanf("%s", p);

sem_wait(gate); // lock shared mem
int found = 0;
for (int i = 0; i < MAX_USERS; i++) {
if (strcmp(data->players[i].username, u) == 0) {
found = 1;
break;
}
}

if (found) {
printf ("Username sudah didaftarkan!\n");
}
else {
for (int i = 0; i < MAX_USERS; i++) {
if (data->players[i].username[0] == '\0') {
strcpy(data->players[i].username, u);
strcpy(data->players[i].password, p);

data->players[i].gold = 150;
data->players[i].lvl = 1;
data->players[i].xp = 0;
data->players[i].is_logged_in = 0;
printf ("Account created!\n");
break;
}
}
}

sem_post(gate); // unlock
}

//login
else if (choice == 2) {
char u[50], p[50];
printf ("\nLOGIN\n");
printf ("Username: ");
scanf("%s", u);

printf("Password: ");
scanf("%s", p);

sem_wait(gate);
int success = 0;
for (int i = 0; i < MAX_USERS; i++) {
if (strcmp(data->players[i].username, u) == 0 &&
strcmp(data->players[i].password, p) == 0) {
if (data->players[i].is_logged_in) {
printf("Akun sedang aktif di sesi lain!\n");
}
else {
data->players[i].is_logged_in = 1;
logged_in_user = &data->players[i];
success = 1;
}

break;
}
}

sem_post(gate);

if (success) {
printf ("Welcome!\n");
menu_game (logged_in_user, data, gate, msgid);
}
else if (!success) {
printf("Username atau password salah!\n");
}
}
else {
break;
}
}
}

int main() {
int shmid = shmget(SHM_KEY, sizeof(GameData), 0666);
int msgid = msgget(MSG_KEY, 0666);

if (shmid < 0 || msgid < 0) {
printf ("Orion are you there?\n");
return 1;
}

GameData *data = (GameData *) shmat(shmid, NULL, 0);
sem_t *gate = sem_open(SEM_NAME, 0);

main_menu(data, gate, msgid);

shmdt(data);
sem_close(gate);
return 0;
}
