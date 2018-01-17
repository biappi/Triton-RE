//
//  main.m
//  sh2
//
//  Created by Antonio Malara on 07/10/16.
//  Copyright © 2016 Antonio Malara. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <vector>

extern "C" {
    #import "sh2.h"
}

using namespace std;

struct Memory {
    const uint32_t   start;
    const uint32_t   length;

    const char     * name;

    Memory(const char * name, const uint32_t start, const uint32_t length)
        : name(name), start(start), length(length)
    { }

    virtual ~Memory() { }
    
    bool contains(uint32_t address) {
        return (start <= address) && (address <= start + length - 1);
    }

    virtual uint8_t relative_read(uint32_t address) = 0;
    virtual void    relative_write(uint32_t address, uint8_t v) = 0;
};

class Rom : public Memory {
    uint8_t * data;
    
public:
    Rom(const char * name, const uint32_t start, const uint32_t length, uint8_t * d)
        : Memory(name, start, length)
    {
        data = (uint8_t *)malloc(length);
        memcpy(data, d, length);
    }
    
    ~Rom() {
        free(data);
    }

    virtual uint8_t relative_read(uint32_t address) {
        return data[address];
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        ;
    }
};

class Ram : public Memory {
    uint8_t * data;
    
public:
    Ram(const char * name, const uint32_t start, const uint32_t length)
        : Memory(name, start, length)
    {
        data = (uint8_t *)malloc(length);
        memset(data, 0, length);
    }
    
    ~Ram() {
        free(data);
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        return data[address];
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        data[address] = v;
    }
};

class SCU : public Memory {
    int i = 0;
    
public:
    SCU(const char * name, const uint32_t start)
        : Memory(name, start, 2)
    {
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        if (address == 0) {
            return 1;
        }
        else if (address == 1) {
            return ++i;
        }
        else {
            return 0;
        }
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        ;
    }
};

