#include <stdio.h>
#include <file.h>
#include <music.h>
#include <unistd.h>

void PrintUsage(void)
{
	printf("Usage:\r\n");
	printf("a : add songs to list\r\n");
	printf("p : play song\r\n");
	printf("n : play next song\r\n");
	printf("u : play prev song\r\n");
	printf("s : pause song\r\n");
	printf("q : stop song\r\n");
}

int main(int argc, char **argv)
{
	char ch;
	PT_SongOpr ptSongOpr;

	PrintUsage();

	while(1)
	{
		ch = getchar();
		switch(ch)
		{
			case 'A':
			case 'a':
			{
				AddSongs();
				PrintSongList();
				break;
			}
			case 'P':
			case 'p':
			{
				ptSongOpr = GetSongsList();
				PlaySong(ptSongOpr);
				break;
			}
			case 'N':
			case 'n':
			{
				NextSong(ptSongOpr);
				break;
			}
			case 'U':
			case 'u':
			{
				PlaySong(ptSongOpr);
				break;
			}
			case 'S':
			case 's':
			{
				PauseSong(ptSongOpr);
				break;
			}
			case 'Q':
			case 'q':
			{
				StopSong(ptSongOpr);
				goto exit;
			}
			default:
			{
				if (ch != '\r' || ch != '\n')
				{
					printf("error! please reenter:\r\n");
					PrintUsage();
				}
				break;
			}
		}
	}

exit:
	FreeSongLink();
	return 0;
}

