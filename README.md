# SISOP-3-2026-IT-105

## Soal 1
### 1. Pengerjaan file ```protocol.h``` (header)
File ini sebagai penyedia struktur data dan fungsi-fungsi global yang digunakan secara bersamaan oleh client (navi.c) dan server (wired.c) agar komunikasi keduanya sinkron. 

```c
#define MAX_BUF 1024
#define MAX_NAME 50
```
Batasan ukuran buffer pesan dan nama user.

```c
typedef struct {
    char sender [MAX_NAME];
    char content [MAX_BUF];
    int type; // login, chat, command, exit.
} Message;
```
Struktur data untuk bertukar informasi antara client dan server. 

```c
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
```
Fungsi untuk membaca pengaturan jaringan dari file ```wired.conf```. Program membuka file tersebut, menggunakan fungsi ```fscanf```. ```%[^:]:%d``` mengambil teks sebelum tanda titik dua sebagai alamat IP dan angka setelahnya sebagai Port. Jika file tidak ditemukan atau formatnya salah, fungsi secara otomatis memberi nilai default ```127.0.0.1``` dengan Port ```8080```.

```c
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
```
Fungsi untuk menangani pencatatan aktivitas ke dalam file ```history.log``` dengan mode append (```a```) sehingga data baru tidak menghapus data lama. Fungsi ini mengambil waktu sistem saat ini (```time(NULL)```), mengubahnya ke format yang bisa dibaca (```strftime```), lalu memcatat bersama dengan label (```tag```) dan isi kejadian ke dalam file log. 

### 2. Pengerjaan ```wired.c``` (server)
File ini sebagai pusat jaringan yang mengelola koneksi dari banyak pengguna sekaligus. 

```c
void broadcast (Message msg, int sender_fd) {
    pthread_mutex_lock (&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->socket != sender_fd) {
            send (clients[i]->socket, &msg, sizeof(msg), 0);
        }
    }
    pthread_mutex_unlock (&clients_mutex);
}
```
Fungsi ini untuk mngatur fitur chat grup. Server akan mengunci data client menggunakan mutex agar aman dari gangguan thread lain, lalu melakukan perulangan untuk mengirimkan pesan (send) kepada setiap pengguna yang sedang terhubung, kecuali kepada orang yang mengirim pesan tersebut agar pesannya tidak muncul dua kali di layar si pengirim.

```c
void *handle_client (void *arg) {
    int client_fd = *((int *)arg);
    free(arg);
    Message msg;
    char log_buf[200];
    char name [MAX_NAME];
    int index = -1;
}
```

Fungsi ini rutin dijalankan oleh setiap thread baru untuk menangani satu client secara spesifik.

```c
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
```
- Dalam fungsi ```handle_client``` terdapat logika login. Saat client pertama kali terhubung, server menunggu nama pengguna (```recv```). Jika namanya bukan admin (The Knights), server akan mencatat slot kosong di array ```clients```. 
- ```mutex``` melindungi proses pencarian slot agar tidak ada 2 thread yang mengisi slot yang sama secara bersamaan. Setelah tersimpan. server mencatat kejadian di file log. 
- ```else``` ika ```strcmp(name, "The Knights")``` bernilai 0. Server mencatat jika yang terhubung adalah Admin. 
- Server memanggil fungsi pencatat (```log_event```) untuk menyimpan informasi login ke file log dengan label ```[System]```. Lalu server mencetak notifikasi tersebut ke layar terminal. 

```c
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
```
- Server menggunakan ```recv``` dalam perulangan ```while``` untuk terus menerima data selama koneksi client tidak terputus. Jika nilai kembalian >0, artinya ada data baru yang masuk. 
- ```if (msg.type == 2)```: Server cek apakah pesan bertipe command (admin). Jika benar, server masuk ke logika pemrosesan perintah internal:
    - case 1: server menghitung berapa banyak slot yang tidak kosong untuk mengetahui jumlah user aktif saat ini.
    - case 2: server menghitung selisih waktu sekarang dengan waktu menyala melalui fungsi ```difftime``` untuk mendapatkan durasi uptime. 
    - case 3: server mencatat kejadian darurat ke log, mencetak peringatan di terminal, lalu mematikan seluruh program (```exit(0)```).
    - ```send```: hasil perintah admin dikirimkan kembali hanya pada admin yang meminta.
    - ```continue```: kembali ke awal ```while``` tanpa menjalankan kode broadcast di bawahnya (pesan admin bersifat rahasia).
