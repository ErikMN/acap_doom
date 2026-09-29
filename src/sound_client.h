#pragma once

int sound_client_init(const char *program, const char *wad);
int sound_client_start(int id, int volume, int separation, int pitch);
void sound_client_stop(int handle);
int sound_client_playing(int handle);
void sound_client_update(int handle, int volume, int separation, int pitch);
void sound_client_shutdown(void);
