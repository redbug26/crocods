#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #define STANDALONE

// Todo:
// Add label for variable (ld hl,#3333)
// Make test

#ifdef STANDALONE
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
#else
#include "crocods.h"
#include "plateform.h"
#endif

#define CODESIZE 65535
#define DEBUGGER 0

u8 Opcodes[CODESIZE];
u8 OpcodesFlags[CODESIZE];
char *OpcodesLabel[CODESIZE];

char *vectorLabel[CODESIZE];
char vectorUsed[CODESIZE];

char *instructions[CODESIZE];

u16 codeBeg, codeEnd;

enum {
    Opcode,
    Operand,
    Data
} DataType;



// Länge eines Opcodes in Bytes ermitteln
u8 OpcodeLen(u32 p)
{
    u8 len = 1;

    switch (Opcodes[p]) {// Opcode
        case 0x06: // LD B,n
        case 0x0E: // LD C,n
        case 0x10: // DJNZ e
        case 0x16: // LD D,n
        case 0x18: // JR e
        case 0x1E: // LD E,n
        case 0x20: // JR NZ,e
        case 0x26: // LD H,n
        case 0x28: // JR Z,e
        case 0x2E: // LD L,n
        case 0x30: // JR NC,e
        case 0x36: // LD (HL),n
        case 0x38: // JR C,e
        case 0x3E: // LD A,n
        case 0xC6: // ADD A,n
        case 0xCE: // ADC A,n
        case 0xD3: // OUT (n),A
        case 0xD6: // SUB n
        case 0xDB: // IN A,(n)
        case 0xDE: // SBC A,n
        case 0xE6: // AND n
        case 0xEE: // XOR n
        case 0xF6: // OR n
        case 0xFE: // CP n

        case 0xCB: // Shift-,Rotate-,Bit-Befehle
            len = 2;
            break;
        case 0x01: // LD BC,nn'
        case 0x11: // LD DE,nn'
        case 0x21: // LD HL,nn'
        case 0x22: // LD (nn'),HL
        case 0x2A: // LD HL,(nn')
        case 0x31: // LD SP,(nn')
        case 0x32: // LD (nn'),A
        case 0x3A: // LD A,(nn')
        case 0xC2: // JP NZ,nn'
        case 0xC3: // JP nn'
        case 0xC4: // CALL NZ,nn'
        case 0xCA: // JP Z,nn'
        case 0xCC: // CALL Z,nn'
        case 0xCD: // CALL nn'
        case 0xD2: // JP NC,nn'
        case 0xD4: // CALL NC,nn'
        case 0xDA: // JP C,nn'
        case 0xDC: // CALL C,nn'
        case 0xE2: // JP PO,nn'
        case 0xE4: // CALL PO,nn'
        case 0xEA: // JP PE,nn'
        case 0xEC: // CALL PE,nn'
        case 0xF2: // JP P,nn'
        case 0xF4: // CALL P,nn'
        case 0xFA: // JP M,nn'
        case 0xFC: // CALL M,nn'
            len = 3;
            break;
        case 0xDD: len = 2;
            switch (Opcodes[p + 1]) { // 2.Teil des Opcodes
                case 0x24: // INC HX
                    len = 2;
                    break;
                case 0x2e: // LD LX,n
                case 0x34: // INC (IX+d)
                case 0x35: // DEC (IX+d)
                case 0x46: // LD B,(IX+d)
                case 0x4E: // LD C,(IX+d)
                case 0x56: // LD D,(IX+d)
                case 0x5E: // LD E,(IX+d)
                case 0x66: // LD H,(IX+d)
                case 0x6E: // LD L,(IX+d)
                case 0x70: // LD (IX+d),B
                case 0x71: // LD (IX+d),C
                case 0x72: // LD (IX+d),D
                case 0x73: // LD (IX+d),E
                case 0x74: // LD (IX+d),H
                case 0x75: // LD (IX+d),L
                case 0x77: // LD (IX+d),A
                case 0x7E: // LD A,(IX+d)
                case 0x86: // ADD A,(IX+d)
                case 0x8E: // ADC A,(IX+d)
                case 0x96: // SUB A,(IX+d)
                case 0x9E: // SBC A,(IX+d)
                case 0xA6: // AND (IX+d)
                case 0xAE: // XOR (IX+d)
                case 0xB6: // OR (IX+d)
                case 0xBE: // CP (IX+d)
                    len = 3;
                    break;
                case 0x21: // LD IX,nn'
                case 0x22: // LD (nn'),IX
                case 0x2A: // LD IX,(nn')
                case 0x36: // LD (IX+d),n
                case 0xCB: // Rotation (IX+d)
                    len = 4;
                    break;
            } /* switch */
            break;
        case 0xED: len = 2;
            switch (Opcodes[p + 1]) { // 2.Teil des Opcodes
                case 0x43: // LD (nn'),BC
                case 0x4B: // LD BC,(nn')
                case 0x53: // LD (nn'),DE
                case 0x5B: // LD DE,(nn')
                case 0x73: // LD (nn'),SP
                case 0x7B: // LD SP,(nn')
                    len = 4;
                    break;
            }
            break;
        case 0xFD: len = 2;
            switch (Opcodes[p + 1]) { // 2.Teil des Opcodes
                case 0x24: // INC HY
                    len = 2;
                    break;
                case 0x2e: // LD LY,n
                case 0x34: // INC (IY+d)
                case 0x35: // DEC (IY+d)
                case 0x46: // LD B,(IY+d)
                case 0x4E: // LD C,(IY+d)
                case 0x56: // LD D,(IY+d)
                case 0x5E: // LD E,(IY+d)
                case 0x66: // LD H,(IY+d)
                case 0x6E: // LD L,(IY+d)
                case 0x70: // LD (IY+d),B
                case 0x71: // LD (IY+d),C
                case 0x72: // LD (IY+d),D
                case 0x73: // LD (IY+d),E
                case 0x74: // LD (IY+d),H
                case 0x75: // LD (IY+d),L
                case 0x77: // LD (IY+d),A
                case 0x7E: // LD A,(IY+d)
                case 0x86: // ADD A,(IY+d)
                case 0x8E: // ADC A,(IY+d)
                case 0x96: // SUB A,(IY+d)
                case 0x9E: // SBC A,(IY+d)
                case 0xA6: // AND (IY+d)
                case 0xAE: // XOR (IY+d)
                case 0xB6: // OR (IY+d)
                case 0xBE: // CP (IY+d)
                    len = 3;
                    break;
                case 0x21: // LD IY,nn'
                case 0x22: // LD (nn'),IY
                case 0x2A: // LD IY,(nn')
                case 0x36: // LD (IY+d),n
                case 0xCB: // Rotation,Bitop (IY+d)
                    len = 4;
                    break;
            } /* switch */
            break;
    } /* switch */
    return(len);
} /* OpcodeLen */

