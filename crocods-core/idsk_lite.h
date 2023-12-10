


#define USER_DELETED 0xE5

#pragma pack(1) // evite le padding des structures qui sont utilisées dans des memcpy par la suite

//
// Structure d'une entree AMSDOS
//
typedef struct {
    u8 UserNumber;     // 00 User
    u8 FileName[15];   // 01-0F Nom + extension
    u8 BlockNum;       // 10    Numéro du bloc (disquette)
    u8 LastBlock;      // 11    Flag "dernier bloc" bloc (disquette)
    u8 FileType;       // 12    Type de fichier
    u16 Length;        // 13-14 Longueur
    u16 Adress;        // 15-16 Adresse
    u8 FirstBlock;     // 17    Flag premier bloc de fichier (disquette)
    u16 LogicalLength; // 18-19 Longueur logique
    u16 EntryAdress;   // 1A-1B Point d'entree
    u8 Unused[0x24];
    u16 RealLength;    // 40-42 Longueur reelle
    u8 BigLength;      //       Longueur reelle (3 octets)
    u16 CheckSum;      // 43-44 CheckSum Amsdos
    u8 Unused2[0x3B];
} idsk_StAmsdos;

typedef struct {
    char debut[0x30];  // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
    u8 NbTracks;
    u8 NbHeads;
    u16 DataSize;      // 0x1300 = 256 + ( 512 * nbsecteurs )
    u8 Unused[0xCC];
} idsk_Ent;

typedef struct {
    u8 C;                    // track
    u8 H;                    // head
    u8 R;                    // sect
    u8 N;                    // size
    short Un1;
    short SizeByte;          // Taille secteur en octets
} idsk_Sect;

typedef struct {
    char ID[0x10];           // "Track-Info\r\n"
    u8 Track;
    u8 Head;
    short Unused;
    u8 SectSize;             // 2
    u8 NbSect;               // 9
    u8 Gap3;                 // 0x4E
    u8 OctRemp;              // 0xE5
    idsk_Sect Sect[29];
} idsk_Track;

typedef struct {
    u8 User;                 // 00
    char Nom[8];             // 01-08
    char Ext[3];             // 09-0B
    u8 NumPage;              // 0C
    u8 Unused[2];            // 0D-0E
    u8 NbPages;              // 0F
    u8 Blocks[16];           // 10-1F
} idsk_StDirEntry;

#pragma pack()


u8 * idsk_createNewDisk(void);
char idsk_importFile(u8 *ImgDsk, u8 *buf, u32 len, char *file);
char * idsk_getDiskBuffer(u8 *ImgDsk, u32 *length);
char idsk_onViewFic(u8 *ImgDsk, int nItem,  u8 *BufFile, u32 TailleFic);
idsk_StDirEntry * idsk_getInfoDirEntry(u8 *ImgDsk, int NumDir);
unsigned char * idsk_readBloc(u8 *ImgDsk, int bloc);
char idsk_checkAmsdos(u8 *Buf);
