//
//  machine.h
//  sh2
//
//  Created by Antonio Malara on 26/02/2018.
//  Copyright © 2018 Antonio Malara. All rights reserved.
//

#ifndef machine_h
#define machine_h

#import <Foundation/Foundation.h>
#import <vector>
#import <map>
#import <functional>

extern "C" {
#import "sh2.h"
    
    unsigned DasmSH2(char *buffer, unsigned pc, uint16_t opcode);
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
public:
    uint8_t * data;

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

class LCDCM : public Ram {
public:
    LCDCM(const char * name, const uint32_t start, const uint32_t length)
    : Ram(name, start, length)
    {
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        return Ram::relative_read(address);
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        Ram::relative_write(address, v);
        if (v) {
            printf("");
        }
    }
};

class LCDCIO : public Ram {
    bool dirty = false;
    
public:
    LCDCIO(const char * name, const uint32_t start, const uint32_t length)
    : Ram(name, start, length)
    {
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        return Ram::relative_read(address);
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        Ram::relative_write(address, v);
        if (v) {
            dirty = true;
        }
    }
    
    bool isDirty() { return dirty; }
    void clearDirty() { dirty = false; }
};

class TGL : public Ram {
public:
    TGL(const char * name, const uint32_t start, const uint32_t length)
    : Ram(name, start, length)
    {
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        if (address == 0) {
            return 4;
        }
        return Ram::relative_read(address);
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        Ram::relative_write(address, v);
        if (v) {
            printf("");
        }
        
        if (address == 0x609 && v == 0x01) {
            Ram::relative_write(address, 0);
        }
        
        if (address == 0xe09 && v == 0x01) {
            Ram::relative_write(address, 0);
        }
        
    }
};

class MOSS : public Memory {
public:
    int i = 0;
    
    MOSS(const char * name, const uint32_t start, const uint32_t length)
    : Memory(name, start, length)
    {
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        return i++;
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        ;
    }
};


struct flash_command {
    uint16_t addr;
    uint8_t  data;
};

flash_command readreset[] = {
    { 0x5555, 0xaa },
    { 0x2aaa, 0x55 },
    { 0x5555, 0xf0 },
    { 0xffff, 0xff },
};

int readreset_cur[] = {0, 0};

flash_command sector_erase[] = {
    { 0x5555, 0xaa },
    { 0x2aaa, 0x55 },
    { 0x5555, 0x80 },
    { 0x5555, 0xaa },
    { 0x2aaa, 0x55 },
    { 0xffff, 0x30 },
};

int sector_erase_cur[] = {0, 0};


bool sector_erase_wants_write = false;



class Flash : public Memory {
    uint8_t * data;
    
public:
    Flash(const char * name, const uint32_t start, const uint32_t length, uint8_t * d)
    : Memory(name, start, length)
    {
        data = (uint8_t *)malloc(length);
        memcpy(data, d, length);
    }
    
    ~Flash() {
        free(data);
    }
    
    virtual uint8_t relative_read(uint32_t address) {
        if ((address == 0) ||
            (address == 1) ||
            (address == 2) ||
            (address == 3))
        {
            printf("status register\n");
        }

        
//        if (address == 0) return 0x00;
//        if (address == 1) return 0x80;
//        if (address == 2) return 0x00;
//        if (address == 3) return 0x80;
        
        return data[address];
    }
    
    virtual void relative_write(uint32_t address, uint8_t v) {
        data[address] = v;
        
        auto doit  = (address & 1);
        auto flash = (address & 0x0002) >> 1;
        auto reg   = (address >> 2);
        
        if (doit)
        {
            printf("flash control %08x %d %04x %02x\n", (address), flash, reg, v);
            
            if ((reg  == readreset[readreset_cur[flash]].addr) &&
                (v    == readreset[readreset_cur[flash]].data))
            {
                readreset_cur[flash]++;
            }
            
            if (readreset_cur[flash] > (sizeof(readreset) / sizeof(flash_command))) {
                readreset_cur[flash] = 0;
            }

            auto wanted_reg = sector_erase[sector_erase_cur[flash]].addr;
            auto wanted_data = sector_erase[sector_erase_cur[flash]].data;
            
            if ((reg  == wanted_reg) &&
                (v    == wanted_data))
            {
                sector_erase_cur[flash]++;
            }

            if (0x30   == wanted_data)
            {
                printf("flash sector erase %08x", address);
                sector_erase_cur[flash]++;
            }

            
            if (sector_erase_cur[flash] > (sizeof(readreset) / sizeof(flash_command))) {
                sector_erase_cur[flash] = 0;
            }
            
            sector_erase_wants_write = (sector_erase[sector_erase_cur[flash]].addr == 0xffff);
        }
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
    
public:
    void add(Memory * memory) {
        memoryList.push_back(memory);
    }
    
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
};

Rom * RomFile(const char * name, const uint32_t start, NSString * file) {
    auto data = [NSData dataWithContentsOfFile:file];
    auto length = (const uint32_t) (data.length - 2);
    
    return new Rom(name, start, length, ((uint8_t *)data.bytes + 2));
}

Flash * FlashFile(const char * name, const uint32_t start, NSString * file) {
    auto data = [NSData dataWithContentsOfFile:file];
    auto length = (const uint32_t) (data.length - 2);
    
    return new Flash(name, start, length, ((uint8_t *)data.bytes + 2));
}

struct CPU {
    SH2 sh2;
    
    AddressSpace * addressSpace;
    
    map<uint32_t, function<uint32_t(uint32_t, uint8_t)>> patches_8;
    map<uint32_t, function<uint32_t(uint32_t, uint16_t)>> patches_16;
    map<uint32_t, function<uint32_t(uint32_t, uint32_t)>> patches_32;
    
    CPU()
    {
        sh2_init(&sh2);
        sh2.context = this;
        
        sh2.read8   = CPU::s_read8;
        sh2.read16  = CPU::s_read16;
        sh2.read32  = CPU::s_read32;
        sh2.write8  = CPU::s_write8;
        sh2.write16 = CPU::s_write16;
        sh2.write32 = CPU::s_write32;
        
        sh2.irq_callback = CPU::s_irq_cb;
    }
    
    void reset() {
        sh2_reset(&sh2);
    }
    
    void step() {
        sh2_execute_interpreter(&sh2, 1, 1);
    }
    
private:
    uint8_t read(uint32_t a) {
        if (a == 0xFFFFF844) {
            return 2;
        }
        if (addressSpace)
            return addressSpace->read(a);
        else
            return 0;
    }
    
    void write(uint32_t a, uint8_t v) {
        if (a == 0xFFFFF844) {
            printf("");
        }

        if (addressSpace)
            addressSpace->write(a, v);
    }
    
    void log(const char * rw,
             const char * s,
             uint32       a,
             uint32       d)
    {
        return;
        
        // LCDCIO
        if (sh2.pc == 0x00000656)
            return;
        
        if ((sh2.pc == 0x000006cc) || (sh2.pc == 0x000006ce))
            return;
        
        if ((sh2.pc == 0x000006e8) || (sh2.pc == 0x000006ea))
            return;
        
        static bool x = false;
        
        // DRAM clear
        if (sh2.pc == 0x005a58f4)
            return;
        
        if (x) {
        if (sh2.ppc == 0x0001577a)
            return;
        
        if (sh2.ppc == 0x0001577c)
            return;
        }
        
        const char * name;
        const char * space;
        
        if (a == sh2.pc || a == sh2.ppc)
            return;
        
        auto mem = addressSpace->memoryAtAddress(a);
        
        if (mem) {
            space = mem->name;
        }
        else {
            space = "!*!*!*!*";
        }
        
        name = name_for(a);
        printf("pc: %08x )  %s %s at %08x : %s %8x %s\n",
               sh2.ppc,
               rw,
               space,
               a,
               s,
               d,
               name_for(a));
        
        if (a == 0x0040aaa8) {
            printf("");
        }

        if (a == 0x00415554) {
            printf("");
        } 
    }
    
    
private:
    uint32_t read8(uint32_t a) {
        uint32_t d = read(a);
        
        auto t = patches_8.find(a);
        if (t != patches_8.end()) {
            d = t->second(a, d);
        }
        
        log(print_read, print_byte, a, d);
        return d;
    }
    
    uint32_t read16(uint32_t a) {
        uint32_t d = (((read(a    ) & 0xff) << 8) +
                      ((read(a + 1) & 0xff)     ));

        auto t = patches_16.find(a);
        if (t != patches_16.end()) {
            d = t->second(a, d);
        }

        log(print_read, print_word, a, d);
        return d;
    }
    
    uint32_t read32(uint32_t a) {
        uint32_t d = (((read(a    ) & 0xff) << 24) +
                      ((read(a + 1) & 0xff) << 16) +
                      ((read(a + 2) & 0xff) <<  8) +
                      ((read(a + 3) & 0xff)      ));
        
        auto t = patches_32.find(a);
        if (t != patches_32.end()) {
            d = t->second(a, d);
        }

        
        if (sh2.pc == 0x0001577e) {
            printf("pc: %08x\n", sh2.pc);
            
            auto t = sh2.r[5];
            printf("t: %08x\n", t);
            return t;
        }
        
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
        
        if (sector_erase_wants_write && ((a & 0xffc00000) == 0x00400000)) {
            printf("flash sector erase %08x %02x\n", a, d);
        }
        
        write(a,     (d & 0xff000000) >> 24);
        write(a + 1, (d & 0x00ff0000) >> 16);
        write(a + 2, (d & 0x0000ff00) >>  8);
        write(a + 3, (d & 0x000000ff)      );
    }
    
    static uint32_t s_read8(uint32_t a, SH2 * sh2) {
        return ((CPU *)sh2->context)->read8(a);
    }
    
    static uint32_t s_read16(uint32_t a, SH2 * sh2) {
        return ((CPU *)sh2->context)->read16(a);
    }
    
    static uint32_t s_read32(uint32_t a, SH2 * sh2) {
        return ((CPU *)sh2->context)->read32(a);
    }
    
    static void s_write8(uint32_t a, uint32_t d, SH2 * sh2) {
        return ((CPU *)sh2->context)->write8(a, d);
    }
    
    static void s_write16(uint32_t a, uint32_t d, SH2 * sh2) {
        return ((CPU *)sh2->context)->write16(a, d);
    }
    
    static void s_write32(uint32_t a, uint32_t d, SH2 * sh2) {
        return ((CPU *)sh2->context)->write32(a, d);
    }
    
    static int s_irq_cb(SH2 *sh2, int level)
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
};

struct Machine {
    CPU cpu;
    AddressSpace memory;
    
    LCDCIO * lcdcio;
    
    void (^frameCallback)(const uint8_t * data, const size_t length);
    
    Machine() {
        cpu.addressSpace = &memory;
        
        memory.add(RomFile    ("INT01080", 0x00000000, @"/Users/willy/Sources/Triton-RE/Triton OS Releases/Triton OS 2.0.0/disk1/INT01080.710"));
        memory.add(RomFile    ("INT13080", 0x00008000, @"/Users/willy/Sources/Triton-RE/Triton OS Releases/Triton OS 2.0.0/disk1/INT13080.710"));
        
        memory.add(FlashFile  ("EXT03080", 0x00400000, @"/Users/willy/Sources/Triton-RE/Triton OS Releases/Triton OS 2.0.0/disk1/EXT03080.710"));
        memory.add(FlashFile  ("EXT34080", 0x004c0000, @"/Users/willy/Sources/Triton-RE/Triton OS Releases/Triton OS 2.0.0/disk2/EXT34080.710"));
        memory.add(FlashFile  ("EXT74080", 0x005c0000, @"/Users/willy/Sources/Triton-RE/Triton OS Releases/Triton OS 2.0.0/disk3/EXT74080.710"));
        
        auto spc       = new Ram      ("SPC     ", 0x00800000, 0x00100000);
        auto lcdcm     = new LCDCM    ("LCDCM   ", 0x00900000, 0x00100000);
             lcdcio    = new LCDCIO   ("LCDCIO  ", 0x00a00000, 0x00100000);
        auto tgl       = new TGL      ("TGL     ", 0x00b00000, 0x00100000);
        auto moss      = new MOSS     ("MOSS    ", 0x00d00000, 0x00100000);
        auto fdc       = new Ram      ("FDC     ", 0x00e00000, 0x00100000);
        auto scu       = new Ram      ("SCU     ", 0x00f00000, 0x00000002);
        auto dram      = new Ram      ("DRAM    ", 0x01000000, 0x01000000);
        auto chipram   = new Ram      ("CHIPRAM ", 0xfffff000, 0x00001000);
        
        memory.add(spc);
        memory.add(lcdcm);
        memory.add(lcdcio);
        memory.add(tgl);
        memory.add(moss);
        memory.add(fdc);
        memory.add(scu);
        memory.add(dram);
        memory.add(chipram);
        
        cpu.sh2.pc = cpu.sh2.read32(0, &cpu.sh2);
        
        static uint32_t serial_ack    = 0;
        static uint32_t serial_cnt    = 0;
        static uint32_t serial_data[] = { 0x66, 0x31 };
        
        cpu.patches_8 = {
            { 0x00f00001, [&](auto a, auto v){
                return serial_ack++;
            }},
            
            { 0x00f00000, [&](auto a, auto v){
                auto b = serial_data[serial_cnt];
                
                serial_cnt++;
                serial_cnt %= sizeof(serial_data) / sizeof(uint32_t);
                
                return b;
            }},
        };
        
        cpu.patches_16 = {
            { 0xffff835a, [&](auto a, auto v) { return 0xffff; }},
        };
        
        cpu.patches_32 = {
            { 0xFFFFF844, [&](auto a, auto v) { return 0xa; }},
        };
    }
    
    void run() {
        bool trace = false;
        uint32_t saved_sp = 0;
        
        while (true) {
            cpu.step();
            
            if (cpu.sh2.ppc == 0x00469C34) {
                {
                    char cstring[200] = {0};
                    
                    auto _x = cpu.sh2.r[4];
                    auto _y = cpu.sh2.r[5];
                    auto string = cpu.sh2.r[6];
                    
                    for (int i = 0; i < sizeof(cstring); i++) {
                        auto c = memory.read(string + i);
                        cstring[i] = c;
                        
                        if (c == 0)
                            break;
                    }
                    
                    printf(" MAYBE PRINT > %3d %3d: %s\n", _x, _y, cstring);
                }
                
                if (cpu.sh2.pc == 0x00016678)
                    trace = true;
                saved_sp = cpu.sh2.r[15];
            }
            
            if (saved_sp && cpu.sh2.r[15] > saved_sp) {
                trace = false;
                saved_sp = 0;
            }
            
            char buff[256];
            auto opcode = ((memory.read(cpu.sh2.ppc)     << 8) +
                           (memory.read(cpu.sh2.ppc + 1))    );
            
            DasmSH2(buff, cpu.sh2.ppc, opcode);
            
            if (trace) {
                printf("pc: %08x -- [ %04x ] -- %s\n", cpu.sh2.ppc, opcode, buff);
            }

            if ((opcode & 0x0000ff00) == 0x0000c300) {
                printf("pc: %08x ) TRAPA %08x -- r0: %02x\n", cpu.sh2.ppc, opcode, cpu.sh2.r[0]);
            }
            
            if (lcdcio->isDirty() && frameCallback) {
                frameCallback(lcdcio->data, 320 * 240 * 8);
                lcdcio->clearDirty();
            }
          
            static uint32_t bp = 0x00497D94;
            
            if (cpu.sh2.pc  == bp) {
                trace = true;
            }
//            if (cpu.sh2.pc  == 0x0000af10) {
//                cpu.sh2.r[0] = 0xffffffbd;
//                cpu.sh2.pc = cpu.sh2.vbr + 4 * 65;
//                trace = true;
//
//            }
        }
    }
};

#endif /* machine_h */
