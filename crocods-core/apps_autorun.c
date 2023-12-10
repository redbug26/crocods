#include "plateform.h"

#ifdef ZIP_SUPPORT
#include "miniz.h"
#endif

#include "3rdparty/rasm/rasm.h"

#include "idsk_lite.h"
#include "basic.h"

#include "os.h"

#include "z80_cap32.h"
extern t_z80regs z80; // in cap32

void DispAutorun(core_crocods_t *core, u16 keys_pressed0);
void LireDiskMem(core_crocods_t *core, u8 *rom, u32 romsize);

#define USER_DELETED 0xE5

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

#pragma pack(1)

typedef struct {
    unsigned char User;
    char Nom[ 8 ];
    char Ext[ 3 ];
    unsigned char NumPage;
    unsigned char Unused[ 2 ];
    unsigned char NbPages;
    unsigned char Blocks[ 16 ];
} StDirEntry;

#pragma pack()

typedef struct {
    unsigned char user;
    char name[ 8 + 1 ];
    char ext[ 3 + 1 ];
    int nbpages;
    char hidden, readonly, archived;
    u8 type;
    u16 exec, load;
    u16 size;
} StDirEntryCroco;

StDirEntryCroco apps_autorun_files[64];
int apps_autorun_files_count = 0;
int apps_autorun_files_begin = 0;
int apps_autorun_files_selected = 0;

int apps_autorun_files_flag = 0;

void apps_autorun_end(core_crocods_t *core)
{
    core->runApplication = NULL;

    core->wait_key_released = 1;
}



void apps_autorun_init(core_crocods_t *core, int flag)
{
    ddlog(core, 2, "apps_autorun_init: %s\n", core->openFilename);

    if ((core->openFilename[0] == 0) && (core->resources == NULL) && (core->dataFile == NULL)) {
        apps_autorun_end(core);
        return;
    }

    core->runApplication = DispAutorun;
    apps_autorun_files_flag = flag;

    apps_autorun_files_count = 0;
    apps_autorun_files_begin = 0;
    apps_autorun_files_selected = 0;

    u8 *snapshot = NULL;
    u32 snapshotLength;

    u8 *dsk;
    long dsk_size;

    if (core->dataFile != NULL) {
        ddlog(core, 2, "Open resources\n");

        dsk_size = core->dataFile_len;

        dsk = (u8 *)malloc(core->dataFile_len);
        memcpy(dsk, core->dataFile, core->dataFile_len);
    } else if (core->resources != NULL) {
        ddlog(core, 2, "Open resources\n");

        dsk_size = core->resources_len;

        dsk = (u8 *)malloc(core->resources_len);
        memcpy(dsk, core->resources, core->resources_len);
    } else {
        ddlog(core, 2, "open file\n");

        FILE *fic = os_fopen(core->openFilename, "rb");
//        mydebug(core,"Open file: %s\n", core->openFilename);

        if (fic == NULL) {
            ddlog(core, 0, "File not found: %s\n", core->openFilename);
            if (core->cliMode) {
                exit(EXIT_FAILURE);
            }
            appendIcon(core, 0, 4, 60);
            apps_autorun_end(core);
            return;
        }
        fseek(fic, 0, SEEK_END);
        dsk_size = ftell(fic);
        fseek(fic, 0, SEEK_SET);

        dsk = (u8 *)malloc(dsk_size);
        if (dsk == NULL) {
            appendIcon(core, 0, 4, 60);
            apps_autorun_end(core);
            return;
        }
        fread(dsk, 1, dsk_size, fic);
        fclose(fic);

        ddlog(core, 2, "Open filename: %s\n", core->openFilename);

        char *p = strrchr(core->openFilename, '/');

        ddlog(core, 2, "P: %s\n", p);

        strcpy(core->filename, p ? p + 1 : (char *)core->openFilename);

        // Copy basename of filename to current directory

        char directory[2048];
        strcpy(directory, core->openFilename);
        apps_disk_path2Abs(directory, "..");

        core->file_dir = (char *)realloc(core->file_dir, strlen(directory) + 1);
        strcpy(core->file_dir, directory);

//        mydebug(core,"New dir: %s\n", core->file_dir);
    }

    char *ext = strrchr(core->openFilename, '.');

    if (ext != NULL) {
        ext++;

        if (!strcasecmp(ext, "s")) {
            if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                ddlog(core, 0, "Detected file: Assembler (.s)\n\n");
            }

            SoftResetCPC(core);

            if (1 == 1) {
                struct s_rasm_info *debug;
                unsigned char *buf = NULL;
                int asmsize = 0;
                int res = RasmAssembleInfo((const char *)dsk, dsk_size, &buf, &asmsize, &debug);
                if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                    ddlog(core, 0, "Result=%d, Generated code size=%d\n", res, asmsize);
                    ddlog(core, 0, " Run: %x\n", debug->run);
                    ddlog(core, 0, " Start: %x\n", debug->start);

                    if (res != 0) {
                        ddlog(core, 0, "Failure\n");
                    }
                }

                int ramblock = debug->start / 16384;
                memcpy(core->ROMEXT[ramblock] + debug->start - (ramblock * 16384), buf, asmsize);
                memcpy(core->MemCPC + debug->start, buf, asmsize);

                if (core->cliGetInfo) {
                    ExecuteMenu(core, ID_EXIT, NULL);
                }


                z80.PC.w.l = debug->start;

                if (buf)
                    free(buf);

                if (debug)
                    free(debug);
            } else {
                struct s_rasm_info *debug;

                int res = RasmAssembleInfoIntoRAM((const char *)dsk, dsk_size, &debug, core->MemCPC, 65536 * 2);
                if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                    ddlog(core, 0, "Result=%d\n", res);
                    ddlog(core, 0, " Run: %x\n", debug->run);
                    ddlog(core, 0, " Start: %x\n", debug->start);

                    if (res != 0) {
                        ddlog(core, 0, "Failure\n");
                    }
                }

                z80.PC.w.l = debug->start;

                if (debug)
                    free(debug);
            }

            apps_autorun_end(core);
            return;
        }


        if (!strcasecmp(ext, "bas")) {
            if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                ddlog(core, 0, "Detected file: Basic (.bas)\n\n");
            }

            u8 *ImgDsk = idsk_createNewDisk();

            u16 basLength;
            u8 *bas = clean_basic(dsk, dsk_size, &basLength);

