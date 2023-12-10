#include "plateform.h"



#if !defined (_WIN32)

#include <dirent.h>
#define DIRENT_INCLUDED
#endif

#ifndef DIRENT_INCLUDED
#define DIRENT_INCLUDED

// Implementation of POSIX directory browsing functions and types for Win32.

// Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
// History: Created March 1997. Updated June 2003 and July 2012.

#ifndef S_IFMT
#define S_IFMT  0170000                        /* [XSI] type of file mask */
#endif

#ifndef S_IFDIR
#define S_IFDIR 0040000                        /* [XSI] directory */
#endif

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR) /* directory */

typedef struct DIR DIR;

struct dirent {
    char *d_name;
};

DIR * opendir(const char *);
int           closedir(DIR *);
struct dirent * readdir(DIR *);
void          rewinddir(DIR *);


#include <errno.h>
#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
#include <stdlib.h>
#include <string.h>

typedef ptrdiff_t handle_type; /* C99's intptr_t not sufficiently portable */

struct DIR {
    handle_type handle;         /* -1 for failed rewind */
    struct _finddata_t info;
    struct dirent result;       /* d_name null iff first time */
    char *name;                 /* null-terminated char string */
};

DIR * opendir(const char *name)
{
    DIR *dir = 0;

    if (name && name[0]) {
        size_t base_length = strlen(name);
        const char *all = /* search pattern must end with suitable wildcard */
            strchr("/\\", name[base_length - 1]) ? "*" : "/*";

        if ((dir = (DIR *)malloc(sizeof *dir)) != 0 &&
            (dir->name = (char *)malloc(base_length + strlen(all) + 1)) != 0) {
            strcat(strcpy(dir->name, name), all);

            if ((dir->handle =
                     (handle_type)_findfirst(dir->name, &dir->info)) != -1) {
                dir->result.d_name = 0;
            } else { /* rollback */
                free(dir->name);
                free(dir);
                dir = 0;
            }
        } else { /* rollback */
            free(dir);
            dir = 0;
            errno = ENOMEM;
        }
    } else {
        errno = EINVAL;
    }

    return dir;
} /* opendir */

int closedir(DIR *dir)
{
    int result = -1;

    if (dir) {
        if (dir->handle != -1) {
            result = _findclose(dir->handle);
        }

        free(dir->name);
        free(dir);
    }

    if (result == -1) { /* map all errors to EBADF */
        errno = EBADF;
    }

    return result;
}

struct dirent * readdir(DIR *dir)
{
    struct dirent *result = 0;

    if (dir && dir->handle != -1) {
        if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
            result = &dir->result;
            result->d_name = dir->info.name;
        }
    } else {
        errno = EBADF;
    }

    return result;
}

void rewinddir(DIR *dir)
{
    if (dir && dir->handle != -1) {
        _findclose(dir->handle);
        dir->handle = (handle_type)_findfirst(dir->name, &dir->info);
        dir->result.d_name = 0;
    } else {
        errno = EBADF;
    }
}

#endif /* ifndef DIRENT_INCLUDED */



void DispAppsDisk(core_crocods_t *core, u16 keys_pressed0);
void apps_disk_readdir(core_crocods_t *core);

void apps_disk_tpath2Abs(char *p, char *Ficname);
void apps_disk_path2Abs(char *p, const char *relatif);

#define     USER_DELETED 0xE5

#ifdef _WIN32
#define DEFSLASH         '\\'
#else
#define DEFSLASH         '/'
#endif

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

typedef struct {
    char *filename;
    unsigned char folder;
} DirEntryCroco;

AppsListCroco apps_disk_files;
DirEntryCroco *apps_disk_files_entry;

// DirEntryCroco *apps_disk_files;
// int apps_disk_files.count = 0;
// int apps_disk_files.begin = 0;
// int apps_disk_files.selected = 0;
// int apps_disk_mouseSelected;

int apps_disk_files_flag = 0;

void apps_disk_end(core_crocods_t *core)
{
    core->runApplication = NULL;

    core->wait_key_released = 1;
}

void apps_disk_init(core_crocods_t *core, int flag)
{
    apps_disk_files.mouseSelected = -1;
    apps_disk_files_flag = flag;
    core->runApplication = DispAppsDisk;

    apps_disk_files_entry = NULL;

    apps_disk_readdir(core);
}

