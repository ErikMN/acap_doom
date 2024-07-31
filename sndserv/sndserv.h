#pragma once

#include <math.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <syslog.h>
#include <pthread.h>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#define SNDSERV_VERSION "1.0"

#ifdef HOST
#define SAMPLECOUNT 256
#else
#define SAMPLECOUNT 512
#endif
#define MIXBUFFERSIZE (SAMPLECOUNT * 2 * 2)

#ifdef DEBUG
#define DBUG(x) x
#else
#define DBUG(x)
#endif

// clang-format off
/* Print colors */
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"
#define PRINT_COLOR(color, format, ...) DBUG(printf(GRN "%s:%d %s\t\t" RESET color format RESET "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
#define PRINT_RED(format, ...)     PRINT_COLOR(RED, format, ##__VA_ARGS__)
#define PRINT_GREEN(format, ...)   PRINT_COLOR(GRN, format, ##__VA_ARGS__)
#define PRINT_YELLOW(format, ...)  PRINT_COLOR(YEL, format, ##__VA_ARGS__)
#define PRINT_BLUE(format, ...)    PRINT_COLOR(BLU, format, ##__VA_ARGS__)
#define PRINT_MAGENTA(format, ...) PRINT_COLOR(MAG, format, ##__VA_ARGS__)
#define PRINT_CYAN(format, ...)    PRINT_COLOR(CYN, format, ##__VA_ARGS__)
#define PRINT_WHITE(format, ...)   PRINT_COLOR(WHT, format, ##__VA_ARGS__)
// clang-format on

#define print_debug(...) DBUG(printf(__VA_ARGS__))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(0 [x]))
#endif

/* GCC extension for marking stuff as unused */
#ifdef __GNUC__
#define NOT_USED __attribute__((unused))
#else
#define NOT_USED
#endif

/* Function prototypes */
void I_InitMusic(void);
void I_InitSound(int samplerate, int samplesound);
void I_SubmitOutputBuffer(void *samples, int samplecount);
void I_ShutdownSound(void);
void I_ShutdownMusic(void);

/* SoundFX struct. */
typedef struct sfxinfo_struct sfxinfo_t;

struct sfxinfo_struct {
  /* up to 6-character name */
  char *name;
  /* Sfx singularity (only one at a time) */
  int singularity;
  /* Sfx priority */
  int priority;
  /* referenced sound if a link */
  sfxinfo_t *link;
  /* pitch if a link */
  int pitch;
  /* volume if a link */
  int volume;
  /* sound data */
  void *data;
  /* this is checked every second to see if sound can be thrown out (if 0, then
   * decrement, if -1, then throw out, if > 0, then it's in use) */
  int usefulness;
  /* lump number of sfx */
  int lumpnum;
};

typedef struct {
  /* sound information (if null, channel avail.) */
  sfxinfo_t *sfxinfo;
  /* origin of sound */
  void *origin;
  /* handle of the sound being played */
  int handle;
} channel_t;

/* Identifiers for all sfx in game. */
typedef enum {
  sfx_None,
  sfx_pistol,
  sfx_shotgn,
  sfx_sgcock,
  sfx_dshtgn,
  sfx_dbopn,
  sfx_dbcls,
  sfx_dbload,
  sfx_plasma,
  sfx_bfg,
  sfx_sawup,
  sfx_sawidl,
  sfx_sawful,
  sfx_sawhit,
  sfx_rlaunc,
  sfx_rxplod,
  sfx_firsht,
  sfx_firxpl,
  sfx_pstart,
  sfx_pstop,
  sfx_doropn,
  sfx_dorcls,
  sfx_stnmov,
  sfx_swtchn,
  sfx_swtchx,
  sfx_plpain,
  sfx_dmpain,
  sfx_popain,
  sfx_vipain,
  sfx_mnpain,
  sfx_pepain,
  sfx_slop,
  sfx_itemup,
  sfx_wpnup,
  sfx_oof,
  sfx_telept,
  sfx_posit1,
  sfx_posit2,
  sfx_posit3,
  sfx_bgsit1,
  sfx_bgsit2,
  sfx_sgtsit,
  sfx_cacsit,
  sfx_brssit,
  sfx_cybsit,
  sfx_spisit,
  sfx_bspsit,
  sfx_kntsit,
  sfx_vilsit,
  sfx_mansit,
  sfx_pesit,
  sfx_sklatk,
  sfx_sgtatk,
  sfx_skepch,
  sfx_vilatk,
  sfx_claw,
  sfx_skeswg,
  sfx_pldeth,
  sfx_pdiehi,
  sfx_podth1,
  sfx_podth2,
  sfx_podth3,
  sfx_bgdth1,
  sfx_bgdth2,
  sfx_sgtdth,
  sfx_cacdth,
  sfx_skldth,
  sfx_brsdth,
  sfx_cybdth,
  sfx_spidth,
  sfx_bspdth,
  sfx_vildth,
  sfx_kntdth,
  sfx_pedth,
  sfx_skedth,
  sfx_posact,
  sfx_bgact,
  sfx_dmact,
  sfx_bspact,
  sfx_bspwlk,
  sfx_vilact,
  sfx_noway,
  sfx_barexp,
  sfx_punch,
  sfx_hoof,
  sfx_metal,
  sfx_chgun,
  sfx_tink,
  sfx_bdopn,
  sfx_bdcls,
  sfx_itmbk,
  sfx_flame,
  sfx_flamst,
  sfx_getpow,
  sfx_bospit,
  sfx_boscub,
  sfx_bossit,
  sfx_bospn,
  sfx_bosdth,
  sfx_manatk,
  sfx_mandth,
  sfx_sssit,
  sfx_ssdth,
  sfx_keenpn,
  sfx_keendt,
  sfx_skeact,
  sfx_skesit,
  sfx_skeatk,
  sfx_radio,
  NUMSFX
} sfxenum_t;

extern sfxinfo_t S_sfx[];

/**
 * Opens the wadfile specified.
 * Must be called before any calls to loadlump() or getsfx().
 */
void openwad(char *wadname);

/**
 * Gets a sound effect from the wad file. The pointer points to the
 * start of the data. Returns a 0 if the sfx was not found.
 * Sfx names should be no longer than 6 characters.
 * All data is rounded up in size to the nearest MIXBUFFERSIZE and is
 * padded out with 0x80's. Returns the data length in len.
 */
void *getsfx(char *sfxname, int *len);

/* Get all SFX from WAD */
void grabdata(void);

/* Lengths of all sound effects */
extern int lengths[NUMSFX];
