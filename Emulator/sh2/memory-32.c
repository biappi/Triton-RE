/*
 * PicoDrive
 * (C) notaz, 2009,2010,2013
 *
 * This work is licensed under the terms of MAME license.
 * See COPYING file in the top-level directory.
 *
 * Register map:
 * a15100 F....... R.....EA  F.....AC N...VHMP 4000 // Fm Ren nrEs Aden Cart heN V H cMd Pwm
 * a15102 ........ ......SM  ?                 4002 // intS intM
 * a15104 ........ ......10  ........ hhhhhhhh 4004 // bk1 bk0 Hint
 * a15106 ........ F....SDR  UE...... .....SDR 4006 // Full 68S Dma Rv fUll[fb] Empt[fb]
 * a15108           (32bit DREQ src)           4008
 * a1510c           (32bit DREQ dst)           400c
 * a15110          llllllll llllll00           4010 // DREQ Len
 * a15112           (16bit FIFO reg)           4012
 * a15114 0                  (16bit VRES clr)  4014
 * a15116 0                  (16bit Vint clr)  4016
 * a15118 0                  (16bit Hint clr)  4018
 * a1511a .......? .......C  (16bit CMD clr)   401a // TV Cm
 * a1511c 0                  (16bit PWM clr)   401c
 * a1511e 0                  ?                 401e
 * a15120            (16 bytes comm)           2020
 * a15130                 (PWM)                2030
 *
 * SH2 addr lines:
 * iii. .cc. ..xx *   // Internal, Cs, x
 *
 * sh2 map, wait/bus cycles (from docs):
 *                             r    w
 * rom      0000000-0003fff    1    -
 * sys reg  0004000-00040ff    1    1
 * vdp reg  0004100-00041ff    5    5
 * vdp pal  0004200-00043ff    5    5
 * cart     2000000-23fffff     6-15
 * dram/fb  4000000-401ffff 5-12  1-3
 * fb ovr   4020000-403ffff
 * sdram    6000000-603ffff   12    2  (cycles)
 * d.a.    c0000000-?
 */
//#include "../pico_int.h"
#include "memory.h"
//#include "compiler.h"

#include "sh2.h"


#include <stdio.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  uptr; // unsigned pointer-sized int

unsigned char  SDRAM[0x40000];
unsigned short DRAM[2][0x20000/2];    // AKA fb


# define sh2_end_run(sh2, after_) do { \
if ((sh2)->icount > (after_)) { \
(sh2)->cycles_timeslice -= (sh2)->icount - (after_); \
(sh2)->icount = after_; \
} \
} while (0)
# define sh2_cycles_left(sh2) (sh2)->icount
# define sh2_burn_cycles(sh2, n) (sh2)->icount -= n
# define sh2_pc(sh2) (sh2)->ppc


# define sh2_cycles_left(sh2) (sh2)->icount

#define pevt_log_sh2(sh2, e)
#define pevt_log_sh2_o(sh2, e)
#define sh2_cycles_done(sh2) ((int)(sh2)->cycles_timeslice - sh2_cycles_left(sh2))

#define SH2_READ_SHIFT 25
#define SH2_WRITE_SHIFT 25

#define MAP_FLAG ((uptr)1 << (sizeof(uptr) * 8 - 1))

#define map_flag_set(x) ((x) & MAP_FLAG)

// 32x
typedef struct {
    uptr addr; // stores (membase >> 1) or ((handler >> 1) | (1<<31))
    u32 mask;
} sh2_memmap;

typedef u32 (sh2_read_handler)(u32 a, SH2 *sh2);
typedef void REGPARM(3) (sh2_write_handler)(u32 a, u32 d, SH2 *sh2);

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


// --------- //

static const char str_mars[] = "MARS";

void *p32x_bios_g, *p32x_bios_m, *p32x_bios_s;

static void bank_switch(int b);

// addressing byte in 16bit reg
#define REG8IN16(ptr, offs) ((u8 *)ptr)[(offs) ^ 1]

// poll detection
#define POLL_THRESHOLD 3


static void sh2_poll_detect(SH2 *sh2, u32 a, u32 flags, int maxcnt)
{
  int cycles_left = sh2_cycles_left(sh2);

  if (a == sh2->poll_addr && sh2->poll_cycles - cycles_left <= 10) {
    if (sh2->poll_cnt++ > maxcnt) {
      if (!(sh2->state & flags))
        printf("state: %02x->%02x",
          sh2->state, sh2->state | flags);

      sh2->state |= flags;
      sh2_end_run(sh2, 1);
      pevt_log_sh2(sh2, EVT_POLL_START);
      return;
    }
  }
  else
    sh2->poll_cnt = 0;
  sh2->poll_addr = a;
  sh2->poll_cycles = cycles_left;
}

void p32x_sh2_poll_event(SH2 *sh2, u32 flags, u32 m68k_cycles)
{
  if (sh2->state & flags) {
    printf("state: %02x->%02x", sh2->state,
      sh2->state & ~flags);

//    if (sh2->m68krcycles_done < m68k_cycles)
//      sh2->m68krcycles_done = m68k_cycles;

    pevt_log_sh2_o(sh2, EVT_POLL_END);
  }

  sh2->state &= ~flags;
  sh2->poll_addr = sh2->poll_cycles = sh2->poll_cnt = 0;
}

