#ifndef _FILE_H
#define _FILE_H

#define PATH_NAME_LEN 256

enum{
    DIRTYPE_UNKNOWN = 0,
    DIRTYPE_FIFO = 1,
    DIRTYPE_CHR = 2,
    DIRTYPE_DIR = 4,
    DIRTYPE_BLK = 6,
    DIRTYPE_REG = 8,
    DIRTYPE_LNK = 10,
    DIRTYPE_SOCK = 12,
    DIRTYPE_WHT = 14
};

typedef enum {
	FILETYPE_DIR,	/* 目录 */
	FILETYPE_FILE,	/* 文件 */
	FILETYPE_MP3,	/* MP3文件 */
	FILETYPE_WAV,	/* WAV文件 */
}E_FileType;

typedef struct DirContent {
	char strName[PATH_NAME_LEN];
	E_FileType eFileType;
}T_DirContent, *PT_DirContent;

int GetDirContent(const char *strDirPath, PT_DirContent **apptDirContent);
int FreeDirContent(PT_DirContent *aptDirContent, int iSum);
int GetFileFrmDir(const char *strDirName, E_FileType eFileType, int *piHadGetFileSum, char apFileName[][PATH_NAME_LEN]);

#endif /* _FILE_H */
