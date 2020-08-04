#include <stdio.h>
#include <file.h>
#include <music.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	char str[16];

	while(1)
	{
		memset(str, '\0', 16);
		scanf("%s", str);
		str[15] = '\0';
		if (strncmp(str, "add", strlen("add")) == 0)
		{
			AddSongs();
			PrintSongList();
		}
		else if (strncmp(str, "play", strlen("play")) == 0)
		{
			PlaySong("/home/book/music/xiaobaima.mp3");
		}
		else if (strncmp(str, "next", strlen("next")) == 0)
		{
			NextSong();
		}
		else if (strncmp(str, "prev", strlen("prev")) == 0)
		{
			PrevSong();
		}
		else if (strncmp(str, "pause", strlen("pause")) == 0)
		{
			PauseSong();
		}
		else if (strncmp(str, "stop", strlen("stop")) == 0)
		{
			StopSong();
		}

	}

exit:
	FreeSongLink();
	return 0;
}

