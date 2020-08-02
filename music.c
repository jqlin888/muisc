#include <file.h>
#include <music.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/wait.h>
#include <string.h>

static PT_SongOpr g_ptSongOprHead;
static pthread_mutex_t g_SearchThreadMtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_SearchThreadCond = PTHREAD_COND_INITIALIZER;

static int AddSong2Link(PT_SongOpr ptSongOpr)
{
	PT_SongOpr ptTmp;
	
	if (!g_ptSongOprHead)
	{
		g_ptSongOprHead = ptSongOpr;
		ptSongOpr->ptPrev = g_ptSongOprHead;
		ptSongOpr->ptNext = g_ptSongOprHead;
	}
	else
	{
		ptTmp = g_ptSongOprHead;
		while(ptTmp->ptNext != g_ptSongOprHead)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptSongOpr;
		ptSongOpr->ptPrev = ptTmp;
		ptSongOpr->ptNext = g_ptSongOprHead;
		g_ptSongOprHead->ptPrev = ptSongOpr;
	}

	return 0;
}

int PrintSongList(void)
{
	int i = 1;
	PT_SongOpr ptTmp;

	ptTmp = g_ptSongOprHead;
	if (NULL == ptTmp)
	{
		printf("Song list is empty!\r\n");
		return -1;
	}

	printf("%02d: %s\r\n",i++, ptTmp->strName);
	while(ptTmp->ptNext != g_ptSongOprHead)
	{
		printf("%02d: %s\r\n",i++, ptTmp->ptNext->strName);
		ptTmp = ptTmp->ptNext;
	}
	
	return 0;
}

int FreeSongLink(void)
{
	int i;
	PT_SongOpr ptTmp;
	PT_SongOpr ptSongOpr;
	ptSongOpr = g_ptSongOprHead;
	if (NULL != ptSongOpr)
	{
		while(ptSongOpr->ptNext != g_ptSongOprHead)
		{
			i++;
			ptTmp = ptSongOpr->ptNext;
			ptSongOpr->ptNext = ptTmp->ptNext;
			ptTmp->ptNext->ptPrev = ptSongOpr;
			free(ptTmp);
		}
		if (ptSongOpr)
		{
			free(ptSongOpr);
			ptSongOpr->ptPrev = NULL;
			ptSongOpr->ptNext = NULL;
			i++;
		}
		printf("i = %d\r\n", i);
	}
	
	return 0;
}

int AddSongs(void)
{
	int i;
	int iFileSum;
	char strCurPath[PATH_NAME_LEN];
	PT_DirContent *aptDirContent;
	PT_SongOpr ptSongOpr;
	
	getcwd(strCurPath, 256);
	strCurPath[255] = '\0';
	
	iFileSum = GetDirContent(strCurPath, &aptDirContent);
	for (i = 0; i < iFileSum; i++)
	{
		if (aptDirContent[i]->eFileType == FILETYPE_MP3 || aptDirContent[i]->eFileType == FILETYPE_WAV)
		{
			ptSongOpr = malloc(sizeof(T_SongOpr));
			if (!ptSongOpr)
			{
				printf("alloc mem for ptSongOpr\r\n");
				continue;
			}
			bzero(ptSongOpr, sizeof(T_SongOpr));
			strncpy(ptSongOpr->strName, aptDirContent[i]->strName, PATH_NAME_LEN);
			ptSongOpr->eFileType = aptDirContent[i]->eFileType;
			AddSong2Link(ptSongOpr);
		}
	}
	
	FreeDirContent(aptDirContent, iFileSum);
	
	return 0;
}

PT_SongOpr GetSongsList(void)
{
	PT_SongOpr ptTmp;
	pthread_mutex_lock(&g_SearchThreadMtx);
	ptTmp = g_ptSongOprHead;
	pthread_mutex_unlock(&g_SearchThreadMtx);
	
	return ptTmp;
}

int Play(PT_SongOpr ptCurSong)
{
	char strCmd[256];
	pid_t iPlayPid;

	while(ptCurSong)
	{
		iPlayPid = fork();
		if (iPlayPid < 0)
		{
			printf("%s: create process failed!\r\n", __FUNCTION__);
			exit(EXIT_FAILURE);
		}
		else if (iPlayPid == 0)
		{
			snprintf(strCmd, 256, "play %s", ptCurSong->strName);
			system(strCmd);
		}
		else
		{
			if (wait(NULL) == iPlayPid)
			{
				ptCurSong = ptCurSong->ptNext;	
				printf("Next song name : %s\r\n", ptCurSong->strName);
			}
		}
	}

	return 0;
}

int PlaySong(PT_SongOpr ptCurSong)
{
	if (NULL == ptCurSong)
	{
		printf("Song list is empty!\r\n");
		return -1;
	}
	else
	{		
		Play(ptCurSong);
	}
	return 0;
}

int NextSong(PT_SongOpr ptCurSong)
{
	if (NULL == ptCurSong)
	{
		printf("Song list is empty!\r\n");
		return -1;
	}
	else
	{
		ptCurSong = ptCurSong->ptNext;
	}
	Play(ptCurSong);

	return 0;
}

int PrevSong(PT_SongOpr ptCurSong)
{
	if (NULL == ptCurSong)
	{
		printf("Song list is empty!\r\n");
		return -1;
	}
	else
	{
		ptCurSong = ptCurSong->ptPrev;
	}
	Play(ptCurSong);

	return 0;
}

int PauseSong(PT_SongOpr ptCurSong)
{
	return 0;
}

int StopSong(PT_SongOpr ptCurSong)
{
	system("killall play");
	return 0;
}