static int apps_disk_compare(void const *a, void const *b)
{
    DirEntryCroco *ad, *bd;

    ad = (DirEntryCroco *)a;
    bd = (DirEntryCroco *)b;

    if (ad->folder == bd->folder) {
        return strcasecmp(ad->filename, bd->filename);
    }

    if (ad->folder > bd->folder) {
        return -1;
    }
    return 1;
}

void apps_disk_readdir(core_crocods_t *core)
{
    apps_disk_files.count = 0;
    apps_disk_files.begin = 0;
    apps_disk_files.selected = 0;

    ddlog(core, 2, "Open dir %s\n", core->file_dir);

    DIR *d;
    struct dirent *dir;

    d = opendir(core->file_dir);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
//            ddlog(core, 2, "%s\n", dir->d_name);

            char filename[256];

            strcpy(filename, dir->d_name);

            char *ext = strrchr(filename, '.');
            if (ext != NULL) {
                ext++;
            }

            char ok = 0;
            char folder = 0;

            if ((ext != NULL) &&
                ((!strcasecmp(ext, "sna")) ||
                 (!strcasecmp(ext, "dsk")) ||
                 (!strcasecmp(ext, "bas")) ||
                 (!strcasecmp(ext, "kcr")) ||
                 (!strcasecmp(ext, "cpr")) ||
                 (!strcasecmp(ext, "rom")) ||
                 (!strcasecmp(ext, "zip")))) {
                ok = 1;
            }

            if (!ok) {
                char directory[2048];
                struct stat s;

                strcpy(directory, core->file_dir);
                apps_disk_path2Abs(directory, dir->d_name);

                stat(directory, &s);

                if (S_ISDIR(s.st_mode)) {
                    if ((filename[0] != '.') || (!strcmp(filename, ".."))) {
                        ok = 1;
                        folder = 1;
                    }
                }
            }

            if (ok) {
                apps_disk_files_entry = (DirEntryCroco *)realloc(apps_disk_files_entry, sizeof(DirEntryCroco) * (apps_disk_files.count + 1));

                apps_disk_files_entry[apps_disk_files.count].filename = malloc(strlen(filename) + 1);
                apps_disk_files_entry[apps_disk_files.count].folder = folder;

                strcpy(apps_disk_files_entry[apps_disk_files.count].filename, filename);
                apps_disk_files.count++;
            }
        }
        closedir(d);

        qsort(apps_disk_files_entry, apps_disk_files.count, sizeof(DirEntryCroco), apps_disk_compare);

        int n;
        for (n = 0; n < apps_disk_files.count; n++) {
            if (!strcasecmp(apps_disk_files_entry[n].filename, core->filename)) {
                apps_disk_files.selected = n;
                if (apps_disk_files.selected > apps_disk_files.begin + 12) {
                    apps_disk_files.begin = apps_disk_files.selected - 12;
                }
            }
        }
    } else {
//        ddlog(core, 2, "Error failed to open input directory -%s\n",strerror(errno) );
    }

    if (apps_disk_files.count == 0) {
        char directory[2048];
        strcpy(directory, core->file_dir);
        apps_disk_path2Abs(directory, "..");

        core->file_dir = (char *)realloc(core->file_dir, strlen(directory) + 1);
        strcpy(core->file_dir, directory);

        apps_disk_readdir(core);
    }


} /* apps_disk_readdir */