// Save cleaned file to test.bas
            // FILE *fic = fopen("test.bas", "wb");
            // fwrite(bas, 1, basLength, fic);
            // fclose(fic);

// Autotype in place of importfile
            // AutoType_SetString(core, bas, 1);    // Rest & Run
            // apps_autorun_files_flag = 0;


            idsk_importFile(ImgDsk, bas, (u32)basLength, "autorun.bas");

            free(bas);



            u32 length;
            u8 *buf = (u8 *)idsk_getDiskBuffer(ImgDsk, &length);

            BOOL autoStart = apps_autorun_files_flag;


            if (core->cliGetInfo) {
                ExecuteMenu(core, ID_EXIT, NULL);
            }

            LireDiskMem(core, buf, length);
            loadIni(core, 1);           // Load local in file

            if (!autoStart) {
                apps_autorun_end(core);
            }


            return;
        }

        if (!strcasecmp(ext, "rom")) {
            if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                ddlog(core, 0, "Detected file: Romfile (.rom)\n\n");
            }


            int ramblock;
            int pos = 0;

            for (ramblock = 0; ramblock < dsk_size / 16384; ramblock++) {
                int chunksize = 16384;

                if (ramblock == 0) {
                    memcpy(core->ROMINF, dsk + pos, chunksize);
                } else {
                    memcpy(core->ROMEXT[ramblock], dsk + pos, chunksize);
                }

                if (ramblock < 4) {
//                    memcpy(core->MemCPC + ramblock * 16384, dsk + pos, chunksize);

                }


                pos += chunksize;
            }

            if (core->cliGetInfo) {
                ExecuteMenu(core, ID_EXIT, NULL);
            }

            SoftResetCPC(core);

//            core->NumRomExt = 32;         // TODO: fix