void ParseOpcodes(u32 adr)
{
    u16 i, len;
    u32 next;
    u8 label = 1;

    do {
        if (label) {
            OpcodesFlags[adr] |= 0x10;
        }

        if ((OpcodesFlags[adr] & 0x0F) == Opcode) {
            break;
        }

        if ((OpcodesFlags[adr] & 0x0F) == Operand) {
            printf("; Illegal opcode?!?");
            return;
        }

        len = OpcodeLen(adr); // Länge vom Opcode ermitteln
        for (i = 0; i < len; i++) {
            OpcodesFlags[adr + i] = Operand; // Opcode eintragen
        }

        OpcodesFlags[adr] = Opcode; // Start des Opcodes markieren
        if (label) { // ein Label setzen?
            OpcodesFlags[adr] |= 0x10; // Label setzen
            label = 0; // Label-Flag zurücksetzen
        }

        next = adr + len; // Ptr auf den Folgeopcode
        switch (Opcodes[adr]) { // Opcode holen
            case 0xCA: // JP c,????
            case 0xC2:
            case 0xDA:
            case 0xD2:
            case 0xEA:
            case 0xE2:
            case 0xFA:
            case 0xF2:
                ParseOpcodes((Opcodes[adr + 2] << 8) + Opcodes[adr + 1]);
                break;
            case 0x28: // JR c,??
            case 0x20:
            case 0x38:
            case 0x30:
                ParseOpcodes(adr + 2 + (s8)Opcodes[adr + 1]);
                break;
            case 0xCC: // CALL c,????
            case 0xC4:
            case 0xDC:
            case 0xD4:
            case 0xEC:
            case 0xE4:
            case 0xFC:
            case 0xF4:
                ParseOpcodes((Opcodes[adr + 2] << 8) + Opcodes[adr + 1]);
                break;
            case 0xC8: // RET c
            case 0xC0:
            case 0xD8:
            case 0xD0:
            case 0xE8:
            case 0xE0:
            case 0xF8:
            case 0xF0:
                break;
            case 0xC7: // RST 0
            case 0xCF: // RST 8
            case 0xD7: // RST 10
            case 0xDF: // RST 18
            case 0xE7: // RST 20
            case 0xEF: // RST 28
            case 0xF7: // RST 30
            case 0xFF: // RST 38
                ParseOpcodes(Opcodes[adr] & 0x38);
                break;
            case 0x10: // DJNZ ??
                ParseOpcodes(adr + 2 + (s8)Opcodes[adr + 1]);
                break;
            case 0xC3: // JP ????
                next = (Opcodes[adr + 2] << 8) + Opcodes[adr + 1];
                label = 1;
                break;
            case 0x18: // JR ??
                next = adr + 2 + (s8)Opcodes[adr + 1];
                label = 1;
                break;
            case 0xCD: // CALL ????
                ParseOpcodes((Opcodes[adr + 2] << 8) + Opcodes[adr + 1]);
                break;
            case 0xC9: // RET
                return;
            case 0xE9:
#if DEBUGGER
                printf("\pJP (HL) gefunden");  // JP (HL)
#endif
                break;
            case 0xDD:
#if DEBUGGER
                if (Opcodes[adr + 1] == 0xE9) { // JP (IX)
                    printf("\pJP (IX) gefunden");
                }
#endif
                break;
            case 0xFD:
#if DEBUGGER
                if (Opcodes[adr + 1] == 0xE9) { // JP (IY)
                    printf("\pJP (IY) gefunden");
                }
#endif
                break;
            case 0xED:
                if (Opcodes[adr + 1] == 0x4D) { // RTI
                    return;
                } else if (Opcodes[adr + 1] == 0x45) { // RETN
                    return;
                }
                break;
        } /* switch */
        adr = next;
    } while (1);
} /* ParseOpcodes */

void labelForAddr(char *s, u16 adr)
{
    if (vectorLabel[adr] != NULL) {
        vectorUsed[adr] = 1;
        strcpy(s, vectorLabel[adr]);
        return;
    }

    if (((OpcodesFlags[adr] & 0x10) == 0x10) && (adr >= codeBeg) && (adr < codeEnd)) {
        if (OpcodesLabel[adr] != NULL) {
            strcpy(s, OpcodesLabel[adr]);
        } else {
            sprintf(s, "L%4.4X", adr);
        }
    } else {
        sprintf(s, "#%4.4X", adr);
    }
}

