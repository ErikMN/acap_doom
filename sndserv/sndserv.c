#include "sndserv.h"

/******************************************************************************/
/* GLOBAL VARIABLES */

/* Lengths of all sound effects */
int lengths[NUMSFX];

/* Information about all the sfx */
/* {name, singularity, prio, *link, pitch, vol, *data, usefulness, lumpnum} */
sfxinfo_t S_sfx[] = {
  { "none", false, 0, 0, -1, -1, 0, 0, 0 },
  { "pistol", false, 64, 0, -1, -1, 0, 0, 0 },
  { "shotgn", false, 64, 0, -1, -1, 0, 0, 0 },
  { "sgcock", false, 64, 0, -1, -1, 0, 0, 0 },
  { "dshtgn", false, 64, 0, -1, -1, 0, 0, 0 },
  { "dbopn", false, 64, 0, -1, -1, 0, 0, 0 },
  { "dbcls", false, 64, 0, -1, -1, 0, 0, 0 },
  { "dbload", false, 64, 0, -1, -1, 0, 0, 0 },
  { "plasma", false, 64, 0, -1, -1, 0, 0, 0 },
  { "bfg", false, 64, 0, -1, -1, 0, 0, 0 },
  { "sawup", false, 64, 0, -1, -1, 0, 0, 0 },
  { "sawidl", false, 118, 0, -1, -1, 0, 0, 0 },
  { "sawful", false, 64, 0, -1, -1, 0, 0, 0 },
  { "sawhit", false, 64, 0, -1, -1, 0, 0, 0 },
  { "rlaunc", false, 64, 0, -1, -1, 0, 0, 0 },
  { "rxplod", false, 70, 0, -1, -1, 0, 0, 0 },
  { "firsht", false, 70, 0, -1, -1, 0, 0, 0 },
  { "firxpl", false, 70, 0, -1, -1, 0, 0, 0 },
  { "pstart", false, 100, 0, -1, -1, 0, 0, 0 },
  { "pstop", false, 100, 0, -1, -1, 0, 0, 0 },
  { "doropn", false, 100, 0, -1, -1, 0, 0, 0 },
  { "dorcls", false, 100, 0, -1, -1, 0, 0, 0 },
  { "stnmov", false, 119, 0, -1, -1, 0, 0, 0 },
  { "swtchn", false, 78, 0, -1, -1, 0, 0, 0 },
  { "swtchx", false, 78, 0, -1, -1, 0, 0, 0 },
  { "plpain", false, 96, 0, -1, -1, 0, 0, 0 },
  { "dmpain", false, 96, 0, -1, -1, 0, 0, 0 },
  { "popain", false, 96, 0, -1, -1, 0, 0, 0 },
  { "vipain", false, 96, 0, -1, -1, 0, 0, 0 },
  { "mnpain", false, 96, 0, -1, -1, 0, 0, 0 },
  { "pepain", false, 96, 0, -1, -1, 0, 0, 0 },
  { "slop", false, 78, 0, -1, -1, 0, 0, 0 },
  { "itemup", true, 78, 0, -1, -1, 0, 0, 0 },
  { "wpnup", true, 78, 0, -1, -1, 0, 0, 0 },
  { "oof", false, 96, 0, -1, -1, 0, 0, 0 },
  { "telept", false, 32, 0, -1, -1, 0, 0, 0 },
  { "posit1", true, 98, 0, -1, -1, 0, 0, 0 },
  { "posit2", true, 98, 0, -1, -1, 0, 0, 0 },
  { "posit3", true, 98, 0, -1, -1, 0, 0, 0 },
  { "bgsit1", true, 98, 0, -1, -1, 0, 0, 0 },
  { "bgsit2", true, 98, 0, -1, -1, 0, 0, 0 },
  { "sgtsit", true, 98, 0, -1, -1, 0, 0, 0 },
  { "cacsit", true, 98, 0, -1, -1, 0, 0, 0 },
  { "brssit", true, 94, 0, -1, -1, 0, 0, 0 },
  { "cybsit", true, 92, 0, -1, -1, 0, 0, 0 },
  { "spisit", true, 90, 0, -1, -1, 0, 0, 0 },
  { "bspsit", true, 90, 0, -1, -1, 0, 0, 0 },
  { "kntsit", true, 90, 0, -1, -1, 0, 0, 0 },
  { "vilsit", true, 90, 0, -1, -1, 0, 0, 0 },
  { "mansit", true, 90, 0, -1, -1, 0, 0, 0 },
  { "pesit", true, 90, 0, -1, -1, 0, 0, 0 },
  { "sklatk", false, 70, 0, -1, -1, 0, 0, 0 },
  { "sgtatk", false, 70, 0, -1, -1, 0, 0, 0 },
  { "skepch", false, 70, 0, -1, -1, 0, 0, 0 },
  { "vilatk", false, 70, 0, -1, -1, 0, 0, 0 },
  { "claw", false, 70, 0, -1, -1, 0, 0, 0 },
  { "skeswg", false, 70, 0, -1, -1, 0, 0, 0 },
  { "pldeth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "pdiehi", false, 32, 0, -1, -1, 0, 0, 0 },
  { "podth1", false, 70, 0, -1, -1, 0, 0, 0 },
  { "podth2", false, 70, 0, -1, -1, 0, 0, 0 },
  { "podth3", false, 70, 0, -1, -1, 0, 0, 0 },
  { "bgdth1", false, 70, 0, -1, -1, 0, 0, 0 },
  { "bgdth2", false, 70, 0, -1, -1, 0, 0, 0 },
  { "sgtdth", false, 70, 0, -1, -1, 0, 0, 0 },
  { "cacdth", false, 70, 0, -1, -1, 0, 0, 0 },
  { "skldth", false, 70, 0, -1, -1, 0, 0, 0 },
  { "brsdth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "cybdth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "spidth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "bspdth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "vildth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "kntdth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "pedth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "skedth", false, 32, 0, -1, -1, 0, 0, 0 },
  { "posact", true, 120, 0, -1, -1, 0, 0, 0 },
  { "bgact", true, 120, 0, -1, -1, 0, 0, 0 },
  { "dmact", true, 120, 0, -1, -1, 0, 0, 0 },
  { "bspact", true, 100, 0, -1, -1, 0, 0, 0 },
  { "bspwlk", true, 100, 0, -1, -1, 0, 0, 0 },
  { "vilact", true, 100, 0, -1, -1, 0, 0, 0 },
  { "noway", false, 78, 0, -1, -1, 0, 0, 0 },
  { "barexp", false, 60, 0, -1, -1, 0, 0, 0 },
  { "punch", false, 64, 0, -1, -1, 0, 0, 0 },
  { "hoof", false, 70, 0, -1, -1, 0, 0, 0 },
  { "metal", false, 70, 0, -1, -1, 0, 0, 0 },
  { "chgun", false, 64, &S_sfx[sfx_pistol], 150, 0, 0, 0, 0 },
  { "tink", false, 60, 0, -1, -1, 0, 0, 0 },
  { "bdopn", false, 100, 0, -1, -1, 0, 0, 0 },
  { "bdcls", false, 100, 0, -1, -1, 0, 0, 0 },
  { "itmbk", false, 100, 0, -1, -1, 0, 0, 0 },
  { "flame", false, 32, 0, -1, -1, 0, 0, 0 },
  { "flamst", false, 32, 0, -1, -1, 0, 0, 0 },
  { "getpow", false, 60, 0, -1, -1, 0, 0, 0 },
  { "bospit", false, 70, 0, -1, -1, 0, 0, 0 },
  { "boscub", false, 70, 0, -1, -1, 0, 0, 0 },
  { "bossit", false, 70, 0, -1, -1, 0, 0, 0 },
  { "bospn", false, 70, 0, -1, -1, 0, 0, 0 },
  { "bosdth", false, 70, 0, -1, -1, 0, 0, 0 },
  { "manatk", false, 70, 0, -1, -1, 0, 0, 0 },
  { "mandth", false, 70, 0, -1, -1, 0, 0, 0 },
  { "sssit", false, 70, 0, -1, -1, 0, 0, 0 },
  { "ssdth", false, 70, 0, -1, -1, 0, 0, 0 },
  { "keenpn", false, 70, 0, -1, -1, 0, 0, 0 },
  { "keendt", false, 70, 0, -1, -1, 0, 0, 0 },
  { "skeact", false, 70, 0, -1, -1, 0, 0, 0 },
  { "skesit", false, 70, 0, -1, -1, 0, 0, 0 },
  { "skeatk", false, 70, 0, -1, -1, 0, 0, 0 },
  { "radio", false, 60, 0, -1, -1, 0, 0, 0 },
};

/* This version of w_wad.c does handle endianess. */
#ifndef __BIG_ENDIAN__
#define LONG(x) (x)
#define SHORT(x) (x)
#else

#define LONG(x) ((long)SwapLONG((unsigned long)(x)))
#define SHORT(x) ((short)SwapSHORT((unsigned short)(x)))

unsigned long
SwapLONG(unsigned long x)
{
  return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}

unsigned short
SwapSHORT(unsigned short x)
{
  return (x >> 8) | (x << 8);
}
#endif

typedef struct wadinfo_struct {
  char identification[4];
  int numlumps;
  int infotableofs;
} wadinfo_t;

typedef struct filelump_struct {
  int filepos;
  int size;
  char name[8];
} filelump_t;

typedef struct lumpinfo_struct {
  int handle;
  int filepos;
  int size;
  char name[8];
} lumpinfo_t;

lumpinfo_t *lumpinfo = NULL;
int numlumps;

static void
derror(char *msg)
{
  fprintf(stderr, "\nwadread error: %s\n", msg);
  exit(-1);
}

NOT_USED static void
strupr(char *s)
{
  while (*s) {
    *s = toupper(*s);
    s++;
  }
}

NOT_USED static int
filelength(int handle)
{
  struct stat fileinfo;
  if (fstat(handle, &fileinfo) == -1) {
    fprintf(stderr, "Error fstating\n");
  }

  return fileinfo.st_size;
}

/******************************************************************************/
/* Functions for reading and extracting data from WAD */

void
openwad(char *wadname)
{
  PRINT_YELLOW("Open WAD: %s", wadname);
  int wadfile = 0;
  int tableoffset = 0;
  int tablelength = 0;
  int tablefilelength = 0;
  wadinfo_t header = { 0 };
  filelump_t *filetable = NULL;

  /* Open and read the wadfile header */
  wadfile = open(wadname, O_RDONLY);

  if (wadfile < 0) {
    derror("Could not open wadfile");
  }

  ssize_t r = read(wadfile, &header, sizeof(header));
  if (r < (ssize_t)sizeof(header)) {
    derror("Failed to read wadfile header");
  }

  if (strncmp(header.identification, "IWAD", 4) != 0) {
    derror("wadfile has weirdo header");
  }

  numlumps = LONG(header.numlumps);
  tableoffset = LONG(header.infotableofs);
  tablelength = numlumps * sizeof(lumpinfo_t);
  tablefilelength = numlumps * sizeof(filelump_t);
  lumpinfo = (lumpinfo_t *)malloc(tablelength);
  if (!lumpinfo) {
    derror("Memory allocation failed for lumpinfo");
  }

  filetable = (filelump_t *)((char *)lumpinfo + tablelength - tablefilelength);

  /* Get the lumpinfo table */
  lseek(wadfile, tableoffset, SEEK_SET);
  r = read(wadfile, filetable, tablefilelength);
  if (r < tablefilelength) {
    derror("Failed to read lumpinfo table");
  }

  /* Process the table to make the endianness right and shift it down */
  for (int i = 0; i < numlumps; i++) {
    memcpy(lumpinfo[i].name, filetable[i].name, 8);
    lumpinfo[i].handle = wadfile;
    lumpinfo[i].filepos = LONG(filetable[i].filepos);
    lumpinfo[i].size = LONG(filetable[i].size);
    // fprintf(stderr, "lump [%.8s] exists\n", lumpinfo[i].name);
  }
}

static void *
loadlump(char *lumpname, int *size)
{
  int i;
  void *lump;

  for (i = 0; i < numlumps; i++) {
    if (!strncasecmp(lumpinfo[i].name, lumpname, 8))
      break;
  }

  if (i == numlumps) {
    // fprintf(stderr, "Could not find lumpname [%s]\n", lumpname);
    lump = 0;
    *size = 0;
  } else {
    lump = (void *)malloc(lumpinfo[i].size);
    lseek(lumpinfo[i].handle, lumpinfo[i].filepos, SEEK_SET);
    ssize_t r = read(lumpinfo[i].handle, lump, lumpinfo[i].size);
    *size = lumpinfo[i].size;
    (void)r;
  }
  // PRINT_MAGENTA("lumpname: %s size: %d", lumpname, *size);

  return lump;
}

/* Get SFX from WAD */
void *
getsfx(char *sfxname, int *len)
{
  unsigned char *sfx;
  unsigned char *paddedsfx;
  int i = 0;
  int size = 0;
  int paddedsize = 0;
  char name[20];

  sprintf(name, "ds%s", sfxname);

  sfx = (unsigned char *)loadlump(name, &size);

  /* Pad the sound effect out to the mixing buffer size */
  paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;
  paddedsfx = (unsigned char *)realloc(sfx, paddedsize + 8);
  for (i = size; i < paddedsize + 8; i++)
    paddedsfx[i] = 128;

  *len = paddedsize;

  // PRINT_BLUE("sfx name: %s length: %d", sfxname, *len);

  return (void *)(paddedsfx + 8);
}

/* Open WAD and populate SoundFX struct */
void
grabdata(void)
{
  bool ret = false;
  char *name = NULL;
  char *doom1wad = NULL;
  char *doomwad = NULL;
  char *doomuwad = NULL;
  char *doom2wad = NULL;
  char *doom2fwad = NULL;
  const char *doomwaddir = getenv("DOOMWADDIR");

  /* Specify WAD dir */
  if (!doomwaddir)
#ifdef HOST
    doomwaddir = "..";
#else
    doomwaddir = ".";
#endif

  size_t dir_len = strlen(doomwaddir);
  /* 10 for the longest WAD name "doom2f.wad", 1 for the separator, and 1 for the NUL terminator */
  size_t max_len = dir_len + 1 + 10 + 1;

  doom1wad = malloc(max_len);
  if (!doom1wad) {
    perror("malloc: doom1wad");
    goto exit;
  }
  snprintf(doom1wad, max_len, "%s/doom1.wad", doomwaddir);

  doom2wad = malloc(max_len);
  if (!doom2wad) {
    perror("malloc: doom2wad");
    goto exit;
  }
  snprintf(doom2wad, max_len, "%s/doom2.wad", doomwaddir);

  doom2fwad = malloc(max_len);
  if (!doom2fwad) {
    perror("malloc: doom2fwad");
    goto exit;
  }
  snprintf(doom2fwad, max_len, "%s/doom2f.wad", doomwaddir);

  doomuwad = malloc(max_len);
  if (!doomuwad) {
    perror("malloc: doomuwad");
    goto exit;
  }
  snprintf(doomuwad, max_len, "%s/doomu.wad", doomwaddir);

  doomwad = malloc(max_len);
  if (!doomwad) {
    perror("malloc: doomwad");
    goto exit;
  }
  snprintf(doomwad, max_len, "%s/doom.wad", doomwaddir);

  if (!access(doom2fwad, R_OK))
    name = doom2fwad;
  else if (!access(doom2wad, R_OK))
    name = doom2wad;
  else if (!access(doomuwad, R_OK))
    name = doomuwad;
  else if (!access(doomwad, R_OK))
    name = doomwad;
  else if (!access(doom1wad, R_OK))
    name = doom1wad;
  else {
    fprintf(stderr, "Could not find wadfile anywhere\n");
    goto exit;
  }
  /* Read the WAD file */
  openwad(name);
  PRINT_BLUE("Loading from [%s]", name);

  /* Get all SFX from WAD */
  for (int i = 1; i < NUMSFX; i++) {
    if (!S_sfx[i].link) {
      S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
    } else {
      S_sfx[i].data = S_sfx[i].link->data;
      lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
    }
  }
  ret = true;

exit:
  /* Free allocated memory */
  free(doom1wad);
  free(doom2wad);
  free(doom2fwad);
  free(doomuwad);
  free(doomwad);

  if (!ret) {
    exit(EXIT_FAILURE);
  }
}
