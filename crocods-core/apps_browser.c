#include "plateform.h"

#if defined (_WIN32) || defined (_3DS) || defined (__circle__)

void apps_browser_init(core_crocods_t *core, int flag)
{
}

#else

#include <ctype.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "autotype.h"

#include "apps_keyboard.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define CLOCKS_PER_SEC 1000000

void DispBrowser(core_crocods_t *core, u16 keys_pressed0);

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

int apps_browser_files_infilter = 0;

int apps_browser_files_count = 0;
int apps_browser_files_filtered_count = 0;
int apps_browser_files_begin = 0;
int apps_browser_files_selected = 0;

int apps_browser_files_flag = 0;

int apps_browser_pos = 0;
int apps_browser_speed_about = 0;
int apps_browser_dir_about = 1;

char *apps_browser_buf;
int apps_browser_buf_length;

char apps_browser_filter[32];

typedef struct {
    char *media_id;
    char *title;
} apps_browser_entry;

apps_browser_entry *apps_browser_files;
apps_browser_entry *apps_browser_filtered_files;

void apps_browser_end(core_crocods_t *core)
{
    free(apps_browser_buf);

    core->inKeyboard = 0;

    core->runApplication = NULL;

    core->wait_key_released = 1;

    free(apps_browser_files);
    free(apps_browser_filtered_files);

    ExecuteMenu(core, ID_MENU_EXIT, NULL);
}

// BDDBrowser/2.9.5d

// Need to free after
char * apps_browser_get_url(core_crocods_t *core, char *url, char *hostname, int *len)
{
    struct sockaddr_in servaddr;
    int sock;
    char *buf = (char *)malloc(256);

    *len = 0;

//    uint32_t t = guestGetMilliSeconds();

    memset(&servaddr, 0, sizeof(servaddr));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        ddlog(core, 2, "Wifi connect: Socket error !");
        ddlog(core, 2, "socket error\n");
        return NULL;
    }

    struct hostent *hostent;

    hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        ddlog(core, 2, "error: gethostbyname(\"%s\")\n", hostname);
        return NULL;
    }

    memcpy(&servaddr.sin_addr, hostent->h_addr_list[0], hostent->h_length);

//    in_addr_t in_addr;
//    in_addr = inet_addr(inet_ntoa(*(struct in_addr *)*(hostent->h_addr_list)));
//    servaddr.sin_addr.s_addr = in_addr;

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);

    ddlog(core, 2, "Wifi contact server ...");
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
        ddlog(core, 2, "Try to connect ...!\n");
//        fcntl(sock, F_SETFL, O_NONBLOCK);

//            int i = 1;
//            ioctl(sock, FIONBIO, &i);
        ddlog(core, 2, "Connected successfully!\n");
    } else {
        ddlog(core, 2, "Connected not done ...\n");
        return NULL;
    }

    char szBuff[16384];
    ssize_t recvd_len;

    // Grab the image
    // 1) grad image URL
    ddlog(core, 2, "Wifi get image ...");
//    sprintf(szBuff, "GET %s\r\nHost: %s\r\nUser-Agent: BDDBrowser/2.9.7c Java/1.8.0_192\r\nAccept: */*\r\n\r\n", url, hostname);
//    ddlog(core, 2, "Send %s\n", szBuff);

    sprintf(szBuff, "GET %s HTTP/1.0\r\nUser-Agent: BDDBrowser/2.9.7c Java/1.8.0_192\r\nHost: %s\r\nAccept: * /*\r\n\r\n", url, hostname);
//
//    ddlog(core, 2, "Send %s\n", szBuff);

    send(sock, szBuff, strlen(szBuff), 0);

    while ( (recvd_len = recv(sock, szBuff, 16384, 0) ) != 0) {
        ddlog(core, 2, "Len: %zd\n", recvd_len);

        if (recvd_len > 0) {
//            ddlog(core, 2, "fun() took %f seconds to execute \n", ((double)(guestGetMilliSeconds() - t)) / CLOCKS_PER_SEC);

//            ddlog(core, 2, "%02X%02X%02X%02X\n", szBuff[recvd_len - 4], szBuff[recvd_len - 3], szBuff[recvd_len - 2], szBuff[recvd_len - 1]);

            buf = realloc(buf, (*len) + recvd_len + 1);
            memcpy(buf + (*len), szBuff, recvd_len);
            (*len) += recvd_len;
        } else {
            perror("recv");
        }
    }
    buf[(*len)] = 0;

//    ddlog(core, 2, "fun() took %f seconds to execute \n", ((double)(guestGetMilliSeconds() - t)) / CLOCKS_PER_SEC);

    return buf;
} /* apps_browser_get_url */

#define BASE_URL "crocods.org"

char * xml_extract(char *xml, char *from, char *to, char *stop, char **result)
{
    char *beg = strstr(xml, from);

    if (stop != NULL) {
        char *stopstr = strstr(xml, stop);
        if (stopstr < beg) {
            return NULL;
        }
    }
    xml = beg;

    if (xml != NULL) {
        xml += strlen(from);
        char *endstr = strstr(xml, to);

        if (endstr != NULL) {
            *endstr = 0;
            *result = xml;
            xml = endstr + 1;
        } else {
            xml = NULL;
        }
    }
    return xml;
} /* xml_extract */

void apps_browser_use(core_crocods_t *core, int id)
{
    apps_browser_entry *entry;

    entry = &apps_browser_files[id];

    if (apps_browser_files_infilter != 0) {
        entry = &apps_browser_filtered_files[id];
    }

#ifdef EMSCRIPTEN
    if (1 == 1) {
        char url[512];
        char filename[MAX_PATH + 1];

        strcpy(filename, entry->title);
        strcat(filename, ".dsk");

        strcpy(url, "browse.php?action=get&id=");
        strcat(url, entry->media_id);

        EM_ASM({
            var url = UTF8ToString($0);
            var filename = UTF8ToString($1);

            loadURL(url, filename);
        }, url, filename);

        return;
    }
#endif /* ifdef EMSCRIPTEN */

    if (/* DISABLES CODE */ (1) == 0) {
        // Download resource
        int len;

        char url[512];

        //        strcpy(url,"/web/browse.php");
        strcpy(url, "/~miguelvanhove/crocods/web/browse.php");

        strcat(url, entry->media_id);

        char *res = apps_browser_get_url(core, url, BASE_URL, &len);
        char *orig = res;

        res = strstr(res, "\r\n\r\n");
        if (res != NULL) {
            res += 4;
            res = strstr(res, "\r\n");
            if (res != NULL) {
                res += 2;

                apps_browser_end(core);
                UseResources(core, res, len - (int)(res - orig));

                ExecuteMenu(core, ID_AUTORUN, NULL);

                return;
            }
        }
    }

    if (1 == 1) {
        // Download resource
        int len;
        char url[512];

//        uint32_t t = guestGetMilliSeconds();

        strcpy(url, "/web/browse.php?action=get&id=");
//        strcpy(url, "/~miguelvanhove/crocods/web/browse.php?action=get&id=");

        strcat(url, entry->media_id);

        printf("Download %s\n", url);

        char *res = apps_browser_get_url(core, url, BASE_URL, &len);  //

        char *orig = res;

        res = strstr(res, "\r\n\r\n");  // Skip header
        if (res != NULL) {
            res += 4;

            strcpy(core->filename, entry->title);
            strcat(core->filename, ".dsk");

            apps_browser_end(core);
            UseResources(core, res, len - (int)(res - orig));

//            ddlog(core, 2, "fun() took %f seconds to load resources \n", ((double)(guestGetMilliSeconds() - t)) / CLOCKS_PER_SEC);

            ExecuteMenu(core, ID_AUTORUN, NULL);

            return;
        }
    }
} /* apps_browser_use */

void apps_browser_usebuffer(core_crocods_t *core, const uint8_t *ptr, int size)
{
    if ((ptr != NULL) && (size != 0)) {
        apps_browser_buf_length = size;
        apps_browser_buf = malloc(apps_browser_buf_length);
        memcpy(apps_browser_buf, ptr, apps_browser_buf_length);
    }

    int i = 0;
    char ok = 0;

    while (!ok) {
        if (i + 5 >= apps_browser_buf_length) {
            break;
        }
        if ((apps_browser_buf[i] == 'K') && (apps_browser_buf[i + 1] == 'Y') && (apps_browser_buf[i + 2] == 'C') && (apps_browser_buf[i + 3] == 'P') && (apps_browser_buf[i + 4] == 'C')) {
            ok = 1;
            break;
        }
        i++;
    }

    i += 5;

    ddlog(core, 2, "%c", apps_browser_buf[0]);

    u16 count = apps_browser_buf[i + 1] * 256 + apps_browser_buf[i];

    i += 2;

    apps_browser_files = malloc(sizeof(apps_browser_entry) * count);
    apps_browser_filtered_files = malloc(sizeof(apps_browser_entry) * count);

    while (count > 0) {
        apps_browser_files_count++;

        char *value;

        value = apps_browser_buf + i;
        apps_browser_files[apps_browser_files_count - 1].title = value;
        i += strlen(value) + 1;

        value = apps_browser_buf + i;
        apps_browser_files[apps_browser_files_count - 1].media_id = value;
        i += strlen(value) + 1;

        value = apps_browser_buf + i;
//            apps_browser_files[apps_browser_files_count - 1].media_id = value;
        i += strlen(value) + 1;

        count--;
    }

    ddlog(core, 2, "count: %d", count);
} /* apps_browser_usebuffer */

