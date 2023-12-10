#include  "snapshot.h"

#include  "plateform.h"
#include  "sound.h"
#include  "crtc.h"
#include  "ppi.h"
#include  "vga.h"
#include  "z80.h"
#include "z80_cap32.h"
#include "emu2149.h"

#include "os.h"

extern t_z80regs z80; // in cap32
extern PSG psg; // in sound.c

#ifdef USE_SNAPSHOT

// https://www.cpcwiki.eu/index.php/Format:SNA_snapshot_file_format

/********************************************************* !NAME! **************
* Nom : StSnapShot
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Structure en-t�te fichier snapshot
*
********************************************************** !0! ****************/
#pragma pack(1)
typedef struct {
    u16 AF;
    u16 BC;
    u16 DE;
    u16 HL;
    u16 IR;
    UBYTE IFF1;
    UBYTE IFF2;
    u16 IX;
    u16 IY;
    u16 SP;
    u16 PC;
    UBYTE InterruptMode;
    u16 _AF;
    u16 _BC;
    u16 _DE;
    u16 _HL;
} _SRegs;

typedef struct {
    char Id[ 0x10 ];  // "MV - SNA'
    UBYTE Version;
    _SRegs Z80;
    UBYTE InkReg;
    UBYTE InkData[ 17 ];
    UBYTE VGARom;
    UBYTE VGARam;
    UBYTE CRTCIndex;
    UBYTE CRTCReg[ 18 ];
    UBYTE NumRom;
    UBYTE PPI[ 4 ];
    UBYTE PsgIndex;
    UBYTE PsgData[ 16 ];

    // v2

    u8 ram_size[2];         // 6B-6C
    u8 CPCType; // 0 = CPC464, 1 = CPC664, 2 = CPC6128, 3 = unknown, 4 = 6128 Plus, 5 = 464 Plus, 6 = GX4000
    u8 intNumber;
    u8 multimode[6];

    // v3

    u8 drvA_DOSfilename[13];
    u8 drvB_DOSfilename[13];
    u8 cart_DOSfilename[13];
    u8 fdc_motor;
    u8 drvA_current_track;
    u8 drvB_current_track;
    u8 drvC_current_track;
    u8 drvD_current_track;
    u8 printer_data;
    u8 psg_env_step;
    u8 psg_env_direction;
    u8 crtc_type;
    u8 crtc_addr[2];
    u8 crtc_scanline[2];
    u8 crtc_char_count[2];
    u8 crtc_line_count;
    u8 crtc_raster_count;
    u8 crtc_vt_adjust_count;
    u8 crtc_hsw_count;
    u8 crtc_vsw_count;
    u8 crtc_flags[2];
    u8 ga_int_delay;
    u8 ga_sl_count;
    u8 z80_int_pending;

    u8 Unused[ 75];
} StSnapShot;

#pragma pack()

int getSnapshotSize(core_crocods_t *core)
{
    return sizeof(StSnapShot) + 0x20000;
}

