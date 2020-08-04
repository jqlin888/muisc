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

static int bStopPlaySignal;
static int g_iSongSum;
static PT_SongOpr g_ptSongOprHead;
//static pthread_mutex_t g_SearchThreadMtx = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t g_SearchThreadCond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t g_PlayThreadMtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_PlayThreadCond = PTHREAD_COND_INITIALIZER;

static E_PlayCmd g_ePlayCmd = PLAY_CMD_NONE;		/* 默认没有播放命令 */
static E_PlayMode g_ePlayMode = PLAY_MODE_ORDER;	/* 默认顺序播放 */

static int isExistProcess(const char *strName)
{
	FILE *ptFd;
	int iPsNum;
	char acCmd[32];
	char acTmp[8];
	snprintf(acCmd, 32, "ps -ef |grep %s |grep -v grep | wc -l", strName);
	acCmd[31] = '\0';
	system(acCmd);

	if((ptFd = popen(acCmd, "r")) != NULL)
	{
		if((fgets(acTmp, sizeof(acTmp), ptFd)) !=  NULL )
		{ 
			iPsNum = atoi(acTmp);
			if (0 != iPsNum)
			{
				pclose(ptFd);
				return 1;
			}
		}
		else
			return 0;
	}
	else
		return 0;
}

static void SetPlayCmd(E_PlayCmd ePlayCmd)
{
//	pthread_mutex_lock(&g_SearchThreadMtx);
	g_ePlayCmd = ePlayCmd;
//	pthread_mutex_unlock(&g_SearchThreadMtx);
}

static E_PlayCmd GetPlayCmd(void)
{
//	pthread_mutex_lock(&g_SearchThreadMtx);
	return g_ePlayCmd;
//	pthread_mutex_unlock(&g_SearchThreadMtx);
}

static void SetPlayMode(E_PlayMode ePlayMode)
{
	g_ePlayMode = ePlayMode;
}

static E_PlayMode GetPlayMode(void)
{
	return g_ePlayMode;
}

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
	g_iSongSum++;	/* 歌曲总数，每次加1 */

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

PT_SongOpr GetSongFrmName(const char *strName)
{
	PT_SongOpr ptTmp;
	if (NULL == g_ptSongOprHead)
		return NULL;
	else
	{
		ptTmp = g_ptSongOprHead;
		do
		{
			if (0 == strncmp(ptTmp->strName, strName, strlen(strName)))
				return ptTmp;
			ptTmp = ptTmp->ptNext;
		}while(ptTmp != g_ptSongOprHead);
	}

	return NULL;
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
			snprintf(ptSongOpr->strName, PATH_NAME_LEN, "%s/%s", strCurPath, aptDirContent[i]->strName);
			ptSongOpr->strName[PATH_NAME_LEN - 1] = '\0';
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
//	pthread_mutex_lock(&g_SearchThreadMtx);
	ptTmp = g_ptSongOprHead;
//	pthread_mutex_unlock(&g_SearchThreadMtx);
	
	return ptTmp;
}

static PT_SongOpr GetSongFrmPlayMode(PT_SongOpr ptCurSong, int iSongSum)
{
	int iRandomVal;
	E_PlayMode ePlayMode;
	ePlayMode = GetPlayMode();
	switch(ePlayMode)
	{
		case PLAY_MODE_ORDER:
		{
			ptCurSong = ptCurSong->ptNext;
			return ptCurSong;
		}
		case PLAY_MODE_REVERSE:
		{
			ptCurSong = ptCurSong->ptPrev;
			return ptCurSong;
		}
		case PLAY_MODE_SINGLE:
		{
			return ptCurSong;
		}
		case PLAY_MODE_RANDOM:
		{
			srand((int)time(0));
			iRandomVal = rand() % iSongSum;
			if (0 == iRandomVal)
				iRandomVal++;
			while(iRandomVal > 0)
			{
				ptCurSong = ptCurSong->ptNext;
				iRandomVal--;
			}
			return ptCurSong;
		}
		default:
			return ptCurSong->ptNext;
	}
}


static void *Play(void *ptArg)
{
	E_PlayCmd ePlayCmd;
	char strCmd[256];
	PT_SongOpr ptCurSong = (PT_SongOpr)ptArg;

	while(ptCurSong)
	{
		if (bStopPlaySignal)
		{
			bStopPlaySignal = 0;
			printf("debug: %s:%d\r\n", __FILE__, __LINE__);
			pthread_mutex_lock(&g_PlayThreadMtx);
			pthread_cond_wait(&g_PlayThreadCond, &g_PlayThreadMtx);
			pthread_mutex_unlock(&g_PlayThreadMtx);
		}
		
		snprintf(strCmd, 256, "play %s", ptCurSong->strName);
		strCmd[255] = '\0';
		system(strCmd);

		ePlayCmd = GetPlayCmd();
		switch (ePlayCmd)
		{
			case PLAY_CMD_NEXT:
			{
				ptCurSong = ptCurSong->ptNext;
				SetPlayCmd(PLAY_CMD_NONE);
				break;
			}
			case PLAY_CMD_PREV:
			{
				ptCurSong = ptCurSong->ptPrev;
				SetPlayCmd(PLAY_CMD_NONE);
				break;
			}
			case PLAY_CMD_NONE:
			{
				ptCurSong = GetSongFrmPlayMode(ptCurSong, g_iSongSum);
			}
			default:
			{
				break;
			}
		}
	}

	return NULL;
}

int PlaySong(const char *strName)
{
	int iRet;
	static pthread_t tPlayThreadID;
	PT_SongOpr ptCurSong;

	ptCurSong = GetSongFrmName(strName);
	if (NULL == ptCurSong)
	{
		printf("Song list is empty!\r\n");
		return -1;
	}

	/* 如果线程存在 */
	if (isExistProcess("play"))
	{
		system("killall -CONT play");
	}
	else
	{
		if (0 == tPlayThreadID)
		{
			iRet = pthread_create(&tPlayThreadID, NULL, &Play, ptCurSong);
			if (iRet < 0)
			{
				printf("create play thread failed!\r\n");
				return -1;
			}
		}
		else
		{
			pthread_mutex_lock(&g_PlayThreadMtx);
			pthread_cond_signal(&g_PlayThreadCond);
			pthread_mutex_unlock(&g_PlayThreadMtx);
		}
	}
	
	return 0;
}

int NextSong(void)
{
	SetPlayCmd(PLAY_CMD_NEXT);
	system("killall -9 play");

	return 0;
}

int PrevSong(void)
{
	SetPlayCmd(PLAY_CMD_PREV);
	system("killall -9 play");

	return 0;
}

int PauseSong(void)
{
	system("killall -STOP play");
	return 0;
}

int StopSong(void)
{
	bStopPlaySignal = 1;
	system("killall -9 play");
	return 0;
}