void apps_browser_init0(core_crocods_t *core, int flag)
{
    core->runApplication = DispBrowser;
    apps_browser_files_flag = flag;

    apps_browser_files_count = 0;
    apps_browser_files_begin = 0;
    apps_browser_files_selected = 0;

    apps_browser_files = NULL;
    apps_browser_files_count = 0;

    apps_browser_filter[0] = 0;
    apps_browser_files_filtered_count = 0;

    apps_browser_files_infilter = 0;
}

void apps_browser_init(core_crocods_t *core, int flag)
{
#ifdef EMSCRIPTEN
    EM_ASM({
        var oReq = new XMLHttpRequest();
        oReq.open("GET", "browse.php", true);
        oReq.responseType = "arraybuffer";

        oReq.onload = function(oEvent) {
            var arrayBuffer = oReq.response;

            var uint8Array = new Uint8Array(arrayBuffer);
            let res = Module['ccall']('emsc_load_browsebuffer',
                                      'int',
                                      ['array', 'number'], // name, data, size
                                      [uint8Array, uint8Array.length]);
            if (res == 0) {
                console.warn('emsc_load_browsebuffer() failed!');
            }
        };

        oReq.send();
    });

    return;     // Don't load from tcp socket
#endif /* ifdef EMSCRIPTEN */

    ddlog(core, 2, "apps_browser_init: %s\n", core->openFilename);

    apps_browser_init0(core, flag);

//    uint32_t t = guestGetMilliSeconds();

    if (1 == 1) {
        char *url = "/web/browse.php";
//        char *url = "/~miguelvanhove/crocods/web/browse.php";

        apps_browser_buf = apps_browser_get_url(core, url, BASE_URL, &apps_browser_buf_length);  // crocods.org
    }

    apps_browser_usebuffer(core, NULL, 0);

//    ddlog(core, 2, "fun() took %f seconds to use buffer \n", ((double)(guestGetMilliSeconds() - t)) / CLOCKS_PER_SEC);

    if (/* DISABLES CODE */ (1) == 0) {
        char *url = "/games/api.php?action=detailist";

        int len;
        apps_browser_buf = apps_browser_get_url(core, url, "cpc.devilmarkus.de", &len);
        if (apps_browser_buf != NULL) {
            char *xml = apps_browser_buf;
            while (xml != NULL) {
                char *id, *title;

                xml =  xml_extract(xml, "<game id=\"", "\"", NULL, &id);
                if (xml == NULL) {
                    break;
                }
                ddlog(core, 2, "id: %s\n", id);
                xml =  xml_extract(xml, "title=\"", "\"", NULL, &title);
                ddlog(core, 2, "title: %s\n", title);
                while (1) {
                    char *media;
                    char *media_id, *media_type;
                    media = xml_extract(xml, "<media id=\"", "\"", "</game>", &media_id);
                    if (media == NULL) {
                        break;
                    }
                    xml = xml_extract(media, "type=\"", "\"", NULL, &media_type);
                    ddlog(core, 2, "media: %s - %s\n", media_type, media_id);

                    if (!strcmp(media_type, "Disquette")) {
                        // Create new entry

                        apps_browser_files_count++;
                        apps_browser_files = realloc(apps_browser_files, sizeof(apps_browser_entry) * apps_browser_files_count);

                        apps_browser_files[apps_browser_files_count - 1].title = title;
                        apps_browser_files[apps_browser_files_count - 1].media_id = media_id;
                    }
                }
            }
        }
    }

//    if (error) {
//        appendIcon(core, 0, 4, 60);
//        apps_browser_end(core);
//        return;
//    }
}         /* apps_browser_init */

