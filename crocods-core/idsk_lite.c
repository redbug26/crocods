#include "crocods.h"
#include "retro_endianness.h"

#include <ctype.h>

#include "idsk_lite.h"

enum { ERR_NO_ERR = 0, ERR_NO_DIRENTRY, ERR_NO_BLOCK, ERR_FILE_EXIST };

#define ASCII_MODE  0
#define BINARY_MODE 1

#define MIN(X, Y)    (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y)    (((X) > (Y)) ? (X) : (Y))

#define SWAP_2(x)    ( (((x) & 0xff) << 8) | ((u16)(x) >> 8) )
#define SWAP_4(x)    ( ((x) << 24) | \
                       (((x) << 8) & 0x00ff0000) | \
                       (((x) >> 8) & 0x0000ff00) | \
                       ((x) >> 24) )

#define FIX_SHORT(x) (*(u16 *)&(x) = SWAP_2(*(u16 *)&(x)))
#define FIX_INT(x)   (*(u32 *)&(x) = SWAP_4(*(u32 *)&(x)))

#define SECTSIZE 512

/*
 * int main(int argc, char **argv)
 * {
 *  u8 *ImgDsk = idsk_createNewDisk();
 *
 *  // TODO: check 0x0A, 0x0D for carriage return
 *
 *  FILE *Hfile;
 *
 *  u8 buf[0x20000];
 *  if ((Hfile = fopen("/Users/miguelvanhove/Dropbox/Sources/cpc/sampleDisk/test.bas", "rb")) == NULL) {
 *      return 0;
 *  }
 *  u32 len = (u32)fread(buf, 1, 0x20000, Hfile);
 *  fclose(Hfile);
 *
 *  idsk_importFile(ImgDsk, buf, len, "redbug.bas");
 *
 *  if (1 == 1) {
 *      u32 length;
 *      char *buf = idsk_getDiskBuffer(ImgDsk, &length);
 *
 *      if (buf != NULL) {
 *          FILE *fic;
 *          fic = fopen("/Users/miguelvanhove/Dropbox/Sources/cpc/sampleDisk/basic.dsk", "wb+");
 *          fwrite(buf, 1, length, fic);
 *          fclose(fic);
 *      }
 *      free(buf);
 *  }
 *
 *  free(ImgDsk);
 *
 *  return (EXIT_SUCCESS);
 * }
 */

// iDsk functions

char idsk_isBigEndian(void)
{
    return RETRO_IS_BIG_ENDIAN;
}

//
// Calcule et positionne le checksum AMSDOS
//
void idsk_setChecksum(idsk_StAmsdos *pEntete)
{
    int i, Checksum = 0;
    u8 *p = (u8 *)pEntete;

    for (i = 0; i < 67; i++) {
        Checksum += *(p + i);
    }

    pEntete->CheckSum = (u16)Checksum;
}

//
// Verifie si en-tete AMSDOS est valide
//
char idsk_checkAmsdos(u8 *Buf)
{
    int i, Checksum = 0;
    char ModeAmsdos = 0;
    u16 CheckSumFile;

    CheckSumFile = Buf[ 0x43 ] + Buf[ 0x43 + 1 ] * 256;
    for (i = 0; i < 67; i++) {
        Checksum += Buf[ i ];
    }

    if ( (CheckSumFile == (u16)Checksum) && Checksum) ModeAmsdos = 1;

    return(ModeAmsdos);
}