void DispAppsDisk(core_crocods_t *core, u16 keys_pressed0)
{
//    static int key = 0;

    int y;

    u16 keys_pressed = appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 168;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap; // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    // Text begin in 12,36 (max 13 lines, 30 columns)

    for (y = 0; y < 168; y++) {
        memcpy(pdwAddr, core->select + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    char *title = "Open a disk or a snapshot";

    cpcprint16(core, core->overlayBitmap, 320, (256 - (int)strlen(title) * 8) / 2, 15, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    if (apps_disk_files.begin != 0) {
        dispIcon8(core, 235, 36 + 0 * 8, 8);
    }
    if (apps_disk_files.count - apps_disk_files.begin > 13) {
        dispIcon8(core, 235, 36 + 12 * 8, 9);
    }

    //                                                     1234567890ABCDEFGHIJKJIHGFEDCBA0987654321
    cpcprint16_6w(core, core->overlayBitmap, 320, 4, 156, "      Arrows & Start to insert disk      ", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    for (y = 0; y < 13; y++) {
        int n = y + apps_disk_files.begin;

        if ((n < apps_disk_files.count) && (n >= 0)) {
            char text[27 + 1];
            char filename[2048];

            strcpy(filename, apps_disk_files_entry[n].filename);

            char *ext;

            if (!strcmp(filename, "..")) {
                strcpy(filename, "[UP]");
                ext = NULL;
            } else {
                ext = strrchr(filename, '.');
                if (ext != NULL) {
                    *ext = 0;
                    ext++;
                }
            }

            if (apps_disk_files_entry[n].folder == 1) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 2);
            } else if ((ext != NULL) && (!strcasecmp(ext, "sna"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 1);
            } else if ((ext != NULL) && (!strcasecmp(ext, "dsk"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 0);
            } else if ((ext != NULL) && (!strcasecmp(ext, "zip"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 4);
            } else if ((ext != NULL) && (!strcasecmp(ext, "bas"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 5);
            } else if ((ext != NULL) && (!strcasecmp(ext, "kcr"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 6);
            } else if ((ext != NULL) && (!strcasecmp(ext, "cpr"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 7);
            } else if ((ext != NULL) && (!strcasecmp(ext, "rom"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 10);
            }

            strncpy(text, filename, 27);
            text[27] = 0;

            if (n == apps_disk_files.selected) {
                cpcprint16(core, core->overlayBitmap, 320, 12 + 2 * 8, 36 + y * 8, text, RGB565(0x00, 0xFF, 0xFF), RGB565(0x00, 0x00, 0xFF), 1, 0);
            } else {
                cpcprint16(core, core->overlayBitmap, 320, 12 + 2 * 8, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
            }
        }
    }

    if (core->ipc.wheelY != 0) {
        apps_disk_files.selected += core->ipc.wheelY;
        if (apps_disk_files.selected < 0) {
            apps_disk_files.selected = 0;
        }

        if (apps_disk_files.selected >=  apps_disk_files.count) {
            apps_disk_files.selected = apps_disk_files.count - 1;
        }

        if (apps_disk_files.selected >  apps_disk_files.begin + 12) {
            apps_disk_files.begin =  apps_disk_files.selected - 12;
        }
        if (apps_disk_files.selected <  apps_disk_files.begin) {
            apps_disk_files.begin =  apps_disk_files.selected;
        }

        core->ipc.wheelY = 0;
    }

    if (core->ipc.touchDown == 1) {
        core->ipc.touchDown = 0;

        int x, y;       // , n;

        x = core->ipc.touchXpx - core->overlayBitmap_posx - 9;
        y = core->ipc.touchYpx - core->overlayBitmap_posy - 36;

        if ((x >= 0) && (x < 226)) {
            apps_disk_files.selected = y / 8 + apps_disk_files.begin;
            keys_pressed = keys_pressed | KEY_START;
        } else if ((x >= 226) && (x < 245)) {
            if ((y >= 0) && (y < 8)) {
                keys_pressed = keys_pressed | KEY_LEFT;
            } else if ((y >= 95) && (y < 103)) {
                keys_pressed = keys_pressed | KEY_RIGHT;
            }
        } else {
            keys_pressed = keys_pressed | KEY_B;
        }
//        ddlog(core, 2, "after: %d,%d\n", x, y);
//        ddlog(core, 2, "select: %d\n", y / 8);
    }

    if (keys_pressed & KEY_START) {
        if (apps_disk_files_entry[apps_disk_files.selected].folder == 1) {
            char directory[2048];
            strcpy(directory, core->file_dir);
            apps_disk_path2Abs(directory, apps_disk_files_entry[apps_disk_files.selected].filename);

            core->file_dir = (char *)realloc(core->file_dir, strlen(directory) + 1);
            strcpy(core->file_dir, directory);

            apps_disk_readdir(core);

            return;
        }
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        strcpy(core->openFilename, core->file_dir);
        apps_disk_path2Abs(core->openFilename, apps_disk_files_entry[apps_disk_files.selected].filename);

        ExecuteMenu(core, ID_PAUSE_EXIT, NULL);

        if (apps_disk_files_flag == 1) {
            ExecuteMenu(core, ID_AUTORUN, NULL);
        } else {
            ExecuteMenu(core, ID_INSERTDISK, NULL);
        }
        return;
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
        apps_disk_files.selected--;
        if (apps_disk_files.selected < 0) {
            apps_disk_files.selected = apps_disk_files.count - 1;
            apps_disk_files.begin = apps_disk_files.count - 13;
            if (apps_disk_files.begin < 0) {
                apps_disk_files.begin = 0;
            }
        }
        if (apps_disk_files.selected < apps_disk_files.begin) {
            apps_disk_files.begin--;
        }
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        apps_disk_files.selected++;
        if (apps_disk_files.selected >= apps_disk_files.count) {
            apps_disk_files.selected = 0;
            apps_disk_files.begin = 0;
        }
        if (apps_disk_files.selected > apps_disk_files.begin + 12) {
            apps_disk_files.begin++;
        }

        ddlog(core, 2, "begin %d,%d\n", apps_disk_files.begin, apps_disk_files.count);
    }

    if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
        apps_disk_files.selected -= 10;
        if (apps_disk_files.selected < 0) {
            apps_disk_files.selected = 0;
            apps_disk_files.begin = apps_disk_files.count - 13;
            if (apps_disk_files.begin < 0) {
                apps_disk_files.begin = 0;
            }
        }
        if (apps_disk_files.selected < apps_disk_files.begin) {
            apps_disk_files.begin = apps_disk_files.selected;
        }
    }

    if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
        apps_disk_files.selected += 10;
        if (apps_disk_files.selected >= apps_disk_files.count) {
            apps_disk_files.selected = apps_disk_files.count - 1;
        }
        if (apps_disk_files.selected > apps_disk_files.begin + 12) {
            apps_disk_files.begin = apps_disk_files.selected - 12;
        }
    }
} /* DispKeyboard */

void apps_disk_path2Abs(char *p, const char *relatif)
{
    static char Ficname[256];
    int l, m, n;
    char car;

    // ddlog(core, 2, "Path2abs %s with %s\n", p, relatif);

    if (relatif[0] == 0) return;

    strcpy(Ficname, relatif);

    for (n = 0; n < strlen(p); n++) {
        if (p[n] == '/') p[n] = DEFSLASH;
    }

    for (n = 0; n < strlen(Ficname); n++) {
        if (Ficname[n] == '/') Ficname[n] = DEFSLASH;
    }

    m = 0;
    l = (int)strlen(Ficname);

    if ((Ficname[l - 1] == DEFSLASH) & (l != 1)) { // --- retire le dernier slash ---
        if (Ficname[l - 2] != ':') { // --- drive --------------------------------
            l--;
            Ficname[l] = 0;
        }
    }

    for (n = 0; n < l; n++) {
        if (Ficname[n] == DEFSLASH) {
            car = Ficname[n + 1];

            Ficname[n + 1] = 0;

            apps_disk_tpath2Abs(p, Ficname + m);

            Ficname[n + 1] = car;
            Ficname[n] = 0;

            m = n + 1;
        }
    }

    apps_disk_tpath2Abs(p, Ficname + m);

    if (p[0] == 0) {
        p[0] = DEFSLASH;
        p[1] = 0;
    }

    // ddlog(core, 2, "Path2abs result: %s\n", p);
} /* apps_disk_path2Abs */

void apps_disk_tpath2Abs(char *p, char *Ficname)
{
    int n;
    static char old[256]; // --- Path avant changement ------------------
    signed int deuxpoint;
    char defslash[2];

    defslash[0] = DEFSLASH;
    defslash[1] = 0;

    if (Ficname[0] == 0) return;

    memcpy(old, p, 256);

    if (p[strlen(p) - 1] == DEFSLASH) p[strlen(p) - 1] = 0;

    if ( (!strncmp(Ficname, "..", 2)) & (p[0] != 0) ) {
        for (n = (int)strlen(p); n > 0; n--) {
            if (p[n] == DEFSLASH) {
                p[n] = 0;
                break;
            }
        }
        if (p[strlen(p) - 1] == ':') strcat(p, defslash);
        return;
    }

    if ((Ficname[0] != '.') || (Ficname[1] != '.')) {
        deuxpoint = -1;

        for (n = 0; n < strlen(Ficname); n++) {
            if (Ficname[n] == ':') deuxpoint = n;
        }

        if (deuxpoint != -1) {
            if (Ficname[deuxpoint + 1] == DEFSLASH) {
                strcpy(p, Ficname);
                if (p[strlen(p) - 1] == ':') strcat(p, defslash);
                return;
            }
        }

        if (Ficname[0] == DEFSLASH) {
            if (p[1] == ':') {
                strcpy(p + 2, Ficname);
            } else {
                strcpy(p, Ficname);
            }
            if (p[strlen(p) - 1] == ':') strcat(p, defslash);
            return;
        }

        if (p[strlen(p) - 1] != DEFSLASH) strcat(p, defslash);
        strcat(p, Ficname);
    }

    if (p[strlen(p) - 1] == ':') strcat(p, defslash);
} /* apps_disk_tpath2Abs */