void apps_browser_updatefilter(core_crocods_t *core)
{
    apps_browser_files_filtered_count = 0;

    int i;

    for (i = 0; i < apps_browser_files_count; i++) {
        int j = 0;
        int n = 0;
        char isOk = 0;
        char waitSpace = 0;

        while (1) {
            if (waitSpace) {
                if (apps_browser_files[i].title[j] == 32) {
                    waitSpace = 0;
                }
            } else {
                if ((char)tolower(apps_browser_filter[n]) == (char)tolower(apps_browser_files[i].title[j])) {
                    n++;
                    if (apps_browser_filter[n] == 0) {
                        isOk = 1;
                        break;
                    }
                } else {
                    if ((n != 0) &&  (apps_browser_files[i].title[j] != 32)) {
                        waitSpace = 1;
                    }
                }
            }

            j++;

            if (apps_browser_files[i].title[j] == 0) {
                break;
            }
        }

        if (isOk) {
            apps_browser_filtered_files[apps_browser_files_filtered_count] = apps_browser_files[i];
            apps_browser_files_filtered_count++;
        }
    }
} /* apps_browser_updatefilter */

void apps_browser_filter_print(core_crocods_t *core, int y, char *text, char selected)
{
    // int n;

    u16 back, front;

    front = RGB565(0xFF, 0xFF, 0x00);

    if (selected) {
        back = RGB565(0x00, 0x00, 0xFF);
    } else {
        back = RGB565(0x00, 0x00, 0x00);
    }


    int j = 0;
    int n = 0;
    char isOk = 0;
    char waitSpace = 0;

    while (1) {
        char car[2];

        u8 highlight = 0;

        if (!isOk) {
            if (waitSpace) {
                if (text[j] == 32) {
                    waitSpace = 0;
                }
            } else {
                if ((char)tolower(apps_browser_filter[n]) == (char)tolower(text[j])) {
                    highlight = 1;
                    n++;
                    if (apps_browser_filter[n] == 0) {
                        isOk = 1;
                    }
                } else {
                    if ((n != 0) &&  (text[j] != 32)) {
                        waitSpace = 1;
                    }
                }
            }
        }

        car[0] = text[j];
        car[1] = 0;

        cpcprint16(core, core->overlayBitmap, 320, 12 + j * 8, 36 + y * 8, car, highlight ? RGB565(0xFF, 0x00, 0x00) : front, back, 1, 0);

        j++;

        if (text[j] == 0) {
            break;
        }
    }


    // for (n = 0; n < strlen(text); n++) {
    //     car[0] = text[n];
    //     car[1] = 0;

    //     cpcprint16(core, core->overlayBitmap, 320, 12 + n * 8, 36 + y * 8, car, front, back, 1, 0);
    // }


} /* apps_browser_filter_print */