idsk_StAmsdos * idsk_creeEnteteAmsdos(char *NomFic, u16 Longueur)
{
    static char NomReel[ 256 ];
    static idsk_StAmsdos Entete;
    static char Nom[ 12 ];
    int i;

    strcpy(NomReel, NomFic);
    memset(&Entete, 0, sizeof(Entete) );
    memset(Nom, ' ', sizeof(Nom) );
    char *p = NULL;

    do {
        p = strchr(NomReel, '/'); // Sous linux c'est le / qu'il faut enlever ...
        if (p) strcpy(NomReel, ++p);
    } while (p);
    p = strchr(NomReel, '.');
    if (p) *p++ = 0;

    int l = (int)strlen(NomReel);

    if (l > 8) l = 8;

    for (i = 0; i < l; i++) {
        Nom[ i ] = (char)toupper(NomReel[ i ]);
    }

    if (p)
        for (i = 0; i < 3; i++) {
            Nom[ i + 8 ] = (char)toupper(p[ i ]);
        }

    memcpy(Entete.FileName, Nom, 11);
    Entete.Length = 0;     // Non renseign� par AMSDos !!
    Entete.RealLength = Entete.LogicalLength = Longueur;
    Entete.FileType = 2; // Fichier binaire

    idsk_setChecksum(&Entete);

    return(&Entete);
} /* idsk_creeEnteteAmsdos */

/// Return the filename formated to AMSDOS (8+3)
/// Need to be freed!
/// @param AmsName <#AmsName description#>
char * idsk_getNomAmsdos(const char *AmsName)
{
    // Extract the name (without directory components)
    const char *lastSlash = strrchr(AmsName, '/');
    const char *lastBackslash = strrchr(AmsName, '\\');

    if (lastSlash > lastBackslash) {
        AmsName = lastSlash + 1;
    } else {
        if (lastSlash < lastBackslash) {
            AmsName = lastBackslash + 1;
        }
    }

    char *NomAmsdos = (char *)malloc(16);
    int i;

    char *p = NomAmsdos;

    for (i = 0; i < 8; i++) {
        if (*AmsName != ' ' && *AmsName != '.') {
            *p++ = *AmsName++;
        }
    }

    while (*AmsName != '.' && *AmsName)
        AmsName++;

    AmsName++;

    *p = 0;
    strcat(NomAmsdos, ".");

    for (i = 0; *AmsName && i < 3; i++) {
        *++p = *AmsName++;
    }

    *++p = 0;
    i = 0;
    while (NomAmsdos[ i ])
        NomAmsdos[ i++ ] &= 0x7F;

    return(NomAmsdos);
} /* idsk_getNomAmsdos */

// Disk management

void idsk_formatTrack(u8 *ImgDsk, idsk_Ent *Infos, int t, int MinSect, int NbSect)
{
    idsk_Track *tr = (idsk_Track *)&ImgDsk[ sizeof(idsk_Ent) + t * Infos->DataSize ];

    memset(&ImgDsk[ sizeof(idsk_Ent)
                    + sizeof(idsk_Track)
                    + (t * Infos->DataSize)
           ]
           , 0xE5
           , 0x200 * NbSect
           );
    strcpy(tr->ID, "Track-Info\r\n");
    tr->Track = (u8)t;
    tr->Head = 0;
    tr->SectSize = 2;
    tr->NbSect = (u8)NbSect;
    tr->Gap3 = 0x4E;
    tr->OctRemp = 0xE5;
    int ss = 0;
    //
    // Gestion "entrelacement" des secteurs
    //
    int s;

    for (s = 0; s < NbSect;) {
        tr->Sect[ s ].C = (u8)t;
        tr->Sect[ s ].H = 0;
        tr->Sect[ s ].R = (u8)(ss + MinSect);
        tr->Sect[ s ].N = 2;
        tr->Sect[ s ].SizeByte = 0x200;
        ss++;
        if (++s < NbSect) {
            tr->Sect[ s ].C = (u8)t;
            tr->Sect[ s ].H = 0;
            tr->Sect[ s ].R = (u8)(ss + MinSect + 4);
            tr->Sect[ s ].N = 2;
            tr->Sect[ s ].SizeByte = 0x200;
            s++;
        }
    }
} /* idsk_formatTrack */

int idsk_getMinSect(u8 *ImgDsk)
{
    int Sect = 0x100;
    idsk_Track *tr = (idsk_Track *)&ImgDsk[ sizeof(idsk_Ent) ];

    int s;

    for (s = 0; s < tr->NbSect; s++) {
        if (Sect > tr->Sect[ s ].R) Sect = tr->Sect[ s ].R;
    }

    return(Sect);
}

