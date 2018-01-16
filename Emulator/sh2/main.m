//
//  main.m
//  sh2
//
//  Created by Antonio Malara on 07/10/16.
//  Copyright © 2016 Antonio Malara. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "sh2.h"

#define sh2_pc(sh2) (sh2)->pc

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  uptr; // unsigned pointer-sized int


static const uint32_t on_chip_ram_start = 0xFFFFF000;
uint8_t on_chip_ram[0x1000];

static const uint32_t dram_start = 0x01000000;
uint8_t dram[0x1000000];

typedef struct {
    uint32_t   start;
    uint32_t   length;
    uint8_t  * bytes;
} rom_t;

rom_t roms[4];

struct symbols_t {
    uint32_t addr;
    char * name;
} symbols[] = {
    { 0xffff8620, "BCR1    - Bus control register 1" },
    { 0xffff8622, "BCR2    - Bus control register 2" },
    { 0xffff8624, "WCR1    - Wait control register 1" },
    { 0xffff8626, "WCR2    - Wait control register 2" },
    { 0xffff862a, "DCR     - Dram area control register" },
    { 0xffff862c, "RTCSR   - Refresh timer control/status register" },
    { 0xffff862e, "RTCNT   - Refresh timer counter" },
    { 0xffff8630, "RTCOR   - Refresh timer constant register" },
    { 0xffff8204, "TIOR3H" },
    { 0xffff8205, "TIOR3L" },
    { 0xffff8260, "TCR_0   - Timer control register 0" },
    { 0xffff8261, "TMDR_0  - Timer mode register 0" },
    { 0xffff8262, "TIORH_0 - Timer IO control register H 0" },
    { 0xffff8263, "TIORL_0 - Timer IO control register L 0" },
    { 0xffff826e, "TGRD_0  - Timer general register D 0" },
    { 0xffff8200, "TCR_3   - Timer control register 3" },
    { 0xffff8202, "TMDR_3  - Timer mode register 3" },
    { 0xffff8262, "TIORH_3 - Timer IO control register H 3" },
    { 0xffff8263, "TIORL_3 - Timer IO control register L 3" },
    { 0xffff8218, "TGRA_3  - Timer general register A 3" },
    { 0xffff821a, "TGRB_3  - Timer general register B 3" },
    { 0xffff82a0, "TCR_2   - Timer control register 2" },
    { 0xffff82a1, "TMDR_2  - Timer mode register 2" },
    { 0xffff82a2, "TIOR_2  - Timer IO control register 2" },
    { 0xffff82a8, "TGRA_2  - Timer IO general register 2" },
    { 0xffff8240, "TSTR    - Timer start register" },
    { 0xffff83b0, "PEDRL   - Port E data register L" },
    { 0xffff83b4, "PEIORL  - Port E IO register L" },
    { 0xffff83b8, "PECRL1  - Port E control register L1" },
    { 0xffff83ba, "PECRL2  - Port E control register L2" },
    { 0xffff8380, "PADRH   - Port A data register H" },
    { 0xffff8382, "PADRL   - Port A data register L" },
    { 0xffff8384, "PAIORH" },
    { 0xffff8386, "PAIORL  - Port A IO register L" },
    { 0xffff8388, "PACRH" },
    { 0xffff838c, "PACRL1  - Port A control register L1" },
    { 0xffff838e, "PACRL2  - Port A control register L2" },
    { 0xffff8398, "BPCR1   - Port B control register 1" },
    { 0xffff839a, "BPCR2   - Port B control register 2" },
    { 0xffff839c, "PCCR    - Port C control register" },
    { 0xffff83a8, "PDCRH1  - Port D control register H1" },
    { 0xffff83a8, "PDCRH2  - Port D control register H2" },
    { 0xffff83ac, "PDCRL1  - Port D control register L1" },
    { 0xffff83ae, "PDCRL2  - Port D control register L2" },
    { 0xffff83aa, "PDCRH2  - Port D control register H2" },
    { 0xffff86cc, "CHCR0   - DMA channel control register 0" },
    { 0xffff86dc, "CHCR1   - DMA channel control register 1" },
    { 0xffff86ec, "CHCR2   - DMA channel control register 2" },
    { 0xffff86fc, "CHCR3   - DMA channel control register 3" },
    { 0xffff86b0, "DMAOR   - DMA operation register" },
};