void DispBrowser(core_crocods_t *core, u16 keys_pressed0)
{
//    static int key = 0;

    int y;

    u16 keys_pressed = appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 168;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap;         // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    // Text begin in 12,36 (max 13 lines, 26 columns)

    for (y = 0; y < 168; y++) {
        memcpy(pdwAddr, core->select + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    char title[33];
    sprintf(title, "%-32s", apps_browser_filter);
    cpcprint16(core, core->overlayBitmap, 320, (256 - (int)strlen(title) * 8) / 2, 15, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    if (apps_browser_files_begin != 0) {
        dispIcon8(core, 235, 36 + 0 * 8, 8);
    }

    if (apps_browser_files_filtered_count == 0) {
        if (apps_browser_files_count - apps_browser_files_begin > 13) {
            dispIcon8(core, 235, 36 + 12 * 8, 9);
        }
    } else {
        if (apps_browser_files_filtered_count - apps_browser_files_begin > 13) {
            dispIcon8(core, 235, 36 + 12 * 8, 9);
        }
    }

    //                                                     1234567890ABCDEFGHIJKJIHGFEDCBA0987654321
    cpcprint16_6w(core, core->overlayBitmap, 320, 4, 156, "  Use arrows to navigate & Start to RUN  ", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    for (y = 0; y < 13; y++) {
        int n = y + apps_browser_files_begin;

        if (apps_browser_files_infilter == 0) {
            if (n < apps_browser_files_count) {
                char text[30 + 1];

                snprintf(text, 31, "%s", apps_browser_files[n].title);

                if (n == apps_browser_files_selected) {
                    cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0xFF), 1, 0);
                } else {
                    cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
                }
            }
        } else {
            if (n < apps_browser_files_filtered_count) {
                char text[30 + 1];

                snprintf(text, 31, "%s", apps_browser_filtered_files[n].title);

                apps_browser_filter_print(core, y, text, (n == apps_browser_files_selected) );


            }
        }
    }

    if (core->ipc.wheelY != 0) {
        apps_browser_files_selected += core->ipc.wheelY;
        if (apps_browser_files_selected < 0) {
            apps_browser_files_selected = 0;
        }

        if (apps_browser_files_filtered_count == 0) {
            if (apps_browser_files_selected >=  apps_browser_files_count) {
                apps_browser_files_selected = apps_browser_files_count - 1;
            }
        } else {
            if (apps_browser_files_selected >= apps_browser_files_filtered_count) {
                apps_browser_files_selected = apps_browser_files_filtered_count - 1;
            }
        }

        if (apps_browser_files_selected >  apps_browser_files_begin + 12) {
            apps_browser_files_begin =  apps_browser_files_selected - 12;
        }
        if (apps_browser_files_selected <  apps_browser_files_begin) {
            apps_browser_files_begin =  apps_browser_files_selected;
        }

        core->ipc.wheelY = 0;
    }

    if (core->ipc.touchDown == 1) {
        core->ipc.touchDown = 0;

        int x, y;       // , n;

        x = core->ipc.touchXpx - core->overlayBitmap_posx - 9;
        y = core->ipc.touchYpx - core->overlayBitmap_posy - 36;


        if ((x >= 0) && (x < 226)) {
            apps_browser_files_selected = y / 8 + apps_browser_files_begin;
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
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        apps_browser_use(core, apps_browser_files_selected);
    }

    if ((keys_pressed & KEY_B) == KEY_B) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    int count = apps_browser_files_count;

    if (apps_browser_files_filtered_count != 0) {
        count = apps_browser_files_filtered_count;
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
        apps_browser_files_selected--;
        if (apps_browser_files_selected < 0) {
            apps_browser_files_selected = count - 1;
            apps_browser_files_begin = count - 13;
            if (apps_browser_files_begin < 0) {
                apps_browser_files_begin = 0;
            }
        }
        if (apps_browser_files_selected < apps_browser_files_begin) {
            apps_browser_files_begin--;
        }
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        apps_browser_files_selected++;
        if (apps_browser_files_selected >= count) {
            apps_browser_files_selected = 0;
            apps_browser_files_begin = 0;
        }
        if (apps_browser_files_selected > apps_browser_files_begin + 12) {
            apps_browser_files_begin++;
        }
    }

    if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
        apps_browser_files_selected -= 10;
        if (apps_browser_files_selected < 0) {
            apps_browser_files_selected = 0;
            apps_browser_files_begin =  count - 13;
            if (apps_browser_files_begin < 0) {
                apps_browser_files_begin = 0;
            }
        }
        if (apps_browser_files_selected <  apps_browser_files_begin) {
            apps_browser_files_begin =  apps_browser_files_selected;
        }
    }

    if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
        apps_browser_files_selected += 10;
        if (apps_browser_files_selected >=  count) {
            apps_browser_files_selected =  count - 1;
        }
        if (apps_browser_files_selected >  apps_browser_files_begin + 12) {
            apps_browser_files_begin =  apps_browser_files_selected - 12;
        }
    }

    if (apps_browser_files_filtered_count != 0) {
        apps_browser_files_filtered_count = count;
    } else {
        apps_browser_files_count = count;
    }

    if (keys_pressed & KEY_SELECT) {
        apps_browser_end(core);
    }

    static u8 bit_values[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
    };

    int cpc_scancode = 0;

    for (cpc_scancode = 0; cpc_scancode < CPC_NIL; cpc_scancode++) {
        if (!(core->app_clav[(u8)cpc_scancode >> 3] & bit_values[(u8)cpc_scancode & 7])) {
            char ascii = asciiFromCPC(core, cpc_scancode);
            // printf("%d\n", ascii);

            if (ascii == 127) {
                int len = (int)strlen(apps_browser_filter);
                if (len >= 1) {
                    len--;
                }
                apps_browser_filter[len] = 0;
                if (len != 0) {
                    apps_browser_updatefilter(core);
                } else {
                    apps_browser_files_filtered_count = 0;
                    apps_browser_files_infilter = 0;
                }

                apps_browser_files_selected = 0;
                apps_browser_files_begin = 0;
            } else if (ascii >= 32) {
                int len = (int)strlen(apps_browser_filter);
                apps_browser_filter[len] = ascii;
                apps_browser_filter[len + 1] = 0;

                apps_browser_updatefilter(core);

                apps_browser_files_selected = 0;
                apps_browser_files_begin = 0;

                apps_browser_files_infilter = 1;
            }

            ddlog(core, 2, "(Filter %s)\n", apps_browser_filter);
        }
    }


}         /* DispKeyboard */

#endif /* if defined (_WIN32) || defined (_3DS) */