// Disassemblieren
void Disassemble(u16 adr, char *s)
{
    u8 a = Opcodes[adr];
    u8 d = (a >> 3) & 7;
    u8 e = a & 7;
    static char *reg[8] = {"B", "C", "D", "E", "H", "L", "(HL)", "A"};
    static char *dreg[4] = {"BC", "DE", "HL", "SP"};
    static char *cond[8] = {"NZ", "Z", "NC", "C", "PO", "PE", "P", "M"};
    static char *arith[8] = {"ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP "};
    char stemp[80];
    char ireg[3];

    *s = 0;

    switch (a & 0xC0) {
        case 0x00:
            switch (e) {
                case 0x00:
                    switch (d) {
                        case 0x00:
                            strcpy(s, "NOP");
                            break;
                        case 0x01:
                            strcpy(s, "EX AF,AF'");
                            break;
                        case 0x02:
                            strcpy(s, "DJNZ ");
                            labelForAddr(stemp, adr + 2 + (s8)Opcodes[adr + 1]);
                            strcat(s, stemp);
                            break;
                        case 0x03:
                            strcpy(s, "JR ");
                            labelForAddr(stemp, adr + 2 + (s8)Opcodes[adr + 1]);
                            strcat(s, stemp);
                            break;
                        default:
                            strcpy(s, "JR ");
                            strcat(s, cond[d & 3]);
                            strcat(s, ",");
                            labelForAddr(stemp, adr + 2 + (s8)Opcodes[adr + 1]);
                            strcat(s, stemp);
                            break;
                    } /* switch */
                    break;
                case 0x01:
                    if (a & 0x08) {
                        strcpy(s, "ADD HL,");
                        strcat(s, dreg[d >> 1]);
                    } else {
                        strcpy(s, "LD ");
                        strcat(s, dreg[d >> 1]);
                        strcat(s, ",");
                        sprintf(stemp, "#%4.4X", Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                        strcat(s, stemp);
                    }
                    break;
                case 0x02:
                    switch (d) {
                        case 0x00:
                            strcpy(s, "LD (BC),A");
                            break;
                        case 0x01:
                            strcpy(s, "LD A,(BC)");
                            break;
                        case 0x02:
                            strcpy(s, "LD (DE),A");
                            break;
                        case 0x03:
                            strcpy(s, "LD A,(DE)");
                            break;
                        case 0x04:
                            strcpy(s, "LD (");
                            labelForAddr(stemp, Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                            strcat(s, stemp);
                            strcat(s, "),HL");
                            break;
                        case 0x05:
                            strcpy(s, "LD HL,(");
                            labelForAddr(stemp, Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                            strcat(s, stemp);
                            strcat(s, ")");
                            break;
                        case 0x06:
                            strcpy(s, "LD (");
                            labelForAddr(stemp, Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                            strcat(s, stemp);
                            strcat(s, "),A");
                            break;
                        case 0x07:
                            strcpy(s, "LD A,(");
                            labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                            strcat(s, stemp);
                            strcat(s, ")");
                            break;
                    } /* switch */
                    break;
                case 0x03:
                    if (a & 0x08)
                        strcpy(s, "DEC ");
                    else
                        strcpy(s, "INC ");
                    strcat(s, dreg[d >> 1]);
                    break;
                case 0x04:
                    strcpy(s, "INC ");
                    strcat(s, reg[d]);
                    break;
                case 0x05:
                    strcpy(s, "DEC ");
                    strcat(s, reg[d]);
                    break;
                case 0x06: // LD d,n
                    strcpy(s, "LD ");
                    strcat(s, reg[d]);
                    strcat(s, ",");
                    sprintf(stemp, "#%2.2X", Opcodes[adr + 1]);
                    strcat(s, stemp);
                    break;
                case 0x07:
                {
                    static char *str[8] = {"RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF"};
                    strcpy(s, str[d]);
                }
                break;
            } /* switch */
            break;
        case 0x40: // LD d,s
            if (d == e) {
                strcpy(s, "HALT");
            } else {
                strcpy(s, "LD ");
                strcat(s, reg[d]);
                strcat(s, ",");
                strcat(s, reg[e]);
            }
            break;
        case 0x80:
            strcpy(s, arith[d]);
            strcat(s, reg[e]);
            break;
        case 0xC0:
            switch (e) {
                case 0x00:
                    strcpy(s, "RET ");
                    strcat(s, cond[d]);
                    break;
                case 0x01:
                    if (d & 1) {
                        switch (d >> 1) {
                            case 0x00:
                                strcpy(s, "RET");
                                break;
                            case 0x01:
                                strcpy(s, "EXX");
                                break;
                            case 0x02:
                                strcpy(s, "JP (HL)");
                                break;
                            case 0x03:
                                strcpy(s, "LD SP,HL");
                                break;
                        }
                    } else {
                        strcpy(s, "POP ");
                        if ((d >> 1) == 3)
                            strcat(s, "AF");
                        else
                            strcat(s, dreg[d >> 1]);
                    }
                    break;
                case 0x02:
                    strcpy(s, "JP ");
                    strcat(s, cond[d]);
                    strcat(s, ",");
                    labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                    strcat(s, stemp);
                    break;
                case 0x03:
                    switch (d) {
                        case 0x00:
                            strcpy(s, "JP ");
                            labelForAddr(stemp, Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                            strcat(s, stemp);
                            break;
                        case 0x01: // 0xCB
                            a = Opcodes[++adr]; // Erweiterungsopcode holen
                            d = (a >> 3) & 7;
                            e = a & 7;
                            stemp[1] = 0; // temp.String = 1 Zeichen
                            switch (a & 0xC0) {
                                case 0x00:
                                {
                                    static char *str[8] = {"RLC", "RRC", "RL", "RR", "SLA", "SRA", "???", "SRL"};
                                    strcpy(s, str[d]);
                                }
                                    strcat(s, " ");
                                    strcat(s, reg[e]);
                                    break;
                                case 0x40:
                                    strcpy(s, "BIT ");
                                    stemp[0] = d + '0'; strcat(s, stemp);
                                    strcat(s, ",");
                                    strcat(s, reg[e]);
                                    break;
                                case 0x80:
                                    strcpy(s, "RES ");
                                    stemp[0] = d + '0'; strcat(s, stemp);
                                    strcat(s, ",");
                                    strcat(s, reg[e]);
                                    break;
                                case 0xC0:
                                    strcpy(s, "SET ");
                                    stemp[0] = d + '0'; strcat(s, stemp);
                                    strcat(s, ",");
                                    strcat(s, reg[e]);
                                    break;
                            } /* switch */
                            break;
                        case 0x02:
                            strcpy(s, "OUT (");
                            sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                            strcat(s, "),A");
                            break;
                        case 0x03:
                            strcpy(s, "IN A,(");
                            sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                            strcat(s, ")");
                            break;
                        case 0x04:
                            strcpy(s, "EX (SP),HL");
                            break;
                        case 0x05:
                            strcpy(s, "EX DE,HL");
                            break;
                        case 0x06:
                            strcpy(s, "DI");
                            break;
                        case 0x07:
                            strcpy(s, "EI");
                            break;
                    } /* switch */
                    break;
                case 0x04:
                    strcpy(s, "CALL ");
                    strcat(s, cond[d]);
                    strcat(s, ",");
                    labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                    strcat(s, stemp);
                    break;
                case 0x05:
                    if (d & 1) {
                        switch (d >> 1) {
                            case 0x00:
                                strcpy(s, "CALL ");
                                labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                                strcat(s, stemp);
                                break;
                            case 0x02: // 0xED
                                a = Opcodes[++adr]; // Erweiterungsopcode holen
                                d = (a >> 3) & 7;
                                e = a & 7;
                                switch (a & 0xC0) {
                                    case 0x40:
                                        switch (e) {
                                            case 0x00:
                                                strcpy(s, "IN ");
                                                strcat(s, reg[d]);
                                                strcat(s, ",(C)");
                                                break;
                                            case 0x01:
                                                strcpy(s, "OUT (C),");
                                                if (a == 0x71) {
                                                    strcat(s, "0");
                                                } else {
                                                    strcat(s, reg[d]);
                                                }
                                                // sprintf(stemp, "[%02X]", a);
                                                // strcat(s, stemp);
                                                break;
                                            case 0x02:
                                                if (d & 1)
                                                    strcpy(s, "ADC");
                                                else
                                                    strcpy(s, "SBC");
                                                strcat(s, " HL,");
                                                strcat(s, dreg[d >> 1]);
                                                break;
                                            case 0x03:
                                                if (d & 1) {
                                                    strcpy(s, "LD ");
                                                    strcat(s, dreg[d >> 1]);
                                                    strcat(s, ",(");
                                                    labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8)); strcat(s, stemp);
                                                    strcat(s, ")");
                                                } else {
                                                    strcpy(s, "LD (");
                                                    labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8)); strcat(s, stemp);
                                                    strcat(s, "),");
                                                    strcat(s, dreg[d >> 1]);
                                                }
                                                break;
                                            case 0x04:
                                            {
                                                static char *str[8] = {"NEG", "???", "???", "???", "???", "???", "???", "???"};
                                                strcpy(s, str[d]);
                                            }
                                            break;
                                            case 0x05:
                                            {
                                                static char *str[8] = {"RETN", "RETI", "???", "???", "???", "???", "???", "???"};
                                                strcpy(s, str[d]);
                                            }
                                            break;
                                            case 0x06:
                                                strcpy(s, "IM ");
                                                stemp[0] = d + '0' - 1; stemp[1] = 0;
                                                strcat(s, stemp);
                                                break;
                                            case 0x07:
                                            {
                                                static char *str[8] = {"LD I,A", "???", "LD A,I", "???", "RRD", "RLD", "???", "???"};
                                                strcpy(s, str[d]);
                                            }
                                            break;
                                            default:
                                                strcpy(s, "?");
                                                break;

                                        } /* switch */
                                        break;
                                    case 0x80:
                                    {
                                        static char *str[32] = {"LDI",  "CPI",  "INI",  "OUTI",  "???",  "???",  "???",  "???",
                                                                "LDD",  "CPD",  "IND",  "OUTD",  "???",  "???",  "???",  "???",
                                                                "LDIR", "CPIR", "INIR", "OTIR",  "???",  "???",  "???",  "???",
                                                                "LDDR", "CPDR", "INDR", "OTDR",  "???",  "???",  "???",  "???"};
                                        strcpy(s, str[a & 0x1F]);
                                    }
                                    break;
                                    default:
                                        sprintf(s, "DB #%02X,#%02X", 0xED, a);
                                        break;

                                } /* switch */
                                break;
                            default: // 0x01 (0xDD) = IX, 0x03 (0xFD) = IY
                                strcpy(ireg, (a & 0x20) ? "IY" : "IX");
                                a = Opcodes[++adr]; // Erweiterungsopcode holen
                                switch (a) {
                                    case 0x09:
                                        strcpy(s, "ADD ");
                                        strcat(s, ireg);
                                        strcat(s, ",BC");
                                        break;
                                    case 0x19:
                                        strcpy(s, "ADD ");
                                        strcat(s, ireg);
                                        strcat(s, ",DE");
                                        break;
                                    case 0x21:
                                        strcpy(s, "LD ");
                                        strcat(s, ireg);
                                        strcat(s, ",");
                                        sprintf(stemp, "#%4.4X", Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                                        strcat(s, stemp);
                                        break;
                                    case 0x22:
                                        strcpy(s, "LD (");
                                        labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                                        strcat(s, stemp);
                                        strcat(s, "),");
                                        strcat(s, ireg);
                                        break;
                                    case 0x23:
                                        strcpy(s, "INC ");
                                        strcat(s, ireg);
                                        break;
                                    case 0x24:
                                        sprintf(s, "INC H%c,", ireg[1]);
                                        break;
                                    case 0x29:
                                        strcpy(s, "ADD ");
                                        strcat(s, ireg);
                                        strcat(s, ",");
                                        strcat(s, ireg);
                                        break;
                                    case 0x2A:
                                        strcpy(s, "LD ");
                                        strcat(s, ireg);
                                        strcat(s, ",(");
                                        labelForAddr(stemp,  Opcodes[adr + 1] + (Opcodes[adr + 2] << 8));
                                        strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x2B:
                                        strcpy(s, "DEC ");
                                        strcat(s, ireg);
                                        break;
                                    case 0x2E:
                                        sprintf(s, "LD L%c,", ireg[1]);
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]);
                                        strcat(s, stemp);
                                        break;
                                    case 0x34:
                                        strcpy(s, "INC (");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        labelForAddr(stemp, Opcodes[adr + 1]);
                                        strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x35:
                                        strcpy(s, "DEC (");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]);
                                        strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x36:
                                        strcpy(s, "LD (");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]);
                                        strcat(s, stemp);
                                        strcat(s, "),");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 2]);
                                        strcat(s, stemp);
                                        break;
                                    case 0x39:
                                        strcpy(s, "ADD ");
                                        strcat(s, ireg);
                                        strcat(s, ",SP");
                                        break;
                                    case 0x46:
                                    case 0x4E:
                                    case 0x56:
                                    case 0x5E:
                                    case 0x66:
                                    case 0x6E:
                                        strcpy(s, "LD ");
                                        strcat(s, reg[(a >> 3) & 7]);
                                        strcat(s, ",(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x70:
                                    case 0x71:
                                    case 0x72:
                                    case 0x73:
                                    case 0x74:
                                    case 0x75:
                                    case 0x77:
                                        strcpy(s, "LD (");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, "),");
                                        strcat(s, reg[a & 7]);
                                        break;
                                    case 0x7E:
                                        strcpy(s, "LD A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x86:
                                        strcpy(s, "ADD A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x8E:
                                        strcpy(s, "ADC A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x96:
                                        strcpy(s, "SUB (");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0x9E:
                                        strcpy(s, "SBC A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0xA6:
                                        strcpy(s, "AND A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0xAE:
                                        strcpy(s, "XOR A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0xB6:
                                        strcpy(s, "OR A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0xBE:
                                        strcpy(s, "CP A,(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, ")");
                                        break;
                                    case 0xE1:
                                        strcpy(s, "POP ");
                                        strcat(s, ireg);
                                        break;
                                    case 0xE3:
                                        strcpy(s, "EX (SP),");
                                        strcat(s, ireg);
                                        break;
                                    case 0xE5:
                                        strcpy(s, "PUSH ");
                                        strcat(s, ireg);
                                        break;
                                    case 0xE9:
                                        strcpy(s, "JP (");
                                        strcat(s, ireg);
                                        strcat(s, ")");
                                        break;
                                    case 0xF9:
                                        strcpy(s, "LD SP,");
                                        strcat(s, ireg);
                                        break;
                                    case 0xCB:
                                        a = Opcodes[adr + 2]; // weiteren Unteropcode
                                        d = (a >> 3) & 7;
                                        stemp[1] = 0;
                                        switch (a & 0xC0) {
                                            case 0x00:
                                            {
                                                static char *str[8] = {"RLC", "RRC", "RL", "RR", "SLA", "SRA", "???", "SRL"};
                                                strcpy(s, str[d]);
                                            }
                                                strcat(s, " ");
                                                break;
                                            case 0x40:
                                                strcpy(s, "BIT ");
                                                stemp[0] = d + '0';
                                                strcat(s, stemp);
                                                strcat(s, ",");
                                                break;
                                            case 0x80:
                                                strcpy(s, "RES ");
                                                stemp[0] = d + '0';
                                                strcat(s, stemp);
                                                strcat(s, ",");
                                                break;
                                            case 0xC0:
                                                strcpy(s, "SET ");
                                                stemp[0] = d + '0';
                                                strcat(s, stemp);
                                                strcat(s, ",");
                                                break;
                                        } /* switch */
                                        strcat(s, "(");
                                        strcat(s, ireg);
                                        strcat(s, "+");
                                        sprintf(stemp, "#%2.2X", Opcodes[adr + 1]); strcat(s, stemp);
                                        strcat(s, "),A");
                                        break;
                                } /* switch */
                                break;
                        } /* switch */
                    } else {
                        strcpy(s, "PUSH ");
                        if ((d >> 1) == 3)
                            strcat(s, "AF");
                        else
                            strcat(s, dreg[d >> 1]);
                    }
                    break;
                case 0x06:
                    strcpy(s, arith[d]);
                    sprintf(stemp, "#%2.2X", Opcodes[adr + 1]);
                    strcat(s, stemp);
                    break;
                case 0x07:
                    strcpy(s, "RST ");
                    sprintf(stemp, "#%2.2X", a & 0x38); strcat(s, stemp);
                    break;
            } /* switch */
            break;
    } /* switch */
} /* Disassemble */

void setVectorLabel(u16 adr, char *label)
{
    vectorLabel[adr] = (char *)malloc(strlen(label) + 1);
    strcpy(vectorLabel[adr], label);
}

void prepareDisasm(void)
{
    int i;

    memset(&OpcodesFlags[0], 0, CODESIZE * sizeof(char));
    memset(&vectorLabel[0], 0, CODESIZE * sizeof(char *));
    memset(&vectorUsed[0], 0, CODESIZE * sizeof(char));

    //  setVectorLabel(0x0000, "RESET");
    //  setVectorLabel(0x0008, "KL_LO_JUMP");
    //  setVectorLabel(0x000B, "KL_LO_CALL_HL");
    //  setVectorLabel(0x000E, "KL_JP_BC");
    //  setVectorLabel(0x0010, "KL_SIDE_CALL_SP");
    //  setVectorLabel(0x0013, "KL_SIDE_CALL_HL");
    //  setVectorLabel(0x0016, "KL_JP_DE");
    //  setVectorLabel(0x0018, "KL_FAR_CALL_SP");
    //  setVectorLabel(0x001B, "KL_FAR_CALL_C_HL");
    //  setVectorLabel(0x001E, "KL_JP_HL");
    //  setVectorLabel(0x0020, "KL_RAM_LD_A_HL");
    //  setVectorLabel(0x0023, "KL_FAR_CALL_HL");
    //  setVectorLabel(0x0028, "KL_FIRM_JUMP");
    //  setVectorLabel(0x002B, "DATA");
    //  setVectorLabel(0x0030, "FREE_FOR_USER");
    //  setVectorLabel(0x0038, "INTERRUPT");
    //  setVectorLabel(0x003B, "EXT_INTERRUPT_VECTOR");
    setVectorLabel(0xB900, "KL_U_ROM_ENABLE");
    setVectorLabel(0xB903, "KL_U_ROM_DISABLE");
    setVectorLabel(0xB906, "KL_L_ROM_ENABLE");
    setVectorLabel(0xB909, "KL_L_ROM_DISABLE");
    setVectorLabel(0xB90C, "KL_ROM_RESTORE");
    setVectorLabel(0xB90F, "KL_ROM_SELECT");
    setVectorLabel(0xB912, "KL_CURR_SELECTION");
    setVectorLabel(0xB915, "KL_PROBE_ROM");
    setVectorLabel(0xB918, "KL_ROM_DESELECT");
    setVectorLabel(0xB91B, "KL_RAM_LDIR");
    setVectorLabel(0xB91E, "KL_RAM_LDDR");
    setVectorLabel(0xB921, "KL_POLL_SYNCHRONOUS");
    setVectorLabel(0xB92A, "KL_SCAN_NEEDED_664");
    setVectorLabel(0xBB00, "KM_INITIALIZE");
    setVectorLabel(0xBB03, "KM_RESET");
    setVectorLabel(0xBB06, "KM_WAIT_CHAR");
    setVectorLabel(0xBB09, "KM_READ_CHAR");
    setVectorLabel(0xBB0C, "KM_CHAR_RETURN");
    setVectorLabel(0xBB0F, "KM_SET_EXPAND");
    setVectorLabel(0xBB12, "KM_GET_EXPAND");
    setVectorLabel(0xBB15, "KM_EXP_BUF_RESET");
    setVectorLabel(0xBB18, "KM_WAIT_KEY");
    setVectorLabel(0xBB1B, "KM_READ_KEY");
    setVectorLabel(0xBB1E, "KM_TEST_KEY");
    setVectorLabel(0xBB21, "KM_GET_LOCKS");
    setVectorLabel(0xBB24, "KM_GET_JOYSTICK");
    setVectorLabel(0xBB27, "KM_SET_TRANSLATE");
    setVectorLabel(0xBB2A, "KM_GET_TRANSLATE");
    setVectorLabel(0xBB2D, "KM_SET_SHIFT");
    setVectorLabel(0xBB30, "KM_GET_SHIFT");
    setVectorLabel(0xBB33, "KM_SET_CTRL");
    setVectorLabel(0xBB36, "KM_GET_CTRL");
    setVectorLabel(0xBB39, "KM_SET_REPEAT");
    setVectorLabel(0xBB3C, "KM_GET_REPEAT");
    setVectorLabel(0xBB3F, "KM_SET_DELAY");
    setVectorLabel(0xBB42, "KM_GET_DELAY");
    setVectorLabel(0xBB45, "KM_ARM_BREAK");
    setVectorLabel(0xBB48, "KM_DISARM_BREAK");
    setVectorLabel(0xBB4B, "KM_BREAK_EVENT");
    setVectorLabel(0xBB4E, "TXT_INITIALIZE");
    setVectorLabel(0xBB51, "TXT_RESET");
    setVectorLabel(0xBB54, "TXT_VDU_DISABLE");
    setVectorLabel(0xBB57, "TXT_VDU_ENABLE");
    setVectorLabel(0xBB5A, "TXT_OUTPUT");
    setVectorLabel(0xBB5D, "TXT_WR_CHAR");
    setVectorLabel(0xBB60, "TXT_RD_CHAR");
    setVectorLabel(0xBB63, "TXT_SET_GRAPHIC");
    setVectorLabel(0xBB66, "TXT_SET_WINDOW");
    setVectorLabel(0xBB69, "TXT_GET_WINDOW");
    setVectorLabel(0xBB6C, "TXT_CLEAR_WINDOW");
    setVectorLabel(0xBB6F, "TXT_SET_COLUMN");
    setVectorLabel(0xBB72, "TXT_SET_ROW");
    setVectorLabel(0xBB75, "TXT_SET_CURSOR");
    setVectorLabel(0xBB78, "TXT_GET_CURSOR");
    setVectorLabel(0xBB7B, "TXT_CUR_ENABLE");
    setVectorLabel(0xBB7E, "TXT_CUR_DISABLE");
    setVectorLabel(0xBB81, "TXT_CUR_ON");
    setVectorLabel(0xBB84, "TXT_CUR_OFF");
    setVectorLabel(0xBB87, "TXT_VALIDATE");
    setVectorLabel(0xBB8A, "TXT_PLACE_CURSOR");
    setVectorLabel(0xBB8D, "TXT_REMOVE_CURSOR");
    setVectorLabel(0xBB90, "TXT_SET_PEN");
    setVectorLabel(0xBB93, "TXT_GET_PEN");
    setVectorLabel(0xBB96, "TXT_SET_PAPER");
    setVectorLabel(0xBB99, "TXT_GET_PAPER");
    setVectorLabel(0xBB9C, "TXT_INVERSE");
    setVectorLabel(0xBB9F, "TXT_SET_BACK");
    setVectorLabel(0xBBA2, "TXT_GET_BACK");
    setVectorLabel(0xBBA5, "TXT_GET_MATRIX");
    setVectorLabel(0xBBA8, "TXT_SET_MATRIX");
    setVectorLabel(0xBBAB, "TXT_SET_M_TABLE");
    setVectorLabel(0xBBAE, "TXT_GET_M_TABLE");
    setVectorLabel(0xBBB1, "TXT_GET_CONTROLS");
    setVectorLabel(0xBBB4, "TXT_STR_SELECT");
    setVectorLabel(0xBBB7, "TXT_SWAP_STREAMS");
    setVectorLabel(0xBBBA, "GRA_INITIALIZE");
    setVectorLabel(0xBBBD, "GRA_RESET_HOOKS");
    setVectorLabel(0xBBC0, "GRA_MOVE_ABSOLUTE");
    setVectorLabel(0xBBC3, "GRA_MOVE_RELATIVE");
    setVectorLabel(0xBBC6, "GRA_ASK_CURSOR");
    setVectorLabel(0xBBC9, "GRA_SET_ORIGIN");
    setVectorLabel(0xBBCC, "GRA_GET_ORIGIN");
    setVectorLabel(0xBBCF, "GRA_SET_WIN_WIDTH");
    setVectorLabel(0xBBD2, "GRA_SET_WIN_HEIGHT");
    setVectorLabel(0xBBD5, "GRA_GET_WIN_WIDTH");
    setVectorLabel(0xBBD8, "GRA_GET_WIN_HEIGHT");
    setVectorLabel(0xBBDB, "GRA_CLEAR_WINDOW");
    setVectorLabel(0xBBDE, "GRA_SET_PEN");
    setVectorLabel(0xBBE1, "GRA_GET_PEN");
    setVectorLabel(0xBBE4, "GRA_SET_PAPER");
    setVectorLabel(0xBBE7, "GRA_GET_PAPER");
    setVectorLabel(0xBBEA, "GRA_PLOT_ABSOLUTE");
    setVectorLabel(0xBBED, "GRA_PLOT_RELATIVE");
    setVectorLabel(0xBBF0, "GRA_TEST_ABSOLUTE");
    setVectorLabel(0xBBF3, "GRA_TEST_RELATIVE");
    setVectorLabel(0xBBF6, "GRA_LINE_ABSOLUTE");
    setVectorLabel(0xBBF9, "GRA_LINE_RELATIVE");
    setVectorLabel(0xBBFC, "GRA_WR_CHAR");
    setVectorLabel(0xBBFF, "SCR_INITIALIZE");
    setVectorLabel(0xBC02, "SCR_RESET");
    setVectorLabel(0xBC05, "SCR_SET_OFFSET");
    setVectorLabel(0xBC08, "SCR_SET_BASE");
    setVectorLabel(0xBC0B, "SCR_GET_LOCATION");
    setVectorLabel(0xBC0E, "SCR_SET_MODE");
    setVectorLabel(0xBC11, "SCR_GET_MODE");
    setVectorLabel(0xBC14, "SCR_MODE_CLEAR");
    setVectorLabel(0xBC17, "SCR_CHAR_LIMITS");
    setVectorLabel(0xBC1A, "SCR_CHAR_POSITION");
    setVectorLabel(0xBC1D, "SCR_DOT_POSITION");
    setVectorLabel(0xBC20, "SCR_NEXT_BYTE");
    setVectorLabel(0xBC23, "SCR_PREV_BYTE");
    setVectorLabel(0xBC26, "SCR_NEXT_LINE");
    setVectorLabel(0xBC29, "SCR_PREV_LINE");
    setVectorLabel(0xBC2C, "SCR_INK_ENCODE");
    setVectorLabel(0xBC2F, "SCR_INK_DECODE");
    setVectorLabel(0xBC32, "SCR_SET_INK");
    setVectorLabel(0xBC35, "SCR_GET_INK");
    setVectorLabel(0xBC38, "SCR_SET_BORDER");
    setVectorLabel(0xBC3B, "SCR_GET_BORDER");
    setVectorLabel(0xBC3E, "SCR_SET_FLASHING");
    setVectorLabel(0xBC41, "SCR_GET_FLASHING");
    setVectorLabel(0xBC44, "SCR_FILL_BOX");
    setVectorLabel(0xBC47, "SCR_FLOOD_BOX");
    setVectorLabel(0xBC4A, "SCR_CHAR_INVERT");
    setVectorLabel(0xBC4D, "SCR_HARDWARE_ROLL");
    setVectorLabel(0xBC50, "SCR_SOFTWARE_ROLL");
    setVectorLabel(0xBC53, "SCR_UNPACK");
    setVectorLabel(0xBC56, "SCR_REPACK");
    setVectorLabel(0xBC59, "SCR_ACCESS");
    setVectorLabel(0xBC5C, "SCR_PIXELS");
    setVectorLabel(0xBC5F, "SCR_HORIZONTAL");
    setVectorLabel(0xBC62, "SCR_VERTICAL");
    setVectorLabel(0xBC65, "CAS_INITIALIZE");
    setVectorLabel(0xBC68, "CAS_SET_SPEED");
    setVectorLabel(0xBC6B, "CAS_NOISY");
    setVectorLabel(0xBC6E, "CAS_START_MOTOR");
    setVectorLabel(0xBC71, "CAS_STOP_MOTOR");
    setVectorLabel(0xBC74, "CAS_RESTORE_MOTOR");
    setVectorLabel(0xBC77, "CAS_IN_OPEN");
    setVectorLabel(0xBC7A, "CAS_IN_CLOSE");
    setVectorLabel(0xBC7D, "CAS_IN_ABANDON");
    setVectorLabel(0xBC80, "CAS_IN_CHAR");
    setVectorLabel(0xBC83, "CAS_IN_DIRECT");
    setVectorLabel(0xBC86, "CAS_RETURN");
    setVectorLabel(0xBC89, "CAS_TEST_EOF");
    setVectorLabel(0xBC8C, "CAS_OUT_OPEN");
    setVectorLabel(0xBC8F, "CAS_OUT_CLOSE");
    setVectorLabel(0xBC92, "CAS_OUT_ABANDON");
    setVectorLabel(0xBC95, "CAS_OUT_CHAR");
    setVectorLabel(0xBC98, "CAS_OUT_DIRECT");
    setVectorLabel(0xBC9B, "CAS_CATALOG");
    setVectorLabel(0xBC9E, "CAS_WRITE");
    setVectorLabel(0xBCA1, "CAS_READ");
    setVectorLabel(0xBCA4, "CAS_CHECK");
    setVectorLabel(0xBCA7, "SOUND_RESET");
    setVectorLabel(0xBCAA, "SOUND_QUEUE");
    setVectorLabel(0xBCAD, "SOUND_CHECK");
    setVectorLabel(0xBCB0, "SOUND_ARM_EVENT");
    setVectorLabel(0xBCB3, "SOUND_RELEASE");
    setVectorLabel(0xBCB6, "SOUND_PAUSE");
    setVectorLabel(0xBCB9, "SOUND_UNPAUSE");
    setVectorLabel(0xBCBC, "SOUND_SET_ENV");
    setVectorLabel(0xBCBF, "SOUND_SET_ENT");
    setVectorLabel(0xBCC2, "SOUND_GET_ENV");
    setVectorLabel(0xBCC5, "SOUND_GET_ENT");
    setVectorLabel(0xBCC8, "KL_CHOKE_OFF");
    setVectorLabel(0xBCCB, "KL_ROM_WALK");
    setVectorLabel(0xBCCE, "KL_INIT_BACK");
    setVectorLabel(0xBCD1, "KL_LOG_EXT");
    setVectorLabel(0xBCD4, "KL_FIND_COMMAND");
    setVectorLabel(0xBCD7, "KL_NEW_FRAME_FLY");
    setVectorLabel(0xBCDA, "KL_ADD_FRAME_FLY");
    setVectorLabel(0xBCDD, "KL_DELETE_FRAME_FLY");
    setVectorLabel(0xBCE0, "KL_NEW_FAST_TICKER");
    setVectorLabel(0xBCE3, "KL_ADD_FAST_TICKER");
    setVectorLabel(0xBCE6, "KL_DELETE_FAST_TICKER");
    setVectorLabel(0xBCE9, "KL_ADD_TICKER");
    setVectorLabel(0xBCEC, "KL_DELETE_TICKER");
    setVectorLabel(0xBCEF, "KL_INIT_EVENT");
    setVectorLabel(0xBCF2, "KL_EVENT");
    setVectorLabel(0xBCF5, "KL_SYNC_RESET");
    setVectorLabel(0xBCF8, "KL_DEL_SYNCHRONOUS");
    setVectorLabel(0xBCFB, "KL_NEXT_SYNC");
    setVectorLabel(0xBCFE, "KL_DO_SYNC");
    setVectorLabel(0xBD01, "KL_DONE_SYNC");
    setVectorLabel(0xBD04, "KL_EVENT_DISABLE");
    setVectorLabel(0xBD07, "KL_EVENT_ENABLE");
    setVectorLabel(0xBD0A, "KL_DISARM_EVENT");
    setVectorLabel(0xBD0D, "KL_TIME_PLEASE");
    setVectorLabel(0xBD10, "KL_TIME_SET");
    setVectorLabel(0xBD13, "MC_BOOT_PROGRAM");
    setVectorLabel(0xBD16, "MC_START_PROGRAM");
    setVectorLabel(0xBD19, "MC_WAIT_FLYBACK");
    setVectorLabel(0xBD1C, "MC_SET_MODE");
    setVectorLabel(0xBD1F, "MC_SCREEN_OFFSET");
    setVectorLabel(0xBD22, "MC_CLEAR_INKS");
    setVectorLabel(0xBD25, "MC_SET_INKS");
    setVectorLabel(0xBD28, "MC_RESET_PRINTER");
    setVectorLabel(0xBD2B, "MC_PRINT_CHAR");
    setVectorLabel(0xBD2E, "MC_BUSY_PRINTER");
    setVectorLabel(0xBD31, "MC_SEND_PRINTER");
    setVectorLabel(0xBD34, "MC_SOUND_REGISTER");
    setVectorLabel(0xBD37, "JUMP_RESTORE");
    setVectorLabel(0xBD3A, "KM_SET_LOCKS_664");
    setVectorLabel(0xBD3D, "KM_FLUSH_664");
    setVectorLabel(0xBD40, "TXT_ASK_STATE_664");
    setVectorLabel(0xBD43, "GRA_DEFAULT_664");
    setVectorLabel(0xBD46, "GRA_SET_BACK_664");
    setVectorLabel(0xBD49, "GRA_SET_FIRST_664");
    setVectorLabel(0xBD4C, "GRA_SET_LINE_MASK_664");
    setVectorLabel(0xBD4F, "GRA_FROM_USER_664");
    setVectorLabel(0xBD52, "GRA_FILL_664");
    setVectorLabel(0xBD55, "SCR_SET_POSITION_664");
    setVectorLabel(0xBD58, "MC_PRINT_TRANSLATION_664");
    setVectorLabel(0xBD5B, "KL_RAM_SELECT_6128");
    setVectorLabel(0xBDCD, "HOOK_TXT_DRAW_CURSOR");
    setVectorLabel(0xBDD0, "HOOK_TXT_UNDRAW_CURSOR");
    setVectorLabel(0xBDD3, "HOOK_TXT_WRITE_CHAR");
    setVectorLabel(0xBDD6, "HOOK_TXT_UNWRITE");
    setVectorLabel(0xBDD9, "HOOK_TXT_OUT_ACTION");
    setVectorLabel(0xBDDC, "HOOK_GRA_PLOT");
    setVectorLabel(0xBDDF, "HOOK_GRA_TEST");
    setVectorLabel(0xBDE2, "HOOK_GRA_LINE");
    setVectorLabel(0xBDE5, "HOOK_SCR_READ");
    setVectorLabel(0xBDE8, "HOOK_SCR_WRITE");
    setVectorLabel(0xBDEB, "HOOK_SCR_MODE_CLEAR");
    setVectorLabel(0xBDEE, "HOOK_KM_TEST_BREAK");
    setVectorLabel(0xBDF1, "HOOK_MC_PRINT_CHAR");
    setVectorLabel(0xBDF4, "HOOK_KM_SCAN_KEYS_664");

    for (i = 0; i < CODESIZE; i++) { // alles Daten…
        OpcodesFlags[i] = Data;
    }

} /* prepareDisasm */

void unAssemble(u8 *Opcodes, char *output, u16 adrBeg, u16 length)
{
    u16 i;

    u16 adr = 0;
    char s[80];
    char tmp[128];

    output[0] = 0;

    prepareDisasm();

    codeBeg = adrBeg;
    codeEnd = codeBeg + length;

    printf("%d bytes length\n", length);

// First Pass

    for (i = 0; i < 0x40; i += 0x08) {
        if ((OpcodesFlags[i] & 0x0F) == Data) {
            ParseOpcodes(i);  // RST-Vektoren parsen (wenn nötig)
        }
    }

// Second Pass: Search for label datas

    adr = 0;
    while (adr < length) {
        u16 len, i;
        // char stemp[80];

        if ((OpcodesFlags[adr] & 0x0F) == Data) {
            OpcodesFlags[adr] |= 0x10;

            for (i = 0; i < 16; i++) {
                if ((OpcodesFlags[adr + i] & 0x0F) != Data) {
                    break;
                }
            }
            adr += i;
        } else {
            len = OpcodeLen(adr); // Get opcode length
            adr += len;
        }
    }


// Third pass

    adr = 0;
    while (adr < length) {
        u16 len, i, next;
        char stemp[80];

        if ((OpcodesFlags[adr] & 0x0F) == Data) {
            labelForAddr(stemp, adr);
            strcat(stemp, ":");
            sprintf(tmp, "%-24s DB", stemp);
            strcat(output, tmp);

            for (i = 0; i < 16; i++) {
                if ((adr + i >= codeEnd) || ((OpcodesFlags[adr + i] & 0x0F) != Data)) {
                    break;
                }
                sprintf(tmp, "%c%2.2Xh", (i) ? ',' : ' ', Opcodes[adr + i]);
                strcat(output, tmp);

            }
            sprintf(tmp, "\n");
            strcat(output, tmp);

            next = i;
        } else {
            len = OpcodeLen(adr); // Get opcode length

            if (OpcodesFlags[adr] & 0x10) {
                labelForAddr(stemp, adr);
                strcat(stemp, ":");
                sprintf(tmp, "%-24s ", stemp);
            } else {
                sprintf(tmp, "%-24s ", "");
            }
            strcat(output, tmp);

            Disassemble(adr, s);
            sprintf(tmp, "%-32s  ", s);
            strcat(output, tmp);

            strcat(output, ";");

            for (i = 0; i < len; i++) {
                sprintf(tmp, " %02x", Opcodes[adr + i]);
                strcat(output, tmp);

            }
            strcat(tmp, "\n");

            next = len;
        }

        adr += next;
    }

    for (adr = 0; adr < CODESIZE; adr++) {
        if (vectorUsed[adr]) {
            sprintf(tmp, "%s equ #%04X\n", vectorLabel[adr], adr);
            strcat(output, tmp);
        }
    }

} /* unAssemble */


#ifdef STANDALONE
int main(void)
{
    u16 i;
    FILE *f;
    u16 adr = 0;
    char s[80];
    int length;

    prepareDisasm();

    f = os_fopen("rasmoutput.bin", "rb");
    if (!f) return 0;
    length = fread(Opcodes, 1, CODESIZE, f);
    fclose(f);

    codeBeg = 0;
    codeEnd = codeBeg + length;

    printf("%d bytes length\n", length);

// First Pass

    for (i = 0; i < 0x40; i += 0x08) {
        if ((OpcodesFlags[i] & 0x0F) == Data) {
            ParseOpcodes(i);  // RST-Vektoren parsen (wenn nötig)
        }
    }

// Second Pass: Search for label datas

    adr = 0;
    while (adr < length) {
        u16 len, i;
        char stemp[80];

        if ((OpcodesFlags[adr] & 0x0F) == Data) {
            OpcodesFlags[adr] |= 0x10;

            for (i = 0; i < 16; i++) {
                if ((OpcodesFlags[adr + i] & 0x0F) != Data) {
                    break;
                }
            }
            adr += i;
        } else {
            len = OpcodeLen(adr); // Get opcode length
            adr += len;
        }
    }

    f = os_fopen("output.s", "w");
    if (!f) return 0;

// Third pass

    adr = 0;
    while (adr < length) {
        u16 len, i, next;
        char stemp[80];

        if ((OpcodesFlags[adr] & 0x0F) == Data) {
            labelForAddr(stemp, adr);
            strcat(stemp, ":");
            fprintf(f, "%-24s DB", stemp);

            for (i = 0; i < 16; i++) {
                if ((adr + i >= codeEnd) || ((OpcodesFlags[adr + i] & 0x0F) != Data)) {
                    break;
                }
                fprintf(f, "%c%2.2Xh", (i) ? ',' : ' ', Opcodes[adr + i]);
            }
            fprintf(f, "\n");
            next = i;
        } else {
            len = OpcodeLen(adr); // Get opcode length

            if (OpcodesFlags[adr] & 0x10) {
                labelForAddr(stemp, adr);
                strcat(stemp, ":");
                fprintf(f, "%-24s ", stemp);
            } else {
                fprintf(f, "%-24s ", "");
            }

            Disassemble(adr, s);
            fprintf(f, "%-32s  ", s);

            fprintf(f, ";");

            for (i = 0; i < len; i++) {
                fprintf(f, " %02x", Opcodes[adr + i]);
            }
            fprintf(f, "\n");

            next = len;
        }

        adr += next;
    }

    for (adr = 0; adr < CODESIZE; adr++) {
        if (vectorUsed[adr]) {
            fprintf(f, "%s equ #%04X\n", vectorLabel[adr], adr);
        }
    }

    fclose(f);

    return 0;
} /* main */
#endif /* ifdef STANDALONE */