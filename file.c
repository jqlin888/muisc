#include <file.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

static int isRegDir(const char *strDirPath, const char *strName)
{
	int i;
    static const char *astrSpecailDirs[] = {"sbin", "bin", "usr", "lib", "proc", "tmp", "dev", "sys", NULL};

	if (0 == strcmp(strDirPath, "/"))
	{
		while (astrSpecailDirs[i])
		{
			if (0 == strcmp(strName, astrSpecailDirs[i]))
				return 0;
			else
				i++;
		}
	}

	return 1;
}

static int isDir(const char *strDirPath, const char *strName)
{
	char strPathName[PATH_NAME_LEN];
	struct stat tStat;
	
	snprintf(strPathName, PATH_NAME_LEN, "%s/%s", strDirPath, strName);
	strPathName[PATH_NAME_LEN - 1] = '\0';
		
	if ((0 == stat(strPathName, &tStat)) && S_ISDIR(tStat.st_mode))
		return 1;
	else
		return 0;
}

static int isMp3File(const char *strName)
{
	int i;
	size_t iStrLen;
	
	iStrLen = strlen(strName);
	for (i = (iStrLen - 1); i > 0; i--)
	{
		if (strName[i] == '.')
		{
			if (0 == strncmp(&strName[i+1], "mp3", 3))
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

static int isWavFile(const char *strName)
{
	int i;
	size_t iStrLen;
	
	iStrLen = strlen(strName);
	for (i = (iStrLen - 1); i > 0; i--)
	{
		if (strName[i] == '.')
		{
			if (0 == strncmp(&strName[i+1], "wav", 3))
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

static int isRegFile(const char *strDirPath, const char *strName, E_FileType *eFileType)
{
	char strPathName[PATH_NAME_LEN];
	struct stat tStat;
	
	snprintf(strPathName, PATH_NAME_LEN, "%s/%s", strDirPath, strName);
	strPathName[PATH_NAME_LEN - 1] = '\0';
		
	if ((0 == stat(strPathName, &tStat)) && S_ISREG(tStat.st_mode))
	{
		if (isMp3File(strPathName))
			*eFileType = FILETYPE_MP3;
		else if (isWavFile(strPathName))
			*eFileType = FILETYPE_WAV;
		else
			*eFileType = FILETYPE_FILE;
		
		return 1;
	}
	else
		return 0;
}

int GetDirContent(const char *strDirPath, PT_DirContent **apptDirContent)
{
	int i, j;
	int iFileSum;
	E_FileType eFileType;
	PT_DirContent *aptDirContent;
	struct dirent **aptNameList;

	/* 扫描目录，按名字排序存放在aptNameList中 */
 	iFileSum = scandir(strDirPath, &aptNameList, 0, alphasort);
	if (iFileSum < 0)
	{
		printf("Scan diretory failed!\r\n");
		goto err_exit;
	}

	/* 忽略 .和..这两个目录 */
	aptDirContent = malloc(sizeof(PT_DirContent) * (iFileSum * 2));
	if (NULL == aptDirContent)
	{
		printf("alloc mem for aptDirContent failed!\r\n");
		goto err_exit;
	}
	*apptDirContent = aptDirContent;

	/* 忽略 .和..这两个目录 */
	for (i = 0; i < iFileSum; i++)
	{
		aptDirContent[i] = malloc(sizeof(T_DirContent) * (iFileSum * 2));
		if (NULL == aptDirContent[i])
		{
			printf("alloc mem for aptDirContent[%d] failed", i);
			goto err_exit;
		}
		bzero(aptDirContent[i], sizeof(T_DirContent));
	}

	/* 先挑选出目录 */
	for (i = 0, j = 0; i < iFileSum; i++)
	{
		/* 忽略 .和..两个目录 */
		if ((0 == strncmp(aptNameList[i]->d_name, ".", aptNameList[i]->d_reclen)) || \
			(0 == strncmp(aptNameList[i]->d_name, "..", aptNameList[i]->d_reclen)))
			continue;

		if (isDir(strDirPath, aptNameList[i]->d_name))
		{
			strncpy(aptDirContent[j]->strName, aptNameList[i]->d_name, PATH_NAME_LEN);
			aptDirContent[j]->strName[PATH_NAME_LEN - 1] = '\0';
			aptDirContent[j]->eFileType = FILETYPE_DIR;
			free(aptNameList[i]);
			aptNameList[i] = NULL;
			j++;
		}
	}

	/* 挑选出文件 */
	for (i = 0; i < iFileSum; i++)
	{
		/* 判断指针是否已被释放 */
		if (NULL == aptNameList[i])
			continue;
		
		/* 忽略 .和..两个目录 */
		if ((0 == strncmp(aptNameList[i]->d_name, ".", aptNameList[i]->d_reclen)) || \
			(0 == strncmp(aptNameList[i]->d_name, "..", aptNameList[i]->d_reclen)))
			continue;

		if (isRegFile(strDirPath, aptNameList[i]->d_name, &eFileType))
		{
			strncpy(aptDirContent[j]->strName, aptNameList[i]->d_name, PATH_NAME_LEN);
			aptDirContent[j]->strName[PATH_NAME_LEN - 1] = '\0';
			aptDirContent[j]->eFileType = eFileType;
			free(aptNameList[i]);
			aptNameList[i] = NULL;
			j++;
		}
	}

	/* 释放aptDirContent未使用的项 */
	for (i = j; i < iFileSum; i++)
	{
		if (aptDirContent[i])
		{
			free(aptDirContent[i]);
			aptDirContent[i] = NULL;
		}
	}
	
	for (i = 0; i < iFileSum; i++)
	{
		if (aptNameList[i])
		{
			free(aptNameList[i]);
			aptNameList[i] = NULL;
		}
	}
	free(aptNameList);
	aptNameList = NULL;

	return j;
	
err_exit:
	return -1;
}


int FreeDirContent(PT_DirContent *aptDirContent, int iSum)
{
	int i;
	for (i = 0; i < iSum; i++)
	{
		if (aptDirContent[i])
		{
			free(aptDirContent[i]);
			aptDirContent[i] = NULL;
		}
	}

	free(aptDirContent);
	aptDirContent = NULL;

	return i;
}

int GetFileFrmDir(const char *strDirName, E_FileType eFileType, int *piHadGetFileSum, char apFileName[][PATH_NAME_LEN])
{
	int i;
	int iDirContentSum;
	PT_DirContent *aptDirContent;
	char strSubDirName[PATH_NAME_LEN];
	
	iDirContentSum = GetDirContent(strDirName, &aptDirContent);
	if (-1 == iDirContentSum)
		return -1;
	
	for (i = 0; i < iDirContentSum; i++)
	{
		if (aptDirContent[i]->eFileType == eFileType)
		{
			snprintf(apFileName[i], PATH_NAME_LEN, "%s/%s", strDirName, aptDirContent[i]->strName);
			apFileName[i][PATH_NAME_LEN - 1] = '\0';
			free(aptDirContent[i]);
			aptDirContent[i] = NULL;
		}
	}

	for (i = 0; i < iDirContentSum; i++)
	{
		if (NULL == aptDirContent[i])
			continue;
		
		if (aptDirContent[i]->eFileType == FILETYPE_DIR && isRegDir(strDirName, aptDirContent[i]->strName))
		{
			snprintf(strSubDirName, PATH_NAME_LEN, "%s/%s", strDirName, aptDirContent[i]->strName);
			strSubDirName[PATH_NAME_LEN - 1] = '\0';
			GetFileFrmDir(strSubDirName, eFileType, piHadGetFileSum, apFileName);
		}
	}

	FreeDirContent(aptDirContent, iDirContentSum);
	
	return 0;
}