static void sh2s_sync_on_read(SH2 *sh2)
{
  int cycles;
  if (sh2->poll_cnt != 0)
    return;

  cycles = sh2_cycles_done(sh2);
//  if (cycles > 600)
//    p32x_sync_other_sh2(sh2, sh2->m68krcycles_done + cycles / 3);
}

// SH2 faking
//#define FAKE_SH2
#ifdef FAKE_SH2
static int p32x_csum_faked;
static const u16 comm_fakevals[] = {
  0x4d5f, 0x4f4b, // M_OK
  0x535f, 0x4f4b, // S_OK
  0x4D41, 0x5346, // MASF - Brutal Unleashed
  0x5331, 0x4d31, // Darxide
  0x5332, 0x4d32,
  0x5333, 0x4d33,
  0x0000, 0x0000, // eq for doom
  0x0002, // Mortal Kombat
//  0, // pad
};

static u32 sh2_comm_faker(u32 a)
{
  static int f = 0;
  if (a == 0x28 && !p32x_csum_faked) {
    p32x_csum_faked = 1;
    return *(unsigned short *)(Pico.rom + 0x18e);
  }
  if (f >= sizeof(comm_fakevals) / sizeof(comm_fakevals[0]))
    f = 0;
  return comm_fakevals[f++];
}
#endif

// ------------------------------------------------------------------
// 68k regs

static u32 p32x_reg_read16(u32 a)
{
  a &= 0x3e;

#if 0
  if ((a & 0x30) == 0x20)
    return sh2_comm_faker(a);
#else
  if ((a & 0x30) == 0x20) {
      printf("reg read16 %x", a);
      return 0;
  }
#endif

    /*
  if (a == 2) { // INTM, INTS
    unsigned int cycles = SekCyclesDone();
    if (cycles - msh2.m68krcycles_done > 64)
      p32x_sync_sh2s(cycles);
    goto out;
  }
*/
    
    if ((a & 0x30) == 0x30) {
      printf("reg pwn read16 %x", a);
    return 0;
    //return p32x_pwm_read16(a, NULL, SekCyclesDone());
    }
    
    printf("reg read16 %x", a);
    return 0;
}

static void dreq0_write(u16 *r, u32 d)
{
    printf(__PRETTY_FUNCTION__);
    /*
  if (!(r[6 / 2] & P32XS_68S)) {
    elprintf(EL_32X|EL_ANOMALY, "DREQ FIFO w16 without 68S?");
    return; // ignored - tested
  }
  if (Pico32x.dmac0_fifo_ptr < DMAC_FIFO_LEN) {
    Pico32x.dmac_fifo[Pico32x.dmac0_fifo_ptr++] = d;
    if (Pico32x.dmac0_fifo_ptr == DMAC_FIFO_LEN)
      r[6 / 2] |= P32XS_FULL;
    // tested: len register decrements and 68S clears
    // even if SH2s/DMAC aren't active..
    r[0x10 / 2]--;
    if (r[0x10 / 2] == 0)
      r[6 / 2] &= ~P32XS_68S;

    if ((Pico32x.dmac0_fifo_ptr & 3) == 0) {
      p32x_sync_sh2s(SekCyclesDone());
      p32x_dreq0_trigger();
    }
  }
  else
    elprintf(EL_32X|EL_ANOMALY, "DREQ FIFO overflow!");
     */
}