//            core->NumRomExt = 32;         // TODO: fix


            apps_autorun_end(core);
            return;
        }
    }

    u8 header[32];

    if (dsk_size < 32) {
        appendIcon(core, 0, 4, 60);
        apps_autorun_end(core);
        return;
    }

    memcpy(header, dsk, 32);

    if (!memcmp(header, "MV - SNA", 8)) {     // .sna file
        if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
            ddlog(core, 0, "Detected file: Snapshot (.sna)\n\n");
        }

        snapshotLength = (u32)dsk_size;

        snapshot = (u8 *)malloc(snapshotLength);
        memcpy(snapshot, dsk, snapshotLength);

        if (snapshot != NULL) {
            LireSnapshotMem(core, snapshot, snapshotLength);
            loadIni(core, 1);       // Load local in file
        }

        apps_autorun_end(core);
        return;
    }

    if (!memcmp(header, "RIFF", 4)) {     // .cpr file
        if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
            ddlog(core, 0, "Detected file: Cartridge (.cpr)\n");
        }

        if (memcmp(header + 8, "AMS!", 4) != 0) {
            appendIcon(core, 0, 4, 60);
            apps_autorun_end(core);

            return;
        }

        u8 chunkid[4];     // chunk ID (4 character code - cb00, cb01, cb02... upto cb31 (max 512kB), other chunks are ignored)
        u8 chunklen[4];     // chunk length (always little-endian)
        u32 chunksize;     // chunk length, calcaulated from the above
        u32 ramblock;     // 16k RAM block chunk is to be loaded in to
        u32 pos = 0;

        u32 bytes_to_read = header[4] + (header[5] << 8) + (header[6] << 16) + (header[7] << 24);
        bytes_to_read -= 4;     // account for AMS! header

        pos = 12;

        while (bytes_to_read > 0) {
            if (pos + 4 > dsk_size) {
                ddlog(core, 2, "CPR: failed to read from cart image\n");
                return;     // TODO: fix return
            }
            memcpy(chunkid, dsk + pos, 4);
            pos += 4;
            bytes_to_read -= 4;

            if (pos + 4 > dsk_size) {
                ddlog(core, 2, "CPR: failed to read from cart image\n");
                return;
            }
            memcpy(chunklen, dsk + pos, 4);
            pos += 4;
            bytes_to_read -= 4;

            // calculate little-endian value, just to be sure
            chunksize = chunklen[0] + (chunklen[1] << 8) + (chunklen[2] << 16) + (chunklen[3] << 24);

            if (!memcmp(chunkid, "cb", 2)) {
                // load chunk into RAM
                // find out what block this is
                ramblock = (chunkid[2] - 0x30) * 10;
                ramblock += chunkid[3] - 0x30;

                if (ramblock >= 0 && ramblock < 32) {
                    if (chunksize > 16384) chunksize = 16384;
                    // clear RAM block

                    if (ramblock == 0) {
                        memset(core->ROMINF, 0, 0x4000);
                        memcpy(core->ROMINF, dsk + pos, chunksize);
//                    } else if (ramblock == 1) {
//                        memset(core->ROMEXT[0], 0, 0x4000);
//                        memcpy(core->ROMEXT[0], dsk + pos, chunksize);
                    } else if (ramblock == 3) {
                        memset(core->ROMEXT[3], 0, 0x4000);
                        memcpy(core->ROMEXT[3], dsk + pos, chunksize);

                        memset(core->ROMEXT[7], 0, 0x4000);
                        memcpy(core->ROMEXT[7], dsk + pos, chunksize);
                    } else {
                        memset(core->ROMEXT[ramblock], 0, 0x4000);
                        memcpy(core->ROMEXT[ramblock], dsk + pos, chunksize);
                    }

                    pos += chunksize;
                    bytes_to_read -= chunksize;
                    ddlog(core, 2, "CPR: Loaded chunk into RAM block %i\n", ramblock);
                }
            } else {
                ddlog(core, 2, "CPR: Unknown chunk '%4s', skipping %i bytes\n", chunkid, chunksize);
                if (chunksize != 0) {
//                    image_fseek(image, chunksize, SEEK_CUR);
                    pos += chunksize;
                    bytes_to_read -= chunksize;
                }
            }
        }

        if (core->cliGetInfo) {
            ExecuteMenu(core, ID_EXIT, NULL);
        }

        SoftResetCPC(core);
        core->NumRomExt = 32;     // TODO: fix

        apps_autorun_end(core);
        return;
    }

#ifdef ZIP_SUPPORT
    if (!memcmp(header, "PK", 2)) {     // .zip & .kcr files (kcr not supported yet)
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        if (mz_zip_reader_init_mem(&zip_archive, dsk, dsk_size, 0) == MZ_TRUE) {
            int i;
            // char isKcr = 0;

            for (i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++) {
                mz_zip_archive_file_stat file_stat;
                if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
                    ddlog(core, 2, "mz_zip_reader_file_stat() failed!\n");
                    mz_zip_reader_end(&zip_archive);
                    break;
                }

                if (!strcasecmp(file_stat.m_filename, "settings.ini")) {
                    // isKcr = 1;
                }

                ddlog(core, 2, "Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u, Is Dir: %u\n", file_stat.m_filename, file_stat.m_comment, (unsigned int)file_stat.m_uncomp_size, (unsigned int)file_stat.m_comp_size, mz_zip_reader_is_file_a_directory(&zip_archive, i));
            }

            // TODO: handle isKcr flag!

            for (i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++) {
                mz_zip_archive_file_stat file_stat;
                mz_zip_reader_file_stat(&zip_archive, i, &file_stat);     // Return has been tested in the precedent loop

                char *ext = strrchr(file_stat.m_filename, '.');
                if (ext != NULL) {
                    ext++;
                }

                if ((ext != NULL) &&
                    ((!strcasecmp(ext, "sna")) ||
                     (!strcasecmp(ext, "cpr")) ||
                     (!strcasecmp(ext, "dsk")))) {
                    unsigned char *undsk = (unsigned char *)malloc(file_stat.m_uncomp_size);
                    mz_zip_reader_extract_to_mem(&zip_archive, 0, undsk, (unsigned int)file_stat.m_uncomp_size, 0);

                    UseDatafile(core, undsk, (u32)file_stat.m_uncomp_size, file_stat.m_filename);

                    ExecuteMenu(core, ID_PAUSE_EXIT, NULL);

                    BOOL autoStart = 1;
                    if (!strcasecmp(ext, "dsk")) {           // Ignore the flag for cpr & sna
                        autoStart = apps_autorun_files_flag;
                    }

                    if (autoStart) {
                        ExecuteMenu(core, ID_AUTORUN, NULL);     // Do: open disk & autorun // TODO
                    }

//                    BOOL autoStart = apps_autorun_files_flag;

//                    LireDiskMem(core, undsk, (u32)file_stat.m_uncomp_size);
//                    loadIni(core, 1); // Load local in file

//                    if (!autoStart) {
//                        apps_autorun_end(core);
//                    }
                    return;
                }
            }     // End for
                  // No .dsk or .sna found :(
        }

        appendIcon(core, 0, 4, 60);
        apps_autorun_end(core);

        return;
    }
#endif /* ifdef ZIP_SUPPORT */

    if ((!memcmp(header, "MV - CPC", 8)) || (!memcmp(header, "EXTENDED", 8))) {      // .dsk file
        if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
            ddlog(core, 0, "Detected file: Disk (.dsk)\n\n");
        }

        BOOL autoStart = apps_autorun_files_flag;

        LireDiskMem(core, dsk, (u32)dsk_size);
        loadIni(core, 1);       // Load local in file

        if (!autoStart) {
            apps_autorun_end(core);
        }

        if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
            char tmp[32];
            int n, len;

            if (!memcmp(dsk, "MV - CPC", 8)) {
                ddlog(core, 0, "Standard CPC DSK File (");
            }

            if (!memcmp(dsk, "EXTENDED", 8)) {
                ddlog(core, 0, "EXTENDED CPC DSK File (");
            }

            ddlog(core, 0, "%d sides, %d tracks)\n", dsk[0x31], dsk[0x30]);

            len = 0;
            memcpy(tmp, dsk + 0x22, 16);
            tmp[14] = 0;
            for (n = 0; n < 14; n++) {
                if (tmp[n] < 32) {
                    tmp[n] = 32;
                }
                if (tmp[n] > 32) {
                    len++;
                }
            }
            if (len > 0) {
                ddlog(core, 0, "Created by %s\n", tmp);
            }

            ddlog(core, 0, "\n");

            int total = 0, file = 0;
            for (n = 0; n < apps_autorun_files_count; n++) {
                char add[256];

                sprintf(add, "%c%c%c %02x",
                        (apps_autorun_files[n].readonly) ? 'R' : '-',
                        (apps_autorun_files[n].hidden) ? 'H' : '-',
                        (apps_autorun_files[n].archived) ? 'A' : '-',
                        apps_autorun_files[n].user
                        );

                if ((apps_autorun_files[n].load != 0) || (apps_autorun_files[n].exec != 0)) {
                    char tmp[32];
                    sprintf(tmp, " L=%04X E=%04X",
                            apps_autorun_files[n].load,
                            apps_autorun_files[n].exec
                            );
                    strcat(add, tmp);
                }

                char type = '?';
                switch (apps_autorun_files[n].type) {
                    case 0:
                        type = 'A';
                        break;
                    case 1:
                        type = 'P';
                        break;
                    case 2:
                        type = 'B';
                        break;
                }


                ddlog(core, 0, "%-8s %3s   %7d %c %s\n", apps_autorun_files[n].name, apps_autorun_files[n].ext, apps_autorun_files[n].size, type, add);

                total += apps_autorun_files[n].nbpages * 128;
                file++;
                // ddlog(core, 0, "%8s %3s %03d/%03d %02x\n", apps_autorun_files[n].name, apps_autorun_files[n].ext, apps_autorun_files[n].page, apps_autorun_files[n].nbpages, apps_autorun_files[n].user);
            }
            ddlog(core, 0, "%5d file%c  %9d bytes\n", file, (file > 1) ? 's' : ' ', total);

            ddlog(core, 0, "\n");
            // ddlog(core, 0, "\n");


        }

        if (core->cliGetInfo) {
            ExecuteMenu(core, ID_EXIT, NULL);
        }


        return;
    }

    appendIcon(core, 0, 4, 60);
    apps_autorun_end(core);
}     /* apps_autorun_init */

void DispAutorun(core_crocods_t *core, u16 keys_pressed0)
{
//    static int key = 0;

    int y;

    u16 keys_pressed = appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 168;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap;     // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    // Text begin in 12,36 (max 13 lines, 26 columns)

    for (y = 0; y < 168; y++) {
        memcpy(pdwAddr, core->select + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    char *title = "Select file to run";

    cpcprint16(core, core->overlayBitmap, 320, (256 - (int)strlen(title) * 8) / 2, 15, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    if (apps_autorun_files_begin != 0) {
        dispIcon8(core, 235, 36 + 0 * 8, 8);
    }
    if (apps_autorun_files_count - apps_autorun_files_begin > 13) {
        dispIcon8(core, 235, 36 + 12 * 8, 9);
    }

    //                                                     1234567890ABCDEFGHIJKJIHGFEDCBA0987654321
    cpcprint16_6w(core, core->overlayBitmap, 320, 4, 156, "          Arrows & Start to RUN          ", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    for (y = 0; y < 13; y++) {
        int n = y + apps_autorun_files_begin;

        if (n < apps_autorun_files_count) {
            char text[27 + 1];

            // 3+8+1+3+1+5+1+2

            snprintf(text, 28, "   %8s %3s %05d %02x   ", apps_autorun_files[n].name, apps_autorun_files[n].ext, apps_autorun_files[n].nbpages, apps_autorun_files[n].user);

            if (n == apps_autorun_files_selected) {
                cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0xFF), 1, 0);
            } else {
                cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
            }
        }
    }

    if ((apps_autorun_files_count == 1) && (!strcasecmp(apps_autorun_files[0].name, "autorun"))) {
        keys_pressed = KEY_START;
    }

    if (core->ipc.touchDown == 1) {
        core->ipc.touchDown = 0;

        int x, y;           // , n;

        x = core->ipc.touchXpx - core->overlayBitmap_posx - 9;
        y = core->ipc.touchYpx - core->overlayBitmap_posy - 36;

        if ((x >= 0) && (x < 226)) {
            apps_autorun_files_selected = y / 8 + apps_autorun_files_begin;

            if ((y >= 0) && (y < 103) && (apps_autorun_files_selected >= 0) && (apps_autorun_files_selected < apps_autorun_files_count)) {
                keys_pressed = keys_pressed | KEY_START;
            } else {
                keys_pressed = keys_pressed | KEY_B;
            }
        } else if ((x >= 226) && (x < 245)) {
            if ((y >= 0) && (y < 8)) {
                keys_pressed = keys_pressed | KEY_LEFT;
            } else if ((y >= 95) && (y < 103)) {
                keys_pressed = keys_pressed | KEY_RIGHT;
            }
        } else {
            keys_pressed = keys_pressed | KEY_B;
        }
    }

    if ((keys_pressed & KEY_START) || (core->cliAutorun == 1)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        char usefile[256];
        char autoString[256];

        char *ext = apps_autorun_files[apps_autorun_files_selected].ext;

        strcpy(usefile, apps_autorun_files[apps_autorun_files_selected].name);

        if ((ext[0] != 0) & (ext[0] != 32)) {
            strcat(usefile, ".");
            strcat(usefile, ext);
        }

        if (core->cliAutoload == 0) {
            sprintf(autoString, "run\"%s\r", usefile);      // \n is 0x0a, \r is 0x0d
        } else {
            sprintf(autoString, "load\"%s\r", usefile);
        }
        AutoType_SetString(core, autoString, 1);        // Rest & Run
        apps_autorun_end(core);
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
        apps_autorun_files_selected--;
        if (apps_autorun_files_selected < 0) {
            apps_autorun_files_selected = apps_autorun_files_count - 1;
            apps_autorun_files_begin = apps_autorun_files_count - 13;
            if (apps_autorun_files_begin < 0) {
                apps_autorun_files_begin = 0;
            }
        }
        if (apps_autorun_files_selected < apps_autorun_files_begin) {
            apps_autorun_files_begin--;
        }
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        apps_autorun_files_selected++;
        if (apps_autorun_files_selected >= apps_autorun_files_count) {
            apps_autorun_files_selected = 0;
            apps_autorun_files_begin = 0;
        }
        if (apps_autorun_files_selected > apps_autorun_files_begin + 12) {
            apps_autorun_files_begin++;
        }
    }
}     /* DispKeyboard */

// Used in upd.c
int GetMinSect(u8 *imgDsk);
int GetPosData(u8 *imgDsk, int track, int sect, char SectPhysique);

void LireDiskMem(core_crocods_t *core, u8 *rom, u32 romsize)
{
    int j, pos;

    char isOk = 0;

    isOk = ( (!memcmp(rom, "MV - CPC", 8)) || (!memcmp(rom, "EXTENDED", 8)) );

    if (isOk == 0) {
        return;
    }

    EjectDiskUPD(core);

    core->LongFic = (int)romsize - sizeof(core->Infos);

    memcpy(&core->Infos, rom, sizeof(core->Infos));
    memcpy(core->ImgDsk, rom + sizeof(core->Infos), core->LongFic);

    core->Image = 1;
    core->FlagWrite = 0;

    ChangeCurrTrack(core, 0);     // Met a jour posdata

    apps_autorun_files_count = 0;

    pos = core->PosData;

    int i;

    ddlog(core, 2, "First pos: %d\n", pos);

    for (i = 0; i < 64; i++) {
        idsk_StDirEntry Dir;
        memcpy(&Dir, idsk_getInfoDirEntry((u8 *)rom, i), sizeof( idsk_StDirEntry ));

        char filename[9];
        char ext[4];
        char hidden, readonly, archived;

        filename[0] = 0;
        ext[0] = 0;

        for (j = 0; j < 8; j++) {
            filename[j] = Dir.Nom[j] & 0x7F;
            if (filename[j] == 32) filename[j] = 0;
            if ((filename[j] < 32) && (filename[j] > 0)) filename[j] = '.';
        }
        filename[8] = 0;

        readonly = ((Dir.Ext[0] & 0x80) == 0x80);
        hidden = ((Dir.Ext[1] & 0x80) == 0x80);
        archived = ((Dir.Ext[2] & 0x80) == 0x80);

        for (j = 0; j < 3; j++) {
            ext[j] = Dir.Ext[j] & 0x7F;
            if (ext[j] == 32) ext[j] = 0;
            if ((ext[j] < 32) && (ext[j] > 0)) ext[j] = '.';
        }
        ext[3] = 0;

        if ((Dir.User != 0xE5) && (filename[0] != 0) && (Dir.NbPages != 0)) {
            signed int found = -1;
            int n;
            u16 AdresseCharg;
            u16 AdresseExec;
            u16 TailleFic;
            u8 type;
            u8 haveAmsdos;


            // printf("%s,", filename);

            {
                u8 FirstBlock = 1;

                TailleFic = 0;
                AdresseCharg = 0;
                AdresseExec = 0;
                haveAmsdos = 0;
                type = 0;

                int l = ( Dir.NbPages + 7 ) >> 3;
                for ( int j = 0; j < l; j++ ) {
                    int TailleBloc = 1024;
                    unsigned char *p = idsk_readBloc(rom, Dir.Blocks[j]);
                    if ( FirstBlock ) {
                        if ( idsk_checkAmsdos(p) ) {
                            type = p[ 0x12 ];

                            TailleFic = p[ 0x18 + 1 ] * 256 + p[ 0x18 ];
                            AdresseCharg = p[ 0x15 + 1 ] * 256 + p[ 0x15 ];
                            AdresseExec = p[ 0x1a + 1 ] * 256 + p[ 0x1a ];

                            haveAmsdos = 1;

                            TailleBloc -= sizeof( idsk_StAmsdos );
                            memcpy(p, &p[ 0x80 ], TailleBloc);

                            // printf("(found)");
                        }
                        FirstBlock = 0;
                    }
                }

                // printf("%04X,%04X,%d (%d)\n,", AdresseCharg, AdresseExec, TailleFic, FirstBlock);
            }

            for (n = 0; n < apps_autorun_files_count; n++) {
                if ((!strcasecmp(apps_autorun_files[n].name, filename)) && (!strcasecmp(apps_autorun_files[n].ext, ext))) {
                    found = n;
                }
            }

            if (found == -1) {
                strcpy(apps_autorun_files[apps_autorun_files_count].name, filename);
                strcpy(apps_autorun_files[apps_autorun_files_count].ext, ext);
                apps_autorun_files[apps_autorun_files_count].nbpages = Dir.NbPages;
                apps_autorun_files[apps_autorun_files_count].user = Dir.User;
                apps_autorun_files[apps_autorun_files_count].readonly = readonly;
                apps_autorun_files[apps_autorun_files_count].hidden = hidden;
                apps_autorun_files[apps_autorun_files_count].archived = archived;
                if (haveAmsdos) {
                    apps_autorun_files[apps_autorun_files_count].exec = AdresseExec;
                    apps_autorun_files[apps_autorun_files_count].load = AdresseCharg;
                    apps_autorun_files[apps_autorun_files_count].type = type;
                    apps_autorun_files[apps_autorun_files_count].size = TailleFic;
                } else {
                    apps_autorun_files[apps_autorun_files_count].size =  Dir.NbPages * 128;
                }
                apps_autorun_files_count++;

                ddlog(core, 2, "%s %s", filename, ext);
            } else {
                strcpy(apps_autorun_files[found].name, filename);
                strcpy(apps_autorun_files[found].ext, ext);
                apps_autorun_files[found].nbpages += Dir.NbPages;
                apps_autorun_files[found].user = Dir.User;
                apps_autorun_files[found].readonly = readonly;
                apps_autorun_files[found].hidden = hidden;
                apps_autorun_files[found].archived = archived;
            }
        }


    }

    // Move to the first line following extension (empty, .bas & .bin)

    int place = 0;

    int n;

    for (n = 0; n < apps_autorun_files_count; n++) {
        if (!strcasecmp(apps_autorun_files[n].ext, "")) {
            if (place != n) {
                StDirEntryCroco tmp;
                memcpy(&tmp, &apps_autorun_files[n], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[n], &apps_autorun_files[place], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[place], &tmp, sizeof(StDirEntryCroco));
            }
            place++;
        }
    }

    for (n = 0; n < apps_autorun_files_count; n++) {
        if (!strcasecmp(apps_autorun_files[n].ext, "bas")) {
            if (place != n) {
                StDirEntryCroco tmp;
                memcpy(&tmp, &apps_autorun_files[n], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[n], &apps_autorun_files[place], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[place], &tmp, sizeof(StDirEntryCroco));
            }
            place++;
        }
    }

    for (n = 0; n < apps_autorun_files_count; n++) {
        if (!strcasecmp(apps_autorun_files[n].ext, "bin")) {
            if (place != n) {
                StDirEntryCroco tmp;
                memcpy(&tmp, &apps_autorun_files[n], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[n], &apps_autorun_files[place], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[place], &tmp, sizeof(StDirEntryCroco));
            }
            place++;
        }
    }



}     /* LireDiskMem */