struct symbols_t {
    uint32_t addr;
    const char * name;
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

const char * name_for(uint32_t addr) {
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

static const char * print_read  = "read ";
static const char * print_write = "write";

static const char * print_byte  = "byte ";
static const char * print_word  = "word ";
static const char * print_dword = "dword";

class AddressSpace {
    vector<Memory *> memoryList;
    SH2 * sh2;
    
public:
    bool trace = true;
    
    AddressSpace(SH2 * sh2)
        : sh2(sh2)
    { }
    
    void add(Memory * memory) {
        memoryList.push_back(memory);
    }
    
    uint32_t read8(uint32_t a) {
        uint32_t d = read(a);
        
        log(print_read, print_byte, a, d);
        return d;
    }
    
    uint32_t read16(uint32_t a) {
        uint32_t d = (((read(a    ) & 0xff) << 8) +
                      ((read(a + 1) & 0xff)     ));
        
        log(print_read, print_word, a, d);
        return d;

    }
    
    uint32_t read32(uint32_t a) {
        uint32_t d = (((read(a    ) & 0xff) << 24) +
                      ((read(a + 1) & 0xff) << 16) +
                      ((read(a + 2) & 0xff) <<  8) +
                      ((read(a + 3) & 0xff)      ));
        
        log(print_read, print_dword, a, d);
        return d;
    }
    
    void write8(uint32_t a, uint32_t d) {
        log(print_write, print_byte, a, d);
        
        write(a, d);
    }
    
    void write16(uint32_t a, uint32_t d) {
        log(print_write, print_word, a, d);
        
        write(a,     (d & 0xff00) >> 8);
        write(a + 1, (d & 0x00ff)     );
    }
    
    void write32(uint32_t a, uint32_t d) {
        log(print_write, print_dword, a, d);
        
        write(a,     (d & 0xff000000) >> 24);
        write(a + 1, (d & 0x00ff0000) >> 16);
        write(a + 2, (d & 0x0000ff00) >>  8);
        write(a + 3, (d & 0x000000ff)      );
    }

private:
    Memory * memoryAtAddress(uint32_t a) {
        for (auto & mem : memoryList)
            if (mem->contains(a))
                return mem;
        
        return nullptr;
    }
    
    uint8_t read(uint32_t a) {
        auto mem = memoryAtAddress(a);
        
        if (mem) {
            return mem->relative_read(a - mem->start);
        }
        else {
            return 0;
        }
    }
    
    void write(uint32_t a, uint8_t v) {
        auto mem = memoryAtAddress(a);
        
        if (mem) {
            mem->relative_write(a - mem->start, v);
        }
    }
    
    void log(const char * rw,
             const char * s,
             uint32       a,
             uint32       d)
    {

        // LCDCIO
        if (sh2->pc == 0x00000656)
            return;
        
//        if ((sh2->pc == 0x000006cc) || (sh2->pc == 0x000006ce))
//            return;
        
        if ((sh2->pc == 0x000006e8) || (sh2->pc == 0x000006ea))
            return;
        
        const char * name;
        const char * space;
        
        if (a == sh2->pc || a == sh2->ppc)
            return;
        
        auto mem = memoryAtAddress(a);
        
        if (mem) {
            space = mem->name;
        }
        else {
            space = "!*!*!*!*";
        }
        
        name = name_for(a);
        printf("pc: %08x )  %s %s at %08x : %s %8x %s\n",
               sh2->pc,
               rw,
               space,
               a,
               s,
               d,
               name_for(a));
    }
};

static int sh2_irq_cb(SH2 *sh2, int level)
{
    if (sh2->pending_irl > sh2->pending_int_irq) {
        printf( "ack/irl %d @ %08x", level, sh2->pc);
        return 64 + sh2->pending_irl / 2;
    } else {
        printf("ack/int %d/%d @ %08x", level, sh2->pending_int_vector, sh2->pc);
        sh2->pending_int_irq = 0; // auto-clear
        sh2->pending_level = sh2->pending_irl;
        return sh2->pending_int_vector;
    }
}

Rom * RomFile(const char * name, const uint32_t start, NSString * file) {
    auto data = [NSData dataWithContentsOfFile:file];
    auto length = (const uint32_t) (data.length - 2);
    
    return new Rom(name, start, length, ((uint8_t *)data.bytes + 2));
}

SH2 sh2;
auto memory = AddressSpace(&sh2);

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        sh2_init(&sh2);

        memory.add(RomFile("INT01080", 0x00000000, @"disk1/INT01080.710"));
        memory.add(RomFile("INT13080", 0x00008000, @"disk1/INT13080.710"));
        memory.add(RomFile("EXT03080", 0x00400000, @"disk1/EXT03080.710"));
        memory.add(RomFile("EXT34080", 0x004c0000, @"disk2/EXT34080.710"));

        memory.add(new Ram("SPC     ", 0x00800000, 0x00100000));
        memory.add(new Ram("LCDCM   ", 0x00900000, 0x00100000));
        memory.add(new Ram("LCDCIO  ", 0x00a00000, 0x00100000));
        memory.add(new Ram("TGL     ", 0x00b00000, 0x00100000));
        memory.add(new Ram("MOSS    ", 0x00d00000, 0x00100000));
        memory.add(new Ram("FDC     ", 0x00e00000, 0x00100000));
        memory.add(new SCU("SCU     ", 0x00f00000));
        memory.add(new Ram("DRAM    ", 0x01000000, 0x01000000));
        memory.add(new Ram("CHIPRAM ", 0xfffff000, 0x00001000));
        
        sh2.irq_callback = sh2_irq_cb;

        sh2.write8 = [](uint32_t a, uint32_t d, SH2 *sh2) {
            memory.write8(a, d);
        };
        
        sh2.write16 = [](uint32_t a, uint32_t d, SH2 *sh2) {
            memory.write16(a, d);
        };
        
        sh2.write32 = [](uint32_t a, uint32_t d, SH2 *sh2) {
            memory.write32(a, d);
        };
        
        sh2.read8 = [](uint32_t a, SH2 *sh2) {
            return memory.read8(a);
        };

        sh2.read16 = [](uint32_t a, SH2 *sh2) {
            return memory.read16(a);
        };

        sh2.read32 = [](uint32_t a, SH2 *sh2) {
            return memory.read32(a);
        };
        
        sh2_reset(&sh2);
        sh2_execute(&sh2, 10000000, 0);
    }
    return 0;
}