// writable bits tested
static void p32x_reg_write8(u32 a, u32 d)
{
    printf("%s - %8x -- %x", __PRETTY_FUNCTION__, a, d);
    
    /*
  u16 *r = Pico32x.regs;
  a &= 0x3f;

  // for things like bset on comm port
  m68k_poll.cnt = 0;

  switch (a) {
    case 0x00: // adapter ctl: FM writable
      REG8IN16(r, 0x00) = d & 0x80;
      return;
    case 0x01: // adapter ctl: RES and ADEN writable
      if ((d ^ r[0]) & d & P32XS_nRES)
        p32x_reset_sh2s();
      REG8IN16(r, 0x01) &= ~(P32XS_nRES|P32XS_ADEN);
      REG8IN16(r, 0x01) |= d & (P32XS_nRES|P32XS_ADEN);
      return;
    case 0x02: // ignored, always 0
      return;
    case 0x03: // irq ctl
      if ((d ^ r[0x02 / 2]) & 3) {
        int cycles = SekCyclesDone();
        p32x_sync_sh2s(cycles);
        r[0x02 / 2] = d & 3;
        p32x_update_cmd_irq(NULL, cycles);
      }
      return;
    case 0x04: // ignored, always 0
      return;
    case 0x05: // bank
      d &= 3;
      if (r[0x04 / 2] != d) {
        r[0x04 / 2] = d;
        bank_switch(d);
      }
      return;
    case 0x06: // ignored, always 0
      return;
    case 0x07: // DREQ ctl
      REG8IN16(r, 0x07) &= ~(P32XS_68S|P32XS_DMA|P32XS_RV);
      if (!(d & P32XS_68S)) {
        Pico32x.dmac0_fifo_ptr = 0;
        REG8IN16(r, 0x07) &= ~P32XS_FULL;
      }
      REG8IN16(r, 0x07) |= d & (P32XS_68S|P32XS_DMA|P32XS_RV);
      return;
    case 0x08: // ignored, always 0
      return;
    case 0x09: // DREQ src
      REG8IN16(r, 0x09) = d;
      return;
    case 0x0a:
      REG8IN16(r, 0x0a) = d;
      return;
    case 0x0b:
      REG8IN16(r, 0x0b) = d & 0xfe;
      return;
    case 0x0c: // ignored, always 0
      return;
    case 0x0d: // DREQ dest
    case 0x0e:
    case 0x0f:
    case 0x10: // DREQ len
      REG8IN16(r, a) = d;
      return;
    case 0x11:
      REG8IN16(r, a) = d & 0xfc;
      return;
    // DREQ FIFO - writes to odd addr go to fifo
    // do writes to even work? Reads return 0
    case 0x12:
      REG8IN16(r, a) = d;
      return;
    case 0x13:
      d = (REG8IN16(r, 0x12) << 8) | (d & 0xff);
      REG8IN16(r, 0x12) = 0;
      dreq0_write(r, d);
      return;
    case 0x14: // ignored, always 0
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
      return;
    case 0x1a: // what's this?
      elprintf(EL_32X|EL_ANOMALY, "mystery w8 %02x %02x", a, d);
      REG8IN16(r, a) = d & 0x01;
      return;
    case 0x1b: // TV
      REG8IN16(r, a) = d & 0x01;
      return;
    case 0x1c: // ignored, always 0
    case 0x1d:
    case 0x1e:
    case 0x1f:
    case 0x30:
      return;
    case 0x31: // PWM control
      REG8IN16(r, a) &= ~0x0f;
      REG8IN16(r, a) |= d & 0x0f;
      d = r[0x30 / 2];
      goto pwm_write;
    case 0x32: // PWM cycle
      REG8IN16(r, a) = d & 0x0f;
      d = r[0x32 / 2];
      goto pwm_write;
    case 0x33:
      REG8IN16(r, a) = d;
      d = r[0x32 / 2];
      goto pwm_write;
    // PWM pulse regs.. Only writes to odd address send a value
    // to FIFO; reads are 0 (except status bits)
    case 0x34:
    case 0x36:
    case 0x38:
      REG8IN16(r, a) = d;
      return;
    case 0x35:
    case 0x37:
    case 0x39:
      d = (REG8IN16(r, a ^ 1) << 8) | (d & 0xff);
      REG8IN16(r, a ^ 1) = 0;
      goto pwm_write;
    case 0x3a: // ignored, always 0
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
      return;
    pwm_write:
      p32x_pwm_write16(a & ~1, d, NULL, SekCyclesDone());
      return;
  }

  if ((a & 0x30) == 0x20) {
    int cycles = SekCyclesDone();
    int comreg;
    
    if (REG8IN16(r, a) == d)
      return;

    comreg = 1 << (a & 0x0f) / 2;
    if (Pico32x.comm_dirty_68k & comreg)
      p32x_sync_sh2s(cycles);

    REG8IN16(r, a) = d;
    p32x_sh2_poll_event(&sh2s[0], SH2_STATE_CPOLL, cycles);
    p32x_sh2_poll_event(&sh2s[1], SH2_STATE_CPOLL, cycles);
    Pico32x.comm_dirty_68k |= comreg;

    if (cycles - (int)msh2.m68krcycles_done > 120)
      p32x_sync_sh2s(cycles);
    return;
  }
     */
}

static void p32x_reg_write16(u32 a, u32 d)
{
    printf("%s - %8x -- %x", __PRETTY_FUNCTION__, a, d);
/*
  u16 *r = Pico32x.regs;
  a &= 0x3e;

  // for things like bset on comm port
  m68k_poll.cnt = 0;

  switch (a) {
    case 0x00: // adapter ctl
      if ((d ^ r[0]) & d & P32XS_nRES)
        p32x_reset_sh2s();
      r[0] &= ~(P32XS_FM|P32XS_nRES|P32XS_ADEN);
      r[0] |= d & (P32XS_FM|P32XS_nRES|P32XS_ADEN);
      return;
    case 0x08: // DREQ src
      r[a / 2] = d & 0xff;
      return;
    case 0x0a:
      r[a / 2] = d & ~1;
      return;
    case 0x0c: // DREQ dest
      r[a / 2] = d & 0xff;
      return;
    case 0x0e:
      r[a / 2] = d;
      return;
    case 0x10: // DREQ len
      r[a / 2] = d & ~3;
      return;
    case 0x12: // FIFO reg
      dreq0_write(r, d);
      return;
    case 0x1a: // TV + mystery bit
      r[a / 2] = d & 0x0101;
      return;
    case 0x30: // PWM control
      d = (r[a / 2] & ~0x0f) | (d & 0x0f);
      r[a / 2] = d;
      p32x_pwm_write16(a, d, NULL, SekCyclesDone());
      return;
  }

  // comm port
  if ((a & 0x30) == 0x20) {
    int cycles = SekCyclesDone();
    int comreg;
    
    if (r[a / 2] == d)
      return;

    comreg = 1 << (a & 0x0f) / 2;
    if (Pico32x.comm_dirty_68k & comreg)
      p32x_sync_sh2s(cycles);

    r[a / 2] = d;
    p32x_sh2_poll_event(&sh2s[0], SH2_STATE_CPOLL, cycles);
    p32x_sh2_poll_event(&sh2s[1], SH2_STATE_CPOLL, cycles);
    Pico32x.comm_dirty_68k |= comreg;

    if (cycles - (int)msh2.m68krcycles_done > 120)
      p32x_sync_sh2s(cycles);
    return;
  }
  // PWM
  else if ((a & 0x30) == 0x30) {
    p32x_pwm_write16(a, d, NULL, SekCyclesDone());
    return;
  }

  p32x_reg_write8(a + 1, d);
 */
}

// ------------------------------------------------------------------
// VDP regs
static u32 p32x_vdp_read16(u32 a)
{
    printf("%s - %8x", __PRETTY_FUNCTION__, a);
    return 0;
    /*

  u32 d;
  a &= 0x0e;

  d = Pico32x.vdp_regs[a / 2];
  if (a == 0x0a) {
    // tested: FEN seems to be randomly pulsing on hcnt 0x80-0xf0,
    // most often at 0xb1-0xb5, even during vblank,
    // what's the deal with that?
    // we'll just fake it along with hblank for now
    Pico32x.vdp_fbcr_fake++;
    if (Pico32x.vdp_fbcr_fake & 4)
      d |= P32XV_HBLK;
    if ((Pico32x.vdp_fbcr_fake & 7) == 0)
      d |= P32XV_nFEN;
  }
  return d;
     */
}

static void p32x_vdp_write8(u32 a, u32 d)
{
    printf("%s - %8x -- %x", __PRETTY_FUNCTION__, a, d);
/*
  u16 *r = Pico32x.vdp_regs;
  a &= 0x0f;

  // TODO: verify what's writeable
  switch (a) {
    case 0x01:
      // priority inversion is handled in palette
      if ((r[0] ^ d) & P32XV_PRI)
        Pico32x.dirty_pal = 1;
      r[0] = (r[0] & P32XV_nPAL) | (d & 0xff);
      break;
    case 0x03: // shift (for pp mode)
      r[2 / 2] = d & 1;
      break;
    case 0x05: // fill len
      r[4 / 2] = d & 0xff;
      break;
    case 0x0b:
      d &= 1;
      Pico32x.pending_fb = d;
      // if we are blanking and FS bit is changing
      if (((r[0x0a/2] & P32XV_VBLK) || (r[0] & P32XV_Mx) == 0) && ((r[0x0a/2] ^ d) & P32XV_FS)) {
        r[0x0a/2] ^= P32XV_FS;
        Pico32xSwapDRAM(d ^ 1);
        elprintf(EL_32X, "VDP FS: %d", r[0x0a/2] & P32XV_FS);
      }
      break;
  }
 */
}

static void p32x_vdp_write16(u32 a, u32 d, SH2 *sh2)
{
    printf("%s - %8x -- %x", __PRETTY_FUNCTION__, a, d);
/*
  a &= 0x0e;
  if (a == 6) { // fill start
    Pico32x.vdp_regs[6 / 2] = d;
    return;
  }
  if (a == 8) { // fill data
    u16 *dram = Pico32xMem->dram[(Pico32x.vdp_regs[0x0a/2] & P32XV_FS) ^ 1];
    int len = Pico32x.vdp_regs[4 / 2] + 1;
    int len1 = len;
    a = Pico32x.vdp_regs[6 / 2];
    while (len1--) {
      dram[a] = d;
      a = (a & 0xff00) | ((a + 1) & 0xff);
    }
    Pico32x.vdp_regs[0x06 / 2] = a;
    Pico32x.vdp_regs[0x08 / 2] = d;
    if (sh2 != NULL && len > 4) {
      Pico32x.vdp_regs[0x0a / 2] |= P32XV_nFEN;
      // supposedly takes 3 bus/6 sh2 cycles? or 3 sh2 cycles?
      p32x_event_schedule_sh2(sh2, P32X_EVENT_FILLEND, 3 + len);
    }
    return;
  }

  p32x_vdp_write8(a | 1, d);
 */
}

// ------------------------------------------------------------------
// SH2 regs

static u32 p32x_sh2reg_read16(u32 a, SH2 *sh2)
{
    printf("%s - %8x", __PRETTY_FUNCTION__, a);
    return 0;
    
/*
  u16 *r = Pico32x.regs;
  a &= 0x3e;

  switch (a) {
    case 0x00: // adapter/irq ctl
      return (r[0] & P32XS_FM) | Pico32x.sh2_regs[0]
        | Pico32x.sh2irq_mask[sh2->is_slave];
    case 0x04: // H count (often as comm too)
      sh2_poll_detect(sh2, a, SH2_STATE_CPOLL, 3);
      sh2s_sync_on_read(sh2);
      return Pico32x.sh2_regs[4 / 2];
    case 0x06:
      return (r[a / 2] & ~P32XS_FULL) | 0x4000;
    case 0x08: // DREQ src
    case 0x0a:
    case 0x0c: // DREQ dst
    case 0x0e:
    case 0x10: // DREQ len
      return r[a / 2];
    case 0x12: // DREQ FIFO - does this work on hw?
      if (Pico32x.dmac0_fifo_ptr > 0) {
        Pico32x.dmac0_fifo_ptr--;
        r[a / 2] = Pico32x.dmac_fifo[0];
        memmove(&Pico32x.dmac_fifo[0], &Pico32x.dmac_fifo[1],
          Pico32x.dmac0_fifo_ptr * 2);
      }
      return r[a / 2];
    case 0x14:
    case 0x16:
    case 0x18:
    case 0x1a:
    case 0x1c:
      return 0; // ?
  }

  // comm port
  if ((a & 0x30) == 0x20) {
    int comreg = 1 << (a & 0x0f) / 2;
    if (Pico32x.comm_dirty_68k & comreg)
      Pico32x.comm_dirty_68k &= ~comreg;
    else
      sh2_poll_detect(sh2, a, SH2_STATE_CPOLL, 3);
    sh2s_sync_on_read(sh2);
    return r[a / 2];
  }
  if ((a & 0x30) == 0x30)
    return p32x_pwm_read16(a, sh2, sh2_cycles_done_m68k(sh2));

  elprintf_sh2(sh2, EL_32X|EL_ANOMALY, 
    "unhandled sysreg r16 [%02x] @%08x", a, sh2_pc(sh2));
  return 0;
 */
}

static void p32x_sh2reg_write8(u32 a, u32 d, SH2 *sh2)
{
    printf("%s - %8x -- %x", __PRETTY_FUNCTION__, a, d);
/*
  u16 *r = Pico32x.regs;
  u32 old;

  a &= 0x3f;
  sh2->poll_addr = 0;

  switch (a) {
    case 0x00: // FM
      r[0] &= ~P32XS_FM;
      r[0] |= (d << 8) & P32XS_FM;
      return;
    case 0x01: // HEN/irq masks
      old = Pico32x.sh2irq_mask[sh2->is_slave];
      if ((d ^ old) & 1)
        p32x_pwm_sync_to_sh2(sh2);

      Pico32x.sh2irq_mask[sh2->is_slave] = d & 0x0f;
      Pico32x.sh2_regs[0] &= ~0x80;
      Pico32x.sh2_regs[0] |= d & 0x80;

      if ((d ^ old) & 1)
        p32x_pwm_schedule_sh2(sh2);
      if ((old ^ d) & 2)
        p32x_update_cmd_irq(sh2, 0);
      if ((old ^ d) & 4)
        p32x_schedule_hint(sh2, 0); 
      return;
    case 0x04: // ignored?
      return;
    case 0x05: // H count
      d &= 0xff;
      if (Pico32x.sh2_regs[4 / 2] != d) {
        Pico32x.sh2_regs[4 / 2] = d;
        p32x_sh2_poll_event(sh2->other_sh2, SH2_STATE_CPOLL,
          sh2_cycles_done_m68k(sh2));
        sh2_end_run(sh2, 4);
      }
      return;
    case 0x30:
      REG8IN16(r, a) = d & 0x0f;
      d = r[0x30 / 2];
      goto pwm_write;
    case 0x31: // PWM control
      REG8IN16(r, a) = d & 0x8f;
      d = r[0x30 / 2];
      goto pwm_write;
    case 0x32: // PWM cycle
      REG8IN16(r, a) = d & 0x0f;
      d = r[0x32 / 2];
      goto pwm_write;
    case 0x33:
      REG8IN16(r, a) = d;
      d = r[0x32 / 2];
      goto pwm_write;
    // PWM pulse regs.. Only writes to odd address send a value
    // to FIFO; reads are 0 (except status bits)
    case 0x34:
    case 0x36:
    case 0x38:
      REG8IN16(r, a) = d;
      return;
    case 0x35:
    case 0x37:
    case 0x39:
      d = (REG8IN16(r, a ^ 1) << 8) | (d & 0xff);
      REG8IN16(r, a ^ 1) = 0;
      goto pwm_write;
    case 0x3a: // ignored, always 0?
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
      return;
    pwm_write:
      p32x_pwm_write16(a & ~1, d, sh2, 0);
      return;
  }

  if ((a & 0x30) == 0x20) {
    int comreg;
    if (REG8IN16(r, a) == d)
      return;

    REG8IN16(r, a) = d;
    p32x_m68k_poll_event(P32XF_68KCPOLL);
    p32x_sh2_poll_event(sh2->other_sh2, SH2_STATE_CPOLL,
      sh2_cycles_done_m68k(sh2));
    comreg = 1 << (a & 0x0f) / 2;
    Pico32x.comm_dirty_sh2 |= comreg;
    return;
  }

 */
    printf("unhandled sysreg w8  [%02x] %02x @%08x", a, d, sh2->pc);
}

static void p32x_sh2reg_write16(u32 a, u32 d, SH2 *sh2)
{
    printf("%s - %8x -- %x", __PRETTY_FUNCTION__, a, d);
/*
  a &= 0x3e;

  sh2->poll_addr = 0;

  // comm
  if ((a & 0x30) == 0x20) {
    int comreg;
    if (Pico32x.regs[a / 2] == d)
      return;

    Pico32x.regs[a / 2] = d;
    p32x_m68k_poll_event(P32XF_68KCPOLL);
    p32x_sh2_poll_event(sh2->other_sh2, SH2_STATE_CPOLL,
      sh2_cycles_done_m68k(sh2));
    comreg = 1 << (a & 0x0f) / 2;
    Pico32x.comm_dirty_sh2 |= comreg;
    return;
  }
  // PWM
  else if ((a & 0x30) == 0x30) {
    p32x_pwm_write16(a, d, sh2, sh2_cycles_done_m68k(sh2));
    return;
  }

  switch (a) {
    case 0: // FM
      Pico32x.regs[0] &= ~P32XS_FM;
      Pico32x.regs[0] |= d & P32XS_FM;
      break;
    case 0x14:
      Pico32x.sh2irqs &= ~P32XI_VRES;
      goto irls;
    case 0x16:
      Pico32x.sh2irqi[sh2->is_slave] &= ~P32XI_VINT;
      goto irls;
    case 0x18:
      Pico32x.sh2irqi[sh2->is_slave] &= ~P32XI_HINT;
      goto irls;
    case 0x1a:
      Pico32x.regs[2 / 2] &= ~(1 << sh2->is_slave);
      p32x_update_cmd_irq(sh2, 0);
      return;
    case 0x1c:
      p32x_pwm_sync_to_sh2(sh2);
      Pico32x.sh2irqi[sh2->is_slave] &= ~P32XI_PWM;
      p32x_pwm_schedule_sh2(sh2);
      goto irls;
  }

  p32x_sh2reg_write8(a | 1, d, sh2);
  return;

irls:
  p32x_update_irls(sh2, 0);
 */
}

// ------------------------------------------------------------------
/* quirk: in both normal and overwrite areas only nonzero values go through */
#define sh2_write8_dramN(n) \
  if ((d & 0xff) != 0) { \
    u8 *dram = (u8 *)DRAM[n]; \
    dram[(a & 0x1ffff) ^ 1] = d; \
  }

#define sh2_write16_dramN(n) \
  u16 *pd = &DRAM[n][(a & 0x1ffff) / 2]; \
  if (!(a & 0x20000)) { \
    *pd = d; \
    return; \
  } \
  /* overwrite */ \
  if (!(d & 0xff00)) d |= *pd & 0xff00; \
  if (!(d & 0x00ff)) d |= *pd & 0x00ff; \
  *pd = d;



#define sh2_write16_dramN(n) \
u16 *pd = &DRAM[n][(a & 0x1ffff) / 2]; \
if (!(a & 0x20000)) { \
*pd = d; \
return; \
} \
/* overwrite */ \
if (!(d & 0xff00)) d |= *pd & 0xff00; \
if (!(d & 0x00ff)) d |= *pd & 0x00ff; \
*pd = d;



static void m68k_write16_dram0_ow(u32 a, u32 d)
{
  sh2_write16_dramN(0);
}

static void m68k_write16_dram1_ow(u32 a, u32 d)
{
  sh2_write16_dramN(1);
}

// -----------------------------------------------------------------
//                              SH2  
// -----------------------------------------------------------------

// read8
static u32 sh2_read8_unmapped(u32 a, SH2 *sh2)
{
  printf("unmapped r8  [%08x]       %02x @%06x",
    a, 0, sh2_pc(sh2));
  return 0;
}

static u32 sh2_read8_cs0(u32 a, SH2 *sh2)
{
  u32 d = 0;

  sh2_burn_cycles(sh2, 1*2);

  // 0x3ffc0 is veridied
  if ((a & 0x3ffc0) == 0x4000) {
    d = p32x_sh2reg_read16(a, sh2);
    goto out_16to8;
  }

  if ((a & 0x3fff0) == 0x4100) {
    d = p32x_vdp_read16(a);
    sh2_poll_detect(sh2, a, SH2_STATE_VPOLL, 7);
    goto out_16to8;
  }

  return sh2_read8_unmapped(a, sh2);

out_16to8:
  if (a & 1)
    d &= 0xff;
  else
    d >>= 8;

  printf("r8  [%08x]       %02x @%06x",
    a, d, sh2_pc(sh2));
  return d;
}

static u32 sh2_read8_da(u32 a, SH2 *sh2)
{
  return sh2->data_array[(a & 0xfff) ^ 1];
}

// read16
static u32 sh2_read16_unmapped(u32 a, SH2 *sh2)
{
  printf("unmapped r16 [%08x]     %04x @%06x",
    a, 0, sh2_pc(sh2));
  return 0;
}

static u32 sh2_read16_cs0(u32 a, SH2 *sh2)
{
  
  return sh2_read16_unmapped(a, sh2);
}

static u32 sh2_read16_da(u32 a, SH2 *sh2)
{
  return ((u16 *)sh2->data_array)[(a & 0xfff) / 2];
}

// writes
static void REGPARM(3) sh2_write_ignore(u32 a, u32 d, SH2 *sh2)
{
}

// write8
static void REGPARM(3) sh2_write8_unmapped(u32 a, u32 d, SH2 *sh2)
{
  printf("unmapped w8  [%08x]       %02x @%06x",
    a, d & 0xff, sh2_pc(sh2));
}

static void REGPARM(3) sh2_write8_cs0(u32 a, u32 d, SH2 *sh2)
{
  printf("w8  [%08x]       %02x @%06x",
    a, d & 0xff, sh2_pc(sh2));

  if ((a & 0x3ffc0) == 0x4000) {
    p32x_sh2reg_write8(a, d, sh2);
    return;
  }

  sh2_write8_unmapped(a, d, sh2);
}

static void REGPARM(3) sh2_write8_dram0(u32 a, u32 d, SH2 *sh2)
{
  sh2_write8_dramN(0);
}

static void REGPARM(3) sh2_write8_dram1(u32 a, u32 d, SH2 *sh2)
{
  sh2_write8_dramN(1);
}

static void REGPARM(3) sh2_write8_sdram(u32 a, u32 d, SH2 *sh2)
{
  u32 a1 = a & 0x3ffff;
#ifdef DRC_SH2
  int t = Pico32xMem->drcblk_ram[a1 >> SH2_DRCBLK_RAM_SHIFT];
  if (t)
    sh2_drc_wcheck_ram(a, t, sh2->is_slave);
#endif
  SDRAM[a1 ^ 1] = d;
}

static void REGPARM(3) sh2_write8_sdram_wt(u32 a, u32 d, SH2 *sh2)
{
  // xmen sync hack..
  if (a < 0x26000200)
    sh2_end_run(sh2, 32);

  sh2_write8_sdram(a, d, sh2);
}

static void REGPARM(3) sh2_write8_da(u32 a, u32 d, SH2 *sh2)
{
  u32 a1 = a & 0xfff;
#ifdef DRC_SH2
  int id = sh2->is_slave;
  int t = Pico32xMem->drcblk_da[id][a1 >> SH2_DRCBLK_DA_SHIFT];
  if (t)
    sh2_drc_wcheck_da(a, t, id);
#endif
  sh2->data_array[a1 ^ 1] = d;
}

// write16
static void REGPARM(3) sh2_write16_unmapped(u32 a, u32 d, SH2 *sh2)
{
  printf("unmapped w16 [%08x]     %04x @%06x",
    a, d & 0xffff, sh2_pc(sh2));
}

static void REGPARM(3) sh2_write16_cs0(u32 a, u32 d, SH2 *sh2)
{
    /*
  if (((EL_LOGMASK & EL_PWM) || (a & 0x30) != 0x30)) // hide PWM
    elprintf_sh2(sh2, EL_32X, "w16 [%08x]     %04x @%06x",
      a, d & 0xffff, sh2_pc(sh2));

  if (Pico32x.regs[0] & P32XS_FM) {
    if ((a & 0x3fff0) == 0x4100) {
      sh2->poll_addr = 0;
      p32x_vdp_write16(a, d, sh2);
      return;
    }

    if ((a & 0x3fe00) == 0x4200) {
      Pico32xMem->pal[(a & 0x1ff) / 2] = d;
      Pico32x.dirty_pal = 1;
      return;
    }
  }

  if ((a & 0x3ffc0) == 0x4000) {
    p32x_sh2reg_write16(a, d, sh2);
    return;
  }

  sh2_write16_unmapped(a, d, sh2);
     */
}

static void REGPARM(3) sh2_write16_dram0(u32 a, u32 d, SH2 *sh2)
{
  sh2_write16_dramN(0);
}

static void REGPARM(3) sh2_write16_dram1(u32 a, u32 d, SH2 *sh2)
{
  sh2_write16_dramN(1);
}

static void REGPARM(3) sh2_write16_sdram(u32 a, u32 d, SH2 *sh2)
{
  u32 a1 = a & 0x3ffff;
#ifdef DRC_SH2
  int t = Pico32xMem->drcblk_ram[a1 >> SH2_DRCBLK_RAM_SHIFT];
  if (t)
    sh2_drc_wcheck_ram(a, t, sh2->is_slave);
#endif
  ((u16 *)SDRAM)[a1 / 2] = d;
}

static void REGPARM(3) sh2_write16_da(u32 a, u32 d, SH2 *sh2)
{
  u32 a1 = a & 0xfff;
#ifdef DRC_SH2
  int id = sh2->is_slave;
  int t = Pico32xMem->drcblk_da[id][a1 >> SH2_DRCBLK_DA_SHIFT];
  if (t)
    sh2_drc_wcheck_da(a, t, id);
#endif
  ((u16 *)sh2->data_array)[a1 / 2] = d;
}

// -----------------------------------------------------------------

#define SH2MAP_ADDR2OFFS_R(a) \
((u32)(a) >> SH2_READ_SHIFT)

#define SH2MAP_ADDR2OFFS_W(a) \
((u32)(a) >> SH2_WRITE_SHIFT)

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
    
    if (offs == SH2MAP_ADDR2OFFS_R(0xffffc000))
        return sh2_peripheral_read32(a, sh2);
    
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
    
    if (offs == SH2MAP_ADDR2OFFS_W(0xffffc000)) {
        sh2_peripheral_write32(a, d, sh2);
        return;
    }
    
    wh = sh2_wmap[offs];
    wh(a, d >> 16, sh2);
    wh(a + 2, d, sh2);
}


#define MAP_MEMORY(m) ((uptr)(m) >> 1)
#define MAP_HANDLER(h) ( ((uptr)(h) >> 1) | ((uptr)1 << (sizeof(uptr) * 8 - 1)) )

static sh2_memmap sh2_read8_map[0x80], sh2_read16_map[0x80];
// for writes we are using handlers only
static sh2_write_handler *sh2_write8_map[0x80], *sh2_write16_map[0x80];

void Pico32xSwapDRAM(int b)
{
    /*
  cpu68k_map_set(m68k_read8_map,   0x840000, 0x85ffff, Pico32xMem->dram[b], 0);
  cpu68k_map_set(m68k_read16_map,  0x840000, 0x85ffff, Pico32xMem->dram[b], 0);
  cpu68k_map_set(m68k_read8_map,   0x860000, 0x87ffff, Pico32xMem->dram[b], 0);
  cpu68k_map_set(m68k_read16_map,  0x860000, 0x87ffff, Pico32xMem->dram[b], 0);
  cpu68k_map_set(m68k_write8_map,  0x840000, 0x87ffff,
                 b ? m68k_write8_dram1_ow : m68k_write8_dram0_ow, 1);
  cpu68k_map_set(m68k_write16_map, 0x840000, 0x87ffff,
                 b ? m68k_write16_dram1_ow : m68k_write16_dram0_ow, 1);
*/
    
  // SH2
  sh2_read8_map[0x04/2].addr  = sh2_read8_map[0x24/2].addr  =
  sh2_read16_map[0x04/2].addr = sh2_read16_map[0x24/2].addr = MAP_MEMORY(DRAM[b]);

  sh2_write8_map[0x04/2]  = sh2_write8_map[0x24/2]  = b ? sh2_write8_dram1 : sh2_write8_dram0;
  sh2_write16_map[0x04/2] = sh2_write16_map[0x24/2] = b ? sh2_write16_dram1 : sh2_write16_dram0;
}

void PicoMemSetup32x(void)
{
  unsigned int rs;
  int i;

  // SH2 maps: A31,A30,A29,CS1,CS0
  // all unmapped by default
  for (i = 0; i < ARRAY_SIZE(sh2_read8_map); i++) {
    sh2_read8_map[i].addr   = MAP_HANDLER(sh2_read8_unmapped);
    sh2_read16_map[i].addr  = MAP_HANDLER(sh2_read16_unmapped);
  }

  for (i = 0; i < ARRAY_SIZE(sh2_write8_map); i++) {
    sh2_write8_map[i]       = sh2_write8_unmapped;
    sh2_write16_map[i]      = sh2_write16_unmapped;
  }

  // "purge area"
  for (i = 0x40; i <= 0x5f; i++) {
    sh2_write8_map[i >> 1]  =
    sh2_write16_map[i >> 1] = sh2_write_ignore;
  }

  // CS0
  sh2_read8_map[0x00/2].addr  = sh2_read8_map[0x20/2].addr  = MAP_HANDLER(sh2_read8_cs0);
  sh2_read16_map[0x00/2].addr = sh2_read16_map[0x20/2].addr = MAP_HANDLER(sh2_read16_cs0);
  sh2_write8_map[0x00/2]  = sh2_write8_map[0x20/2]  = sh2_write8_cs0;
  sh2_write16_map[0x00/2] = sh2_write16_map[0x20/2] = sh2_write16_cs0;
    
  // CS1 - ROM
  sh2_read8_map[0x02/2].addr  = sh2_read8_map[0x22/2].addr  =
  sh2_read16_map[0x02/2].addr = sh2_read16_map[0x22/2].addr = MAP_MEMORY(Pico.rom);
  sh2_read8_map[0x02/2].mask  = sh2_read8_map[0x22/2].mask  =
  sh2_read16_map[0x02/2].mask = sh2_read16_map[0x22/2].mask = 0x3fffff; // FIXME
    
  // CS2 - DRAM - done by Pico32xSwapDRAM()
  sh2_read8_map[0x04/2].mask  = sh2_read8_map[0x24/2].mask  =
  sh2_read16_map[0x04/2].mask = sh2_read16_map[0x24/2].mask = 0x01ffff;
  // CS3 - SDRAM
  sh2_read8_map[0x06/2].addr   = sh2_read8_map[0x26/2].addr   =
  sh2_read16_map[0x06/2].addr  = sh2_read16_map[0x26/2].addr  = MAP_MEMORY(SDRAM);
  sh2_write8_map[0x06/2]       = sh2_write8_sdram;
  sh2_write8_map[0x26/2]       = sh2_write8_sdram_wt;
  sh2_write16_map[0x06/2]      = sh2_write16_map[0x26/2]      = sh2_write16_sdram;
  sh2_read8_map[0x06/2].mask   = sh2_read8_map[0x26/2].mask   =
  sh2_read16_map[0x06/2].mask  = sh2_read16_map[0x26/2].mask  = 0x03ffff;
  // SH2 data array
  sh2_read8_map[0xc0/2].addr  = MAP_HANDLER(sh2_read8_da);
  sh2_read16_map[0xc0/2].addr = MAP_HANDLER(sh2_read16_da);
  sh2_write8_map[0xc0/2]      = sh2_write8_da;
  sh2_write16_map[0xc0/2]     = sh2_write16_da;
    
    /*
  // SH2 IO
  sh2_read8_map[0xff/2].addr  = MAP_HANDLER(sh2_peripheral_read8);
  sh2_read16_map[0xff/2].addr = MAP_HANDLER(sh2_peripheral_read16);
  sh2_write8_map[0xff/2]      = sh2_peripheral_write8;
  sh2_write16_map[0xff/2]     = sh2_peripheral_write16;
*/
    
  // map DRAM area, both 68k and SH2
  Pico32xSwapDRAM(1);
}

// vim:shiftwidth=2:ts=2:expandtab