char * getSnapshot(core_crocods_t *core, int *len)
{
    char *buffer;

    int i;
    StSnapShot SnapShot;

    strcpy(SnapShot.Id, "MV - SNA");
    SnapShot.Version = 1;

    if ((croco_cpu_doFrame == cap32_cpu_doFrame) || (croco_cpu_doFrame == cap32_cpu_doFrame_debug)) {
        SnapShot.Z80.AF = z80.AF.w.l;
        SnapShot.Z80.BC = z80.BC.w.l;
        SnapShot.Z80.DE = z80.DE.w.l;
        SnapShot.Z80.HL = z80.HL.w.l;

        reg_pair IR;
        IR.b.h = z80.I;
        IR.b.l = z80.R;
        SnapShot.Z80.IR = IR.w.l;

        SnapShot.Z80.IFF1 = z80.IFF1;
        SnapShot.Z80.IFF2 = z80.IFF2;
        SnapShot.Z80.IX = z80.IX.w.l;
        SnapShot.Z80.IY = z80.IY.w.l;
        SnapShot.Z80.SP = z80.SP.w.l;
        SnapShot.Z80.PC = z80.PC.w.l;
        SnapShot.Z80.InterruptMode = z80.IM;
        SnapShot.Z80._AF = z80.AFx.w.l;
        SnapShot.Z80._BC = z80.BCx.w.l;
        SnapShot.Z80._DE = z80.DEx.w.l;
        SnapShot.Z80._HL = z80.HLx.w.l;
    } else {
        SnapShot.Z80.AF = core->Z80.AF.Word;
        SnapShot.Z80.BC = core->Z80.BC.Word;
        SnapShot.Z80.DE = core->Z80.DE.Word;
        SnapShot.Z80.HL = core->Z80.HL.Word;
        SnapShot.Z80.IR = core->Z80.IR.Word;
        SnapShot.Z80.IFF1 = core->Z80.IFF1;
        SnapShot.Z80.IFF2 = core->Z80.IFF2;
        SnapShot.Z80.IX = core->Z80.IX.Word;
        SnapShot.Z80.IY = core->Z80.IY.Word;
        SnapShot.Z80.SP = core->Z80.SP.Word;
        SnapShot.Z80.PC = core->Z80.PC.Word;
        SnapShot.Z80.InterruptMode = core->Z80.InterruptMode;
        SnapShot.Z80._AF = core->Z80._AF.Word;
        SnapShot.Z80._BC = core->Z80._BC.Word;
        SnapShot.Z80._DE = core->Z80._DE.Word;
        SnapShot.Z80._HL = core->Z80._HL.Word;
    }

    SnapShot.InkReg = (UBYTE)core->PenIndex;
    for (i = 0; i < 16 + 1; i++) {
        SnapShot.InkData[ i ] = (UBYTE)core->TabCoul[ i ];
    }

    SnapShot.VGARom = (UBYTE)core->DecodeurAdresse;
    SnapShot.VGARam = (UBYTE)core->RamSelect;
    SnapShot.CRTCIndex = (UBYTE)core->CRTC_Reg;
    for (i = 0; i < 18; i++) {
        SnapShot.CRTCReg[ i ] = (UBYTE)core->RegsCRTC[ i ];
    }

    SnapShot.NumRom = (UBYTE)core->NumRomExt;
    for (i = 0; i < 4; i++) {
        SnapShot.PPI[ i ] = (UBYTE)core->RegsPPI[ i ];
    }

    SnapShot.PsgIndex = (UBYTE)core->RegPSGSel;
    for (i = 0; i < 16; i++) {
        // SnapShot.PsgData[ i ] = 0; // ( UBYTE )RegsPSG[ i ]; // On ne sauve pas les registres PSG
        SnapShot.PsgData[ i ] = psg.reg[ i ]; // On ne sauve pas les registres PSG
    }

    // For snapshot v2

    SnapShot.ram_size[0] = 128;
    SnapShot.ram_size[1] = 0;
    SnapShot.CPCType = 2;
//   u8 intNumber;
//     u8 multimode[6];

    *len = sizeof(StSnapShot) + 0x20000;

    buffer = (char *)malloc(*len);
    memcpy(buffer, &SnapShot, sizeof(StSnapShot));
    memcpy(buffer + sizeof(StSnapShot), core->MemCPC, 0x20000);

    return buffer;
} /* getSnapshot */

/********************************************************* !NAME! **************
* Nom : SauveSnap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Sauvegarde d'un fichier snapshot
*
* R�sultat    : /
*
* Variables globales modifi�es : /
*
********************************************************** !0! ****************/
void SauveSnap(core_crocods_t *core, char *Nom)
{
    FILE *fp = os_fopen(Nom, "wb");

    if (fp) {
        int len;
        char *buffer = getSnapshot(core, &len);
        if (buffer != NULL) {
            fwrite(buffer, 1, len, fp);
            free(buffer);
        }
        fclose(fp);
    }
}

void SauveScreen(char *Nom)
{
    // FILE *fp;
    // fp = fopen( "Nom", "w" );
    // fwrite( MemCPC, 1, 80*50, fp );
    // fclose( fp );
    // fp = fopen( "Nom2", "w" );
    // fwrite( MemCPC, 1, 0x20000, fp );
    // fclose( fp );
}

/********************************************************* !NAME! **************
* Nom : LireSnap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un fichier snapshot
*
* R�sultat    : /
*
* Variables globales modifi�es : Z80, PenSenect, RegPSGSel, RomExt, RegCRTCSel
*
********************************************************** !0! ****************/

/********************************************************* !NAME! **************
* Nom : LireSnap
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un fichier snapshot
*
* R�sultat    : /
*
* Variables globales modifi�es : Z80, PenSenect, RegPSGSel, RomExt, RegCRTCSel
*
********************************************************** !0! ****************/
void LireSnapshotMem(core_crocods_t *core, u8 *snap, u32 snapsize)
{
    int i;
    StSnapShot SnapShot;

    memcpy(&SnapShot, snap, sizeof(SnapShot));
    if (!strncmp(SnapShot.Id, "MV - SNA", 8)) {

        if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
            ddlog(core, 0, "Snapshot v%d\n", SnapShot.Version);
        }

        if (SnapShot.Version > 3) {
            if (core->cliGetInfo) {
                ddlog(core, 0, "Unsupported snapshot\n");

                ExecuteMenu(core, ID_EXIT, NULL);
            }

            appendIcon(core, 0, 4, 60); // TODO: send "Could not read Snapshot"
            return;
        }

        int dwSnapSize;
        dwSnapSize = SnapShot.ram_size[0] + (SnapShot.ram_size[1] * 256); // memory dump size
        dwSnapSize &= ~0x3f; // limit to multiples of 64


        if (dwSnapSize != 0) { // We don't have compressed chunk after the header
            if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                ddlog(core, 0, "Snapsize: %d bytes\n", dwSnapSize);
            }

            memcpy(core->MemCPC, snap + sizeof(SnapShot), dwSnapSize * 1024);
        }

        if (SnapShot.Version >= 1) {        // Load v1 specific
            if ((croco_cpu_doFrame == cap32_cpu_doFrame) || (croco_cpu_doFrame == cap32_cpu_doFrame_debug)) {
                z80.AF.w.l = SnapShot.Z80.AF;
                z80.BC.w.l = SnapShot.Z80.BC;
                z80.DE.w.l = SnapShot.Z80.DE;
                z80.HL.w.l = SnapShot.Z80.HL;

                reg_pair IR;
                IR.w.l = SnapShot.Z80.IR;

                z80.I = IR.b.h;
                z80.R = IR.b.l;

                z80.IFF1 = SnapShot.Z80.IFF1;
                z80.IFF2 = SnapShot.Z80.IFF2;
                z80.IX.w.l = SnapShot.Z80.IX;
                z80.IY.w.l = SnapShot.Z80.IY;
                z80.SP.w.l = SnapShot.Z80.SP;
                z80.PC.w.l = SnapShot.Z80.PC;
                z80.IM = SnapShot.Z80.InterruptMode;
                z80.AFx.w.l = SnapShot.Z80._AF;
                z80.BCx.w.l = SnapShot.Z80._BC;
                z80.DEx.w.l = SnapShot.Z80._DE;
                z80.HLx.w.l = SnapShot.Z80._HL;

                if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                    ddlog(core, 0, "A : %02x F : %02x B : %02x C : %02x D : %02x E : %02x H : %02x L : %02x \n", z80.AF.b.h, z80.AF.b.l, z80.BC.b.h, z80.BC.b.l, z80.DE.b.h, z80.DE.b.l, z80.HL.b.h, z80.HL.b.l);
                    ddlog(core, 0, "A': %02x F': %02x B': %02x C': %02x D': %02x E': %02x H': %02x L': %02x \n", z80.AFx.b.h, z80.AFx.b.l, z80.BCx.b.h, z80.BCx.b.l, z80.DEx.b.h, z80.DEx.b.l, z80.HLx.b.h, z80.HLx.b.l);
                    ddlog(core, 0, "IX: %04x IY: %04x \n", z80.IX.w.l, z80.IY.w.l);
                    ddlog(core, 0, "PC: %04x SP: %04x \n", z80.PC.w.l, z80.SP.w.l);
                    ddlog(core, 0, "I : %02x R : %02x \n", z80.I, z80.R);
                    ddlog(core, 0, "IFF1: %02x IFF2: %02x\n", z80.IFF1, z80.IFF2);
                    ddlog(core, 0, "IM: %02x\n", z80.IM);
                }

            } else {
                core->Z80.AF.Word = SnapShot.Z80.AF;
                core->Z80.BC.Word = SnapShot.Z80.BC;
                core->Z80.DE.Word = SnapShot.Z80.DE;
                core->Z80.HL.Word = SnapShot.Z80.HL;
                core->Z80.IR.Word = SnapShot.Z80.IR;
                core->Z80.IFF1 = SnapShot.Z80.IFF1;
                core->Z80.IFF2 = SnapShot.Z80.IFF2;
                core->Z80.IX.Word = SnapShot.Z80.IX;
                core->Z80.IY.Word = SnapShot.Z80.IY;
                core->Z80.SP.Word = SnapShot.Z80.SP;
                core->Z80.PC.Word = SnapShot.Z80.PC;
                core->Z80.InterruptMode = SnapShot.Z80.InterruptMode;
                core->Z80._AF.Word = SnapShot.Z80._AF;
                core->Z80._BC.Word = SnapShot.Z80._BC;
                core->Z80._DE.Word = SnapShot.Z80._DE;
                core->Z80._HL.Word = SnapShot.Z80._HL;
            }

            // PPI
            WritePPI(core, 0xF400, SnapShot.PPI[ 0 ]);
            //         WritePPI( 0xF500, SnapShot.PPI[ 1 ] );
            WritePPI(core, 0xF600, SnapShot.PPI[ 2 ]);
            WritePPI(core, 0xF700, SnapShot.PPI[ 3 ]);

            Reset8912(core);
            for (i = 0; i < 16; i++) {
                Write8912(core, i, SnapShot.PsgData[ i ]);
            }

            core->RegPSGSel = SnapShot.PsgIndex;

            // GA
            for (i = 0; i < 17; i++) {
                WriteVGA(core, 0, i);
                WriteVGA(core, 0, 0x40 | SnapShot.InkData[ i ]);
            }
            core->PenSelection = SnapShot.InkReg;
            WriteVGA(core, 0, core->PenSelection & 0x1F);

            core->NumRomExt = SnapShot.NumRom;
            WriteVGA(core, 0, 0x80 | SnapShot.VGARom);
            WriteVGA(core, 0, 0xC0 | SnapShot.VGARam);

            // CRTC
            for (i = 0; i < 18; i++) {
                RegisterSelectCRTC(core, i);
                WriteCRTC(core, SnapShot.CRTCReg[i]);
            }
            core->CRTC_Reg = SnapShot.CRTCIndex;

            core->UpdateInk = 1;
        }
        if (SnapShot.Version >= 2) {        // Load v2 specific
        }
        if (SnapShot.Version >= 3) {        // Load v3 specific

            u8 chunkid[4]; // chunk ID (4 character code - cb00, cb01, cb02... upto cb31 (max 512kB), other chunks are ignored)
            u8 chunklen[4]; // chunk length (always little-endian)
            u32 chunksize; // chunk length, calcaulated from the above


            u32 pos = 0x100;

            u32 bytes_to_read = snapsize - 0x100;


            while (bytes_to_read > 0) {
                memcpy(chunkid, snap + pos, 4);
                pos += 4;

                memcpy(chunklen, snap + pos, 4);
                pos += 4;

                chunksize = chunklen[0] + (chunklen[1] << 8) + (chunklen[2] << 16) + (chunklen[3] << 24);     // calculate little-endian value, just to be sure

                bytes_to_read -= 8;
                bytes_to_read -= chunksize;

                if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
                    ddlog(core, 0, "%c%c%c%c: %d bytes\n", chunkid[0], chunkid[1], chunkid[2], chunkid[3], chunksize);
                }

                if (!memcmp(chunkid, "MEM0", 4)) {
                    u8 byte;
                    int mempos = 0;
                    do {
                        byte = snap[pos];
                        if (byte != 0xe5) {
                            core->MemCPC[mempos] = byte;
                            mempos++;
                            pos++;
                            chunksize--;
                        } else {
                            u8 n, count;
                            count = snap[pos + 1];
                            byte = snap[pos + 2];
                            for (n = 0; n < count; n++) {
                                core->MemCPC[mempos] = byte;
                                mempos++;
                            }
                            pos += 3;
                            chunksize -= 3;
                        }
                    } while (chunksize > 0);

                } else if (!memcmp(chunkid, "BRKS", 4)) {

                } else if (!memcmp(chunkid, "BRKC", 4)) {

                }
            }
        }

        if ((core->cliGetInfo) || (core->cliVerbose > 0)) {
            ddlog(core, 0, "\n");
        }
        if (core->cliGetInfo) {
            ExecuteMenu(core, ID_EXIT, NULL);
        }

    }
}     /* LireSnapshotMem */

int HaveSlotSnap(core_crocods_t *core, char *file, int c)
{
    // char snap[256];
    // char *buf;

    // sprintf(snap, "/%s.%d", file, c + 1);
    // buf = strchr(snap, '.');
    // *buf = '_';

    // return FileExists(snap);

    return 0;
}

void LoadSlotSnap(core_crocods_t *core, int c)
{
    char snap[MAX_PATH + 1];
//    char *buf;
    u8 *rom = NULL;
    u32 romsize = 0;

    sprintf(snap, "%s/snap/%s_%d.sna", core->home_dir, core->filename, c);
//    mydebug(core, "Loading snap: %s\n", snap);

    rom = FS_Readfile(snap, &romsize);

    if (rom != NULL) {
        LireSnapshotMem(core, rom, romsize);
        free(rom);
    }
}

#endif /* ifdef USE_SNAPSHOT */
