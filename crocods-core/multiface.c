#include  "multiface.h"

#include  "crtc.h"
#include  "vga.h"
#include  "ppi.h"


#ifdef USE_MULTIFACE
/********************************************************* !NAME! **************
* Nom : MultifaceEnable
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Multiface activ�e ou non
*
********************************************************** !0! ****************/
int MultifaceEnable = 0;


/********************************************************* !NAME! **************
* Nom : MULTIFACE_ROM
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Variables Globales
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Rom/ram multiface
*
********************************************************** !0! ****************/
UBYTE MULTIFACE_ROM[ 0x4000 ];


/********************************************************* !NAME! **************
* Nom : InitMultiface
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation de la multiface : lecture de la rom multiface
*
* R�sultat    : TRUE si ok, FALSE sinon.
*
* Variables globales modifi�es : MULTIFACE_ROM
*
********************************************************** !0! ****************/
BOOL InitMultiface( void )
{
    FILE * fp = fopen( LocRomMulti, "rb" );
    if ( fp )
        {
        fread( MULTIFACE_ROM, 0x2000, 1, fp );
        fclose( fp );
        return( TRUE );
        }
    return( FALSE );
}


/********************************************************* !NAME! **************
* Nom : MultifaceWriteIO
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Interception des �critures ports pour m�morisation par la
*               multiface
*
* R�sultat    : /
*
* Variables globales modifi�es : MULTIFACE_ROM
*
********************************************************** !0! ****************/
void MultifaceWriteIO( int Port, int Data )
{
    UBYTE PortHighByte = ( UBYTE )( Port >> 8 );

    if ( ( Port & 0xFFFD ) == 0xFEE8 )
        {
        MultifaceEnable = ! ( Port & 2 );
        WriteROM( RomExt );
        }
    if ( PortHighByte == ( UBYTE )0x7F )
        {
        if ( ( Data & 0xC0 ) == 0x40 )
            {
            int PenIndex = MULTIFACE_ROM[ 0x3FCF ];
            MULTIFACE_ROM[ 0x3F90 | ( ( PenIndex & 0x10 ) << 2 ) | ( PenIndex & 0x0F ) ] = ( UBYTE )Data;
            }
        else
            MULTIFACE_ROM[ 0x3FCF | ( ( Data & 0xC0 ) >> 2 ) ] = ( UBYTE )Data;
        }

    if ( PortHighByte == ( UBYTE )0xBC )
        MULTIFACE_ROM[ 0x3CFF ] = ( UBYTE )Data;

    if ( PortHighByte == ( UBYTE )0xBD )
        {
        int CRTCRegIndex = MULTIFACE_ROM[ 0x3CFF ];
        MULTIFACE_ROM[ ( 0x3DB0 + ( CRTCRegIndex & 0x0F ) ) ] = ( UBYTE )Data;
        }

    if ( PortHighByte == ( UBYTE )0xF7 )
        MULTIFACE_ROM[ 0x37FF ] = ( UBYTE )Data;

    if ( PortHighByte == ( UBYTE )0xDF )
        MULTIFACE_ROM[ 0x3AAC ] = ( UBYTE )Data;
}
#endif

