#include "sndserv.h"

/******************************************************************************/
/* GLOBAL VARIABLES */

/* Lengths of all sound effects */
int lengths[NUMSFX];
unsigned int sample_rates[NUMSFX];

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

static bool
read_data(int fd, void *buffer, size_t size)
{
  size_t offset = 0;
  while (offset < size) {
    ssize_t result = read(fd, (uint8_t *)buffer + offset, size - offset);
    if (result < 0 && errno == EINTR) {
      continue;
    }
    if (result <= 0) {
      return false;
    }
    offset += (size_t)result;
  }
  return true;
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

  if (!read_data(wadfile, &header, sizeof(header))) {
    derror("Failed to read wadfile header");
  }

  if (strncmp(header.identification, "IWAD", 4) != 0) {
    derror("wadfile has weirdo header");
  }

  numlumps = LONG(header.numlumps);
  tableoffset = LONG(header.infotableofs);
  struct stat fileinfo;
  if (fstat(wadfile, &fileinfo) < 0 || numlumps < 0 || tableoffset < 0 ||
      numlumps > INT_MAX / (int)sizeof(lumpinfo_t) ||
      (uint64_t)tableoffset + (uint64_t)numlumps * sizeof(filelump_t) > (uint64_t)fileinfo.st_size) {
    derror("Invalid WAD directory");
  }
  tablelength = numlumps * sizeof(lumpinfo_t);
  tablefilelength = numlumps * sizeof(filelump_t);
  lumpinfo = (lumpinfo_t *)malloc(tablelength);
  if (!lumpinfo) {
    derror("Memory allocation failed for lumpinfo");
  }

  filetable = (filelump_t *)((char *)lumpinfo + tablelength - tablefilelength);

  /* Get the lumpinfo table */
  if (lseek(wadfile, tableoffset, SEEK_SET) < 0 || !read_data(wadfile, filetable, tablefilelength)) {
    derror("Failed to read lumpinfo table");
  }

  /* Process the table to make the endianness right and shift it down */
  for (int i = 0; i < numlumps; i++) {
    if (LONG(filetable[i].filepos) < 0 || LONG(filetable[i].size) < 0 ||
        (uint64_t)LONG(filetable[i].filepos) + (uint64_t)LONG(filetable[i].size) > (uint64_t)fileinfo.st_size) {
      derror("Invalid WAD lump");
    }
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
    lump = malloc(lumpinfo[i].size);
    if (!lump || lseek(lumpinfo[i].handle, lumpinfo[i].filepos, SEEK_SET) < 0 ||
        !read_data(lumpinfo[i].handle, lump, lumpinfo[i].size)) {
      free(lump);
      *size = 0;
      return NULL;
    }
    *size = lumpinfo[i].size;
  }
  // PRINT_MAGENTA("lumpname: %s size: %d", lumpname, *size);

  return lump;
}

/* Get SFX from WAD */
void *
getsfx(char *sfxname, int *len, unsigned int *rate)
{
  int size = 0;
  char name[20];
  snprintf(name, sizeof(name), "ds%s", sfxname);
  uint8_t *sfx = loadlump(name, &size);
  *len = 0;
  *rate = 0;

  if (!sfx || size < 8 || sfx[0] != 3 || sfx[1] != 0) {
    free(sfx);
    return NULL;
  }
  unsigned int sample_rate = sfx[2] | ((unsigned int)sfx[3] << 8);
  uint32_t count = sfx[4] | ((uint32_t)sfx[5] << 8) | ((uint32_t)sfx[6] << 16) | ((uint32_t)sfx[7] << 24);
  if (sample_rate == 0 || count == 0 || count > (uint32_t)(size - 8)) {
    free(sfx);
    return NULL;
  }

  /* Keep the declared sample length independent of output buffer sizes */
  memmove(sfx, sfx + 8, count);
  *len = (int)count;
  *rate = sample_rate;
  return sfx;
}

/* Open WAD and populate SoundFX struct */
void
grabdata(char *wadname)
{
  openwad(wadname);

  /* Get all SFX from WAD */
  for (int i = 1; i < NUMSFX; i++) {
    if (!S_sfx[i].link) {
      S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i], &sample_rates[i]);
    } else {
      size_t linked = S_sfx[i].link - S_sfx;
      S_sfx[i].data = S_sfx[i].link->data;
      lengths[i] = lengths[linked];
      sample_rates[i] = sample_rates[linked];
    }
  }
}

void
freedata(void)
{
  for (int i = 1; i < NUMSFX; i++) {
    if (!S_sfx[i].link) {
      free(S_sfx[i].data);
    }
    S_sfx[i].data = NULL;
    lengths[i] = 0;
    sample_rates[i] = 0;
  }
  if (numlumps > 0 && lumpinfo) {
    close(lumpinfo[0].handle);
  }
  free(lumpinfo);
  lumpinfo = NULL;
  numlumps = 0;
}