char * name_for(uint32_t addr) {
    if ((0x00200000 <= addr) && (addr <= 0x003FFFFF))
        return "(CS0)";
    
    if ((0x00400000 <= addr) && (addr <= 0x007FFFFF))
        return "(CS1)";
    
    if ((0x00800000 <= addr) && (addr <= 0x00BFFFFF))
        return "(CS2)";
    
    if ((0x00C00000 <= addr) && (addr <= 0x00FFFFFF))
        return "(CS3)";
    
    if ((0x01000000 <= addr) && (addr <= 0x01FFFFFF))
        return "(DRAM)";
    
    for (int i = 0; i < sizeof(symbols) / sizeof(struct symbols_t); i++) {
        if (addr == symbols[i].addr) {
            return symbols[i].name;
        }
    }
    
    return "";
}


/*
 
 cs2 + 0x00 0000 -- SPC
 cs2 + 0x10 0000 -- LCDCM
 cs2 + 0x20 0000 -- LCDCIO
 cs2 + 0x30 0000 -- TGL

 cs3 + 0x00 0000
 cs3 + 0x10 0000 -- MOSS
 cs3 + 0x20 0000 -- FDC
 cs3 + 0x30 0000 -- SCU

 */

u32 p32x_sh2_read8(u32 a, SH2 *sh2) {
    if (a == 0xf00000) {
        printf("pc: %08x )  reading    at %08x : 1\n", sh2->pc, a);
        return 1;
    }
    
    if (a == 0xf00001) {
        printf("reading %08x -- 1\n", a);
        static int i = 0;
        return ++i;
    }
    
    // -----
    
    for (int i = 0; i < (sizeof(roms) / sizeof(rom_t)); i++) {
        if ((roms[i].start <= a) &&
            (a < roms[i].start + roms[i].length))
        {
            return roms[i].bytes[a - roms[i].start];
        }
    }

    if ((a >= dram_start) && (a < (dram_start + sizeof(dram)))) {
        printf("pc: %08x )  read  dram at %08x : byte  %8x %s\n", sh2->pc, a,  dram[a - dram_start], name_for(a));
        
        return dram[a - dram_start];
    }
    else if (a >= on_chip_ram_start)
    {
        printf("pc: %08x )  read  cram at %08x : byte  %8x %s\n", sh2->pc, a,  on_chip_ram[a - on_chip_ram_start], name_for(a));
        return on_chip_ram[a - on_chip_ram_start];
    }
    else {
        printf("pc: %08x )  read UNMAP at %08x : byte  %8x %s\n", sh2->pc, a,  0, name_for(a));
        return 0;
    }
}

u32 p32x_sh2_read16(u32 a, SH2 *sh2) {
    return ((p32x_sh2_read8(a, sh2) & 0xff) << 8) + (p32x_sh2_read8(a + 1, sh2) & 0xff);
}

u32 p32x_sh2_read32(u32 a, SH2 *sh2) {
    return (p32x_sh2_read16(a, sh2) << 16) + p32x_sh2_read16(a + 2, sh2);
}

void p32x_sh2_write8_ex(u32 a, u32 d, SH2 *sh2, BOOL print) {
    if ((a >= dram_start) &&
        (a < (dram_start + sizeof(dram))))
    {
        dram[a - dram_start] = d;
        if (print) printf("pc: %08x )  write dram at %08x : byte  %8x %s\n", sh2->pc, a, d, name_for(a));
    }
    else if (a >= on_chip_ram_start)
    {
        on_chip_ram[a - on_chip_ram_start] = d;
        if (print) printf("pc: %08x )  write cram at %08x : byte  %8x %s\n", sh2->pc, a, d, name_for(a));
    }
    else {
        if (print) printf("pc: %08x )  write      at %08x : byte  %8x %s\n", sh2->pc, a, d, name_for(a));
    }
}

void p32x_sh2_write8(u32 a, u32 d, SH2 *sh2) {
    p32x_sh2_write8_ex(a, d, sh2, true);
}

void p32x_sh2_write16_ex(u32 a, u32 d, SH2 *sh2, BOOL print) {
    p32x_sh2_write8_ex(a,     (d & 0xff00) >> 8, sh2, false);
    p32x_sh2_write8_ex(a + 1, (d & 0x00ff),      sh2, false);

    if ((a >= dram_start) &&
        (a < (dram_start + sizeof(dram))))
    {
        if (print) printf("pc: %08x )  write dram at %08x : word  %8x %s\n", sh2->pc, a, d, name_for(a));
    }
    else if (a >= on_chip_ram_start)
    {
        if (print) printf("pc: %08x )  write cram at %08x : word  %8x %s\n", sh2->pc, a, d, name_for(a));
    }
    else {
        if (print) printf("pc: %08x )  write      at %08x : word  %8x %s\n", sh2->pc, a, d, name_for(a));
    }
}

void p32x_sh2_write16(u32 a, u32 d, SH2 *sh2) {
    p32x_sh2_write16_ex(a, d, sh2, true);
}

void p32x_sh2_write32(u32 a, u32 d, SH2 *sh2) {
    p32x_sh2_write16_ex(a,     (d & 0xffff0000) >> 16, sh2, false);
    p32x_sh2_write16_ex(a + 2, (d & 0x0000ffff),       sh2, false);

//    if (sh2->pc == 0x6ce) return;

    if ((a >= dram_start) &&
        (a < (dram_start + sizeof(dram))))
    {
        printf("pc: %08x )  write dram at %08x : dword %8x %s\n", sh2->pc, a, d, name_for(a));
    }
    else if (a >= on_chip_ram_start)
    {
        printf("pc: %08x )  write cram at %08x : dword %8x %s\n", sh2->pc, a, d, name_for(a));
    }
    else {
        printf("pc: %08x )  write      at %08x : dword %8x %s\n", sh2->pc, a, d, name_for(a));
    }
}

static int sh2_irq_cb(SH2 *sh2, int level)
{
    if (sh2->pending_irl > sh2->pending_int_irq) {
        printf( "ack/irl %d @ %08x", level, sh2_pc(sh2));
        return 64 + sh2->pending_irl / 2;
    } else {
        printf("ack/int %d/%d @ %08x", level, sh2->pending_int_vector, sh2_pc(sh2));
        sh2->pending_int_irq = 0; // auto-clear
        sh2->pending_level = sh2->pending_irl;
        return sh2->pending_int_vector;
    }
}

void loadRom(uint32_t start, NSString * file, rom_t * rom) {
    __auto_type data = [NSData dataWithContentsOfFile:file];
    
    rom->start  = start;
    rom->length = (uint32_t)  (data.length - 2);
    rom->bytes  = malloc(rom->length);
    
    memcpy(rom->bytes, data.bytes + 2, rom->length);
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        loadRom(0x00000000, @"disk1/INT01080.710", &roms[0]);
        loadRom(0x00008000, @"disk1/INT13080.710", &roms[1]);
        loadRom(0x00400000, @"disk1/EXT03080.710", &roms[2]);
        loadRom(0x004c0000, @"disk2/EXT34080.710", &roms[3]);

        SH2 sh2;
        sh2_init(&sh2);
        sh2.irq_callback = sh2_irq_cb;
        sh2.read8 = p32x_sh2_read8;
        sh2.read16 = p32x_sh2_read16;
        sh2.read32 = p32x_sh2_read32;
        sh2.write8 = p32x_sh2_write8;
        sh2.write16 = p32x_sh2_write16;
        sh2.write32 = p32x_sh2_write32;
        
        p32x_sh2_read8(0x040008e, &sh2);
        
        sh2_reset(&sh2);
        sh2_execute(&sh2, 10000000, 0);
    }
    return 0;
}