int idsk_getPosData(u8 *ImgDsk, int track, int sect, char SectPhysique)
{
    // Recherche position secteur
    int Pos = sizeof(idsk_Ent);
    idsk_Track *tr = (idsk_Track *)&ImgDsk[ Pos ];
    short SizeByte;

    int t;

    for (t = 0; t <= track; t++) {
        Pos += sizeof(idsk_Track);

        int s;
        for (s = 0; s < tr->NbSect; s++) {
            if (t == track) {
                if (  ( (tr->Sect[ s ].R == sect) && SectPhysique)
                      || ( (s == sect) && !SectPhysique)
                      ) break;
            }
            SizeByte = tr->Sect[ s ].SizeByte;
            if (SizeByte) Pos += SizeByte;
            else Pos += (128 << tr->Sect[ s ].N);
        }
    }
    return(Pos);
} /* idsk_getPosData */

idsk_StDirEntry * idsk_getInfoDirEntry(u8 *ImgDsk, int NumDir)
{
    static idsk_StDirEntry Dir;
    int MinSect = idsk_getMinSect(ImgDsk);
    int s = (NumDir >> 4) + MinSect;
    int t = (MinSect == 0x41 ? 2 : 0);

    if (MinSect == 1) t = 1;

    memcpy(&Dir
           , &ImgDsk[ ( (NumDir & 15) << 5) + idsk_getPosData(ImgDsk, t, s, 1) ]
           , sizeof(idsk_StDirEntry)
           );
    return(&Dir);
}

u8 * idsk_fillBitmap(u8 *ImgDsk)
{
    // int NbKo = 0;

    u8 *Bitmap = (u8 *)malloc(256);

    memset(Bitmap, 0, 256);
    Bitmap[ 0 ] = Bitmap[ 1 ] = 1;

    int i;

    for (i = 0; i < 64; i++) {
        idsk_StDirEntry *Dir = idsk_getInfoDirEntry(ImgDsk, i);
        if (Dir->User != USER_DELETED) {
            int j;
            for (j = 0; j < 16; j++) {
                int b = Dir->Blocks[ j ];
                if (b > 1 && (!Bitmap[ b ]) ) {
                    Bitmap[ b ] = 1;
                    // NbKo++;
                }
            }
        }
    }

    return(Bitmap);
} /* idsk_fillBitmap */

