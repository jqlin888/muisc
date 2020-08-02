#ifndef _MUSIC_H
#define _MUSIC_H
#include <file.h>

typedef struct SongOpr {
	char strName[256];
	E_FileType eFileType;
	struct SongOpr *ptPrev;
	struct SongOpr *ptNext;
}T_SongOpr, *PT_SongOpr;
int PrintSongList(void);
int FreeSongLink(void);
int AddSongs(void);
PT_SongOpr GetSongsList(void);
int Play(PT_SongOpr ptCurSong);
int PlaySong(PT_SongOpr ptCurSong);
int NextSong(PT_SongOpr ptCurSong);
int PrevSong(PT_SongOpr ptCurSong);
int PauseSong(PT_SongOpr ptCurSong);
int StopSong(PT_SongOpr ptCurSong);

#endif /* _MUSIC_H */

