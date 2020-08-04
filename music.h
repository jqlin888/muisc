#ifndef _MUSIC_H
#define _MUSIC_H
#include <file.h>

typedef enum {
	PLAY_MODE_ORDER,	/* 播放模式：顺序播放 */
	PLAY_MODE_REVERSE,	/* 播放模式：逆序播放 */
	PLAY_MODE_SINGLE,	/* 播放模式：单曲循环 */
	PLAY_MODE_RANDOM,	/* 播放模式：随机播放 */
}E_PlayMode;

typedef enum {
	PLAY_CMD_NONE,		/* 播放命令：无 */
	PLAY_CMD_NEXT,		/* 播放命令：下一曲 */
	PLAY_CMD_PREV,		/* 播放命令：上一曲 */
}E_PlayCmd;

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

int PlaySong(const char *strName);
int NextSong(void);
int PrevSong(void);
int PauseSong(void);
int StopSong(void);

#endif /* _MUSIC_H */