u8 * idsk_createNewDisk(void)
{
    int NbSect = 9;
    int NbTrack = 42;

    u8 *ImgDsk = (u8 *)malloc(0x80000);

    idsk_Ent *Infos = (idsk_Ent *)ImgDsk;

    strcpy(Infos->debut, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n");
    Infos->DataSize = (short)(sizeof(idsk_Track) + (0x200 * NbSect) );
    Infos->NbTracks = (u8)NbTrack;
    Infos->NbHeads = 1;

    int t;

    for (t = 0; t < NbTrack; t++) {
        idsk_formatTrack(ImgDsk, Infos, t, 0xC1, NbSect);
    }

    u8 *bmp = idsk_fillBitmap(ImgDsk);

    free(bmp);

    return ImgDsk;
} /* idsk_createNewDisk */

void idsk_fixEndianTrack(u8 *ImgDsk, idsk_Ent *Infos, int t, int NbSect)
{
    idsk_Track *tr;

    if (Infos->DataSize != 0) tr = (idsk_Track *)&ImgDsk[ sizeof(idsk_Ent) + t * Infos->DataSize ];
    else {
        int ExtendedDataSize = ImgDsk[ 0x34 + t ] * 256; // case of a extended dsk image
        tr = (idsk_Track *)&ImgDsk[ sizeof(idsk_Ent) + t * ExtendedDataSize ];
    }
    // int ss = 0;

    //
    // Gestion "entrelacement" des secteurs
    //
    int s;

    for (s = 0; s < NbSect;) {
        FIX_SHORT(tr->Sect[ s ].SizeByte);
        FIX_SHORT(tr->Sect[ s ].Un1);
        // ss++;
        if (++s < NbSect) {
            FIX_SHORT(tr->Sect[ s ].SizeByte);
            FIX_SHORT(tr->Sect[ s ].Un1);
            s++;
        }
    }
    FIX_SHORT(tr->Unused);
} /* idsk_fixEndianTrack */

void idsk_fixEndianDsk(u8 *ImgDsk, char littleToBig)
{
    idsk_Ent *Infos = (idsk_Ent *)ImgDsk;

    // std::cerr<< "FixEndianDsk() Infos->DataSize : " << Infos->DataSize <<std::endl;

    if (!littleToBig) {
        FIX_SHORT(Infos->DataSize);
    }
    int t;

    for (t = 0; t < Infos->NbTracks; t++) {
        idsk_fixEndianTrack(ImgDsk, Infos, t, 9);
    }
    if (littleToBig) {
        FIX_SHORT(Infos->DataSize);
    }
    u8 *bmp = idsk_fillBitmap(ImgDsk);

    free(bmp);
}

char * idsk_getDiskBuffer(u8 *ImgDsk,  u32 *length)
{
    idsk_Ent *Infos = (idsk_Ent *)ImgDsk;
    u32 Taille;

    if (!Infos->DataSize) Infos->DataSize = 0x100 + SECTSIZE * 9;
    Taille = Infos->NbTracks * Infos->DataSize + sizeof(*Infos);
    if (idsk_isBigEndian() ) {
        idsk_fixEndianDsk(ImgDsk, 1);        // Fix endianness for Big endian machines (PPC)
    }

    char *retBuf = (char *)malloc(Taille);

    if (retBuf == NULL) {
        return NULL;
    }
    memcpy(retBuf, ImgDsk, Taille);

    // in case of the same DSK image stay in memory
    if (idsk_isBigEndian() ) {
        idsk_fixEndianDsk(ImgDsk, 0);     // unFix endianness for Big endian machines (PPC)
    }

    *length = Taille;

    return retBuf;
} /* idsk_getDiskBuffer */

idsk_StAmsdos * idsk_stAmsdosEndian(idsk_StAmsdos *pEntete)
{
    FIX_SHORT(pEntete->Length);
    FIX_SHORT(pEntete->Adress);
    FIX_SHORT(pEntete->LogicalLength);
    FIX_SHORT(pEntete->EntryAdress);
    FIX_SHORT(pEntete->RealLength);
    FIX_SHORT(pEntete->CheckSum);
    return (pEntete);
}

idsk_StDirEntry * idsk_getNomDir(char *NomFic)
{
    static idsk_StDirEntry DirLoc;
    int i;

    memset(&DirLoc, 0, sizeof(DirLoc) );
    memset(DirLoc.Nom, ' ', 8);
    memset(DirLoc.Ext, ' ', 3);

    char *dot = strchr(NomFic, '.');

    if (dot != NULL) {
        *dot = 0;
        memcpy(DirLoc.Nom, NomFic, (strlen(NomFic) > 8) ? 8 : strlen(NomFic));
        dot++;
        memcpy(DirLoc.Ext, dot, (strlen(dot) > 3) ? 3 : strlen(dot));
    } else {
        memcpy(DirLoc.Nom, NomFic, (strlen(NomFic) > 8) ? 8 : strlen(NomFic));
    }
    for (i = 0; i < 8; i++) {
        DirLoc.Nom[ i ] = (u8)toupper(DirLoc.Nom[ i ]);
    }
    for (i = 0; i < 3; i++) {
        DirLoc.Ext[ i ] = (u8)toupper(DirLoc.Ext[ i ]);
    }

    return(&DirLoc);
} /* idsk_getNomDir */

int idsk_searchFreeFolder(u8 *ImgDsk)
{
    int i;

    for (i = 0; i < 64; i++) {
        idsk_StDirEntry *Dir = idsk_getInfoDirEntry(ImgDsk, i);
        if (Dir->User == USER_DELETED) return(i);
    }
    return(-1);
}

int idsk_searchFreeBlock(u8 *Bitmap, int MaxBloc)
{
    int i;

    for (i = 2; i < MaxBloc; i++) {
        if (!Bitmap[ i ]) {
            Bitmap[ i ] = 1;
            return(i);
        }
    }
    return(0);
}

unsigned char * idsk_readBloc(u8 *ImgDsk, int bloc)
{
    static unsigned char BufBloc[ SECTSIZE * 2 ];
    int track = ( bloc << 1 ) / 9;
    int sect = ( bloc << 1 ) % 9;
    int MinSect = idsk_getMinSect(ImgDsk);

    if ( MinSect == 0x41 )
        track += 2;
    else
    if ( MinSect == 0x01 )
        track++;

    int Pos = idsk_getPosData(ImgDsk, track, sect + MinSect, 1);

    memcpy(BufBloc, &ImgDsk[ Pos ], SECTSIZE);
    if ( ++sect > 8 ) {
        track++;
        sect = 0;
    }

    Pos = idsk_getPosData(ImgDsk, track, sect + MinSect, 1);
    memcpy(&BufBloc[ SECTSIZE ], &ImgDsk[ Pos ], SECTSIZE);
    return( BufBloc );
} /* idsk_readBloc */

void idsk_writeBloc(u8 *ImgDsk, int bloc, u8 BufBloc[ SECTSIZE * 2 ])
{
    int track = (bloc << 1) / 9;
    int sect = (bloc << 1) % 9;
    int MinSect = idsk_getMinSect(ImgDsk);

    if (MinSect == 0x41) {
        track += 2;
    } else {
        if (MinSect == 0x01) track++;
    }

    // Ajuste le nombre de pistes si d�passement capacit�
    idsk_Ent *Entete = (idsk_Ent *)ImgDsk;

    if (track > Entete->NbTracks - 1) {
        Entete->NbTracks = (u8)(track + 1);
        idsk_formatTrack(ImgDsk, Entete, track, MinSect, 9);
    }

    int Pos = idsk_getPosData(ImgDsk, track, sect + MinSect, 1);

    memcpy(&ImgDsk[ Pos ], BufBloc, SECTSIZE);
    if (++sect > 8) {
        track++;
        sect = 0;
    }
    Pos = idsk_getPosData(ImgDsk, track, sect + MinSect, 1);
    memcpy(&ImgDsk[ Pos ], &BufBloc[ SECTSIZE ], SECTSIZE);
} /* idsk_writeBloc */

void idsk_setInfoDirEntry(u8 *ImgDsk, int NumDir, idsk_StDirEntry *Dir)
{
    int MinSect = idsk_getMinSect(ImgDsk);
    int s = (NumDir >> 4) + MinSect;
    int t = (MinSect == 0x41 ? 2 : 0);

    if (MinSect == 1) {
        t = 1;
    }

    int i;

    for (i = 0; i < 16; i++) {
        memcpy(&ImgDsk[ ( (NumDir & 15) << 5) + idsk_getPosData(ImgDsk, t, s, 1) ]
               , Dir
               , sizeof(idsk_StDirEntry)
               );
    }
}

int idsk_copieFichier(u8 *ImgDsk, u8 *BufFile, char *NomFic, u32 TailleFic, u32 MaxBloc, int UserNumber, char System_file, char Read_only)
{
    u32 PosFile;
    int j, l, Bloc, NbPages = 0, PosDir, TaillePage;
    u8 *Bitmap = idsk_fillBitmap(ImgDsk);
    idsk_StDirEntry *DirLoc = idsk_getNomDir(NomFic);   // Construit l'entr�e pour mettre dans le catalogue

    for (PosFile = 0; PosFile < TailleFic;) {   // Pour chaque bloc du fichier
        PosDir = idsk_searchFreeFolder(ImgDsk);   // Trouve une entr�e libre dans le CAT
        if (PosDir != -1) {
            DirLoc->User = UserNumber;        // Remplit l'entr�e : User 0
            // http://www.cpm8680.com/cpmtools/cpm.htm
            if (System_file) DirLoc->Ext[1] |= 0x80;
            if (Read_only) DirLoc->Ext[0] |= 0x80;
            DirLoc->NumPage = (u8)NbPages++;      // Num�ro de l'entr�e dans le fichier
            TaillePage = (int)((TailleFic - PosFile + 127) >> 7);     // Taille de la page (on arrondit par le haut)
            if (TaillePage > 128) {           // Si y'a plus de 16k il faut plusieurs pages
                TaillePage = 128;
            }

            DirLoc->NbPages = (u8)TaillePage;
            l = (DirLoc->NbPages + 7) >> 3;   // Nombre de blocs=TaillePage/8 arrondi par le haut
            memset(DirLoc->Blocks, 0, 16);
            for (j = 0; j < l; j++) {   // Pour chaque bloc de la page
                Bloc = idsk_searchFreeBlock(Bitmap, MaxBloc);      // Met le fichier sur la disquette
                if (Bloc) {
                    DirLoc->Blocks[ j ] = (u8)Bloc;
                    idsk_writeBloc(ImgDsk, Bloc, &BufFile[ PosFile ]);
                    PosFile += 1024;    // Passe au bloc suivant
                } else {
                    free(Bitmap);
                    return(ERR_NO_BLOCK);
                }
            }
            idsk_setInfoDirEntry(ImgDsk, PosDir, DirLoc);
        } else {
            free(Bitmap);
            return(ERR_NO_DIRENTRY);
        }
    }

    free(Bitmap);
    return(ERR_NO_ERR);
} /* idsk_copieFichier */

// MyDsk.PutFileInDsk(*iter, AmsdosType, loadAdress, exeAdress, UserNumber, System_file, Read_only);

/// Import file in disk
/// @param ImgDsk disk image
/// @param Masque filename
char idsk_importFile(u8 *ImgDsk, u8 *buf, u32 len, char *Masque)
{
    static u8 Buff[ 0x20000 ];
    u32 Lg;
    char ret;

    int TypeModeImport = ASCII_MODE;  // ASCII_MODE or BINARY_MODE
    int loadAdress = 0;
    int exeAdress = 0;
    int UserNumber = 0;
    char System_file = 0;
    char Read_only = 0;

    if (len > 0x10080) {    // Max 64k allowed !
        return 0;
    }

    memcpy(Buff, buf, len);
    Lg = len;

    char AddHeader = 0;
    idsk_StAmsdos *e = (idsk_StAmsdos *)Buff;

    if (TypeModeImport == ASCII_MODE) {
        int i;
        for (i = 0; i < 0x20000; i++) { // last ascii char
            if (Buff[i] > 136) {
                Buff[i] = '?'; // replace by unknown char
            }
        }
    }

    char *cFileName = idsk_getNomAmsdos(Masque);

    char IsAmsdos = idsk_checkAmsdos(Buff);

    if (!IsAmsdos) { // Add default AMSDOS Header
        e = idsk_creeEnteteAmsdos(cFileName, (u16)Lg);
        if (loadAdress != 0) {
            e->Adress = (u16)loadAdress;
            TypeModeImport = BINARY_MODE;
        }
        if (exeAdress != 0) {
            e->EntryAdress = (u16)exeAdress;
            TypeModeImport = BINARY_MODE;
        }
        // Il faut recalculer le checksum en comptant es adresses !
        idsk_setChecksum(e);
        // fix the endianness of the input file
        if (idsk_isBigEndian() ) e = idsk_stAmsdosEndian(e);
    } else {
//        cout << "Le fichier a d�j� une en-t�te\n";
    }
    //
    // En fonction du mode d'importation...
    //
    switch (TypeModeImport) {
        case ASCII_MODE:    // Import ASCII
            if (IsAmsdos) {
                // Supprmier en-tete si elle existe
                memcpy(Buff, &Buff[ sizeof(idsk_StAmsdos) ], Lg - sizeof(idsk_StAmsdos));
                Lg -= sizeof(idsk_StAmsdos);
            }
            break;

        case BINARY_MODE:  // Import Binary
            if (!IsAmsdos) { // Add Header
                AddHeader = 1;
            }
            break;
    }

    //
    // Si fichier ok pour etre import
    //
    if (AddHeader) {
        // Ajoute l'en-tete amsdos si necessaire

        memmove(&Buff[ sizeof(idsk_StAmsdos) ], Buff, Lg);
        memcpy(Buff, e, sizeof(idsk_StAmsdos) );
        Lg += sizeof(idsk_StAmsdos);
    }

    // if (MODE_BINAIRE) ClearAmsdos(Buff); //Remplace les octets inutilis�s par des 0 dans l'en-t�te

    if (idsk_copieFichier(ImgDsk, Buff, cFileName, Lg, 256, UserNumber, System_file, Read_only) != ERR_NO_ERR) {
        ret = 0;
    } else {
        ret = 1;
    }

    free(cFileName);

    return ret;
} /* idsk_importFile */

char idsk_onViewFic(u8 *ImgDsk, int nItem,  u8 *BufFile, u32 TailleFic)
{
    int LongFic = 0;

    char NomFic[ 16 ];
    char current[ 16 ];
    int i = nItem;
    char FirstBlock = 1;
    idsk_StDirEntry TabDir[ 64 ];

    for ( int j = 0; j < 64; j++ ) {
        memcpy(&TabDir[ j ], idsk_getInfoDirEntry(ImgDsk, j), sizeof( idsk_StDirEntry ));
    }

    memset(NomFic, 0, sizeof( NomFic ) );
    strcpy(NomFic, idsk_getNomAmsdos(TabDir[ i ].Nom));

    int lMax = TailleFic;

    TailleFic = 0;

    do{
        // Longueur du fichier
        int l = ( TabDir[ i ].NbPages + 7 ) >> 3;
        for ( int j = 0; j < l; j++ ) {
            int TailleBloc = 1024;
            unsigned char *p = idsk_readBloc(ImgDsk, TabDir[ i ].Blocks[ j ]);
            if ( FirstBlock ) {
                if ( idsk_checkAmsdos(p) ) {
                    TailleFic = p[ 0x18 + 1 ] * 256 + p[ 0x18 ];
                    // u16 AdresseCharg = p[ 0x15 + 1 ] * 256 + p[ 0x15 ];
                    // u16 AdresseExec = p[ 0x1a + 1 ] * 256 + p[ 0x1a ];

                    TailleBloc -= sizeof( idsk_StAmsdos );
                    memcpy(p, &p[ 0x80 ], TailleBloc);
                }
                FirstBlock = 0;

            }

            // printf("nboctets: %d,%d => %d\n", lMax, TailleBloc, MIN(lMax, TailleBloc));

            int NbOctets = MIN(lMax, TailleBloc);
            if ( NbOctets > 0 ) {
                memcpy(&BufFile[ LongFic ], p, NbOctets);
                LongFic += NbOctets;
            }
            lMax -= 1024;
        }
        memset(current, 0, sizeof( current ) );
        i++;
        strcpy(current, idsk_getNomAmsdos(TabDir[ i ].Nom));
        if ( i > 64 ) return 0;
    }while ( !strncmp(NomFic, current, MAX(strlen(current), strlen(NomFic) ) ) );

    // printf("Taille %d,%d\n", LongFic, TailleFic);

    if ( TailleFic == 0 ) {
        TailleFic = LongFic;
    }

    return 1;
} /* DSK::OnViewFic */