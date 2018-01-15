//
//  memory.c
//  sh2
//
//  Created by Antonio Malara on 07/10/16.
//  Copyright © 2016 Antonio Malara. All rights reserved.
//

#include "memory.h"
#include "sh2.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  uptr; // unsigned pointer-sized int

typedef struct {
    uptr addr; // stores (membase >> 1) or ((handler >> 1) | (1<<31))
    u32 mask;
} sh2_memmap;

#define SH2MAP_ADDR2OFFS_R(a) \
((u32)(a) >> SH2_READ_SHIFT)

#define SH2MAP_ADDR2OFFS_W(a) \
((u32)(a) >> SH2_WRITE_SHIFT)

#define SH2_READ_SHIFT 25
#define SH2_WRITE_SHIFT 25

#define MAP_FLAG ((uptr)1 << (sizeof(uptr) * 8 - 1))
#define map_flag_set(x) ((x) & MAP_FLAG)

#define REGPARM(x)

typedef u32 (sh2_read_handler)(u32 a, SH2 *sh2);
typedef void REGPARM(3) (sh2_write_handler)(u32 a, u32 d, SH2 *sh2);

u32 REGPARM(2) p32x_sh2_read8(u32 a, SH2 *sh2)
{
    const sh2_memmap *sh2_map = sh2->read8_map;
    uptr p;
    
    sh2_map += SH2MAP_ADDR2OFFS_R(a);
    p = sh2_map->addr;
    if (map_flag_set(p))
        return ((sh2_read_handler *)(p << 1))(a, sh2);
    else
        return *(u8 *)((p << 1) + ((a & sh2_map->mask) ^ 1));
}

u32 REGPARM(2) p32x_sh2_read16(u32 a, SH2 *sh2)
{
    const sh2_memmap *sh2_map = sh2->read16_map;
    uptr p;
    
    sh2_map += SH2MAP_ADDR2OFFS_R(a);
    p = sh2_map->addr;
    if (map_flag_set(p))
        return ((sh2_read_handler *)(p << 1))(a, sh2);
    else
        return *(u16 *)((p << 1) + ((a & sh2_map->mask) & ~1));
}

u32 REGPARM(2) p32x_sh2_read32(u32 a, SH2 *sh2)
{
    const sh2_memmap *sh2_map = sh2->read16_map;
    sh2_read_handler *handler;
    u32 offs;
    uptr p;
    
    offs = SH2MAP_ADDR2OFFS_R(a);
    sh2_map += offs;
    p = sh2_map->addr;
    if (!map_flag_set(p)) {
        // XXX: maybe 32bit access instead with ror?
        u16 *pd = (u16 *)((p << 1) + ((a & sh2_map->mask) & ~1));
        return (pd[0] << 16) | pd[1];
    }
    
//    if (offs == SH2MAP_ADDR2OFFS_R(0xffffc000))
//        return sh2_peripheral_read32(a, sh2);
    
    handler = (sh2_read_handler *)(p << 1);
    return (handler(a, sh2) << 16) | handler(a + 2, sh2);
}

void REGPARM(3) p32x_sh2_write8(u32 a, u32 d, SH2 *sh2)
{
    const void **sh2_wmap = sh2->write8_tab;
    sh2_write_handler *wh;
    
    wh = sh2_wmap[SH2MAP_ADDR2OFFS_W(a)];
    wh(a, d, sh2);
}

void REGPARM(3) p32x_sh2_write16(u32 a, u32 d, SH2 *sh2)
{
    const void **sh2_wmap = sh2->write16_tab;
    sh2_write_handler *wh;
    
    wh = sh2_wmap[SH2MAP_ADDR2OFFS_W(a)];
    wh(a, d, sh2);
}

void REGPARM(3) p32x_sh2_write32(u32 a, u32 d, SH2 *sh2)
{
    const void **sh2_wmap = sh2->write16_tab;
    sh2_write_handler *wh;
    u32 offs;
    
    offs = SH2MAP_ADDR2OFFS_W(a);
    
    //if (offs == SH2MAP_ADDR2OFFS_W(0xffffc000)) {
    //    sh2_peripheral_write32(a, d, sh2);
    //    return;
    //}
    
    wh = sh2_wmap[offs];
    wh(a, d >> 16, sh2);
    wh(a + 2, d, sh2);
}

