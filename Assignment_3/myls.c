#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

#define MAX_DIR 1000		// 명령어로 인식 가능한 최대 디렉토리 수
#define MIN_BLKSIZE 4096	// 블록 안에서 할당된 각 파일 사이즈의 최소 단위

struct statInfo {
	char type;
	char perm[10];
	char uid[10];
	char gid[10];
	char time[20];	// 날짜 및 시간
	unsigned long nlink;	// 하드 링크 수
	unsigned long inode;	// inode값
	long size;		// 파일 크기
	long mtime;		// 최근 수정 시간
	char file[30];	// 파일 이름
};

char type(mode_t);
char* perm(mode_t, char *);
void saveStat(char *, char *, struct stat *, struct statInfo *);
void sortStat(struct statInfo *, int);
void printStat(struct statInfo *, int);
void printStatList(struct statInfo *, int);
void calTotalSize(struct statInfo *, int);
int argCheck(int, char**);

char *dir[MAX_DIR];
char *file[MAX_DIR];
bool optAll = false;
bool optInode = false;
bool optList = false;
bool optModTime = false;
bool isFile = false;
off_t totalSize;

int main(int argc, char **argv)
{
	DIR *dp;
	struct statInfo si[MAX_DIR];
	struct stat st;
	struct dirent *d;

	char path[PATH_MAX]; 
	int num_of_dir;		 //	인자로 받은 디렉토리 수 
	int readCnt;		 // readdir 호출 횟수

	num_of_dir = argCheck(argc, argv);	// 수여러 개의 디렉토리 또는 명령어의 옵션 등을 판단하는 함수

	for (int i = 0; i < num_of_dir; i++) {
		readCnt = 0;
		totalSize = 0;
		isFile = false;

		if (file[i] != NULL)
			isFile = true;
		else
			putchar('\n');

		if ((dp = opendir(dir[i])) == NULL)
			perror(dir[i]);

		if (num_of_dir > 1 && !isFile)
			printf("%s:\n", dir[i]);

		while ((d = readdir(dp)) != NULL) {
			sprintf(path, "%s/%s", dir[i], d->d_name);	// 파일 경로명 만들기
			if (lstat(path, &st) < 0)			// 파일 상태 정보 가져오기
				perror(path);
			if (isFile) {
				if (strcmp(d->d_name, file[i]) == 0)
					saveStat(path, d->d_name, &st, &si[readCnt++]);
			}
			else {
				saveStat(path, d->d_name, &st, &si[readCnt++]);		// 상태 정보 구조체에 저장
				totalSize += (int) (ceil((double) st.st_size / (double) MIN_BLKSIZE) * 4);	// myls -l 합계
			}
		}

		sortStat(si, readCnt);


		if (optList) {// ls -l 명령어 
			if (!isFile)
				calTotalSize(si, readCnt);
			printStatList(si, readCnt);
		}
		else {
			printStat(si, readCnt);
			if (!isFile)
				putchar('\n');
		}

		if (num_of_dir > 1 && !isFile)
			putchar('\n');

		closedir(dp);
	}
	putchar('\n');
	exit(0);
}

// 옵션 여부 판단 및 디렉토리 이름 순으로 정렬
int argCheck(int argc, char **argv)
{
	DIR *dp;
	int fd;
	int dir_count = 0;
	char *temp_d;
	char *temp_f;

	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-", 1) == 0) {	// 옵션에 따른 flag 설정
			if (strchr(argv[i], 'a') != NULL)
				optAll = true;
			if (strchr(argv[i], 'i') != NULL)
				optInode = true;
			if (strchr(argv[i], 'l') != NULL)
				optList = true;
			if (strchr(argv[i], 't') != NULL)
				optModTime = true;
		}
		else {	// 인자가 파일인지 디렉토리인지 판단 
			if ((dp = opendir(argv[i])) == NULL) {	
				if ((fd = open(argv[i], O_RDONLY)) < 0) {
					fprintf(stderr, "ls: '%s'에 접근할 수 없습니다. 그런 파일이나 디렉토리가 없습니다.\n", argv[i]);
					exit(0);
				}
				file[dir_count] = argv[i];
				dir[dir_count++] = ".";
				close(fd);
			}
			else {
				file[dir_count] = NULL;
				dir[dir_count++] = argv[i];
			}
			closedir(dp);
		}
	}

	if (dir_count == 0)
		dir[dir_count++] = ".";

	// 파일 및 디렉토리 정렬
	for (int i = 0; i < dir_count - 1; i++)
		for (int j = 0; j < dir_count - 1 - i; j++)
			if (strcmp(dir[j], dir[j+1]) > 0) {
				temp_d = dir[j];
				temp_f = file[j];
				dir[j] = dir[j+1];
				file[j] = file[j+1];
				dir[j+1] = temp_d;
				file[j+1] = temp_f;
			}

	return dir_count;
}

// 파일 상태 정보를 구조체리스트에 저장
void saveStat(char *pathname, char *file, struct stat *st, struct statInfo *si)
{
	char perms[10];
	memset(perms, '-', 9);
	perms[9] = 0;

	si->type = type(st->st_mode);
	sprintf(si->perm, "%s", perm(st->st_mode, perms));
	si->nlink = st->st_nlink;
	si->size = st->st_size;
	sprintf(si->uid, "%s", getpwuid(st->st_uid)->pw_name);
	sprintf(si->gid, "%s", getgrgid(st->st_gid)->gr_name);
	sprintf(si->time, "%s", ctime(&st->st_mtime) + 4);
	sprintf(si->file, "%s", file);
	si->inode = st->st_ino;
	si->mtime = st->st_mtime;
}

// 파일의 간단한 정보 출력
void printStat(struct statInfo *si, int n)
{
	for (int i = 0; i < n; i++) {
		if (optAll) {	// ls -a 옵션일 경우 숨김 파일 이름 전부 표시
			if (optInode)
				printf("%lu ", si[i].inode);
			printf("%s  ", si[i].file);
		}
		else {			// ls 명령어로 파일 이름 출력
			if (strncmp(si[i].file, ".", 1) != 0) {
				if (optInode)
					printf("%lu ", si[i].inode);
				printf("%s  ", si[i].file);
			}
			else
				totalSize -= 4;
		}
	}
}

// 파일 상태 정보를 리스트로 출력
void printStatList(struct statInfo *si, int n)
{
	for (int i = 0; i < n; i++) {
		if (optAll) {	// ls -a 옵션일 경우 숨김 파일 전부 표시
			if (optInode)
				printf("%lu ", si[i].inode);
			printf("%c%s ", si[i].type, si[i].perm);
			printf("%lu ", si[i].nlink);
			printf("%-s %-s ", si[i].uid, si[i].gid);
			printf("%6ld ", si[i].size);
			printf("%.12s ", si[i].time);
			printf("%s", si[i].file);
			putchar('\n');
		}
		else {			// 숨김 파일은 제거하고 표시
			if (strncmp(si[i].file, ".", 1) != 0) {
				if (optInode)
					printf("%lu ", si[i].inode);
				printf("%c%s ", si[i].type, si[i].perm);
				printf("%lu ", si[i].nlink);
				printf("%-s %-s ", si[i].uid, si[i].gid);
				printf("%6ld ", si[i].size);
				printf("%.12s ", si[i].time);
				printf("%s", si[i].file);
				putchar('\n');
			}
		}
	}
}

// ls -t인 경우 최종 수정시간 순서로 정렬, 아닌 경우는 이름 순으로 정렬
void sortStat(struct statInfo *si, int n)
{
	struct statInfo temp;
	if (optModTime) {
		for (int i = 0; i < n - 1; i++)
			for (int j = 0; j < n - 1 - i; j++)
				if (si[j].mtime < si[j+1].mtime) {
					memcpy(&temp, &si[j], sizeof(si[j]));
					memcpy(&si[j], &si[j+1], sizeof(si[j+1]));
					memcpy(&si[j+1], &temp, sizeof(temp));
				}
	}
	else {
		for (int i = 0; i < n - 1; i++)
			for (int j = 0; j < n - 1 - i; j++)
				if (strcmp(si[j].file, si[j+1].file) > 0) {
					memcpy(&temp, &si[j], sizeof(si[j]));
					memcpy(&si[j], &si[j+1], sizeof(si[j+1]));
					memcpy(&si[j+1], &temp, sizeof(temp));
				}
	}
}

// ls -l 명령어인 경우에 나오는 블록 크기 총합을 계산하여 출력
void calTotalSize(struct statInfo *si, int n)
{
	for (int i = 0; i < n; i++)
		if (!optAll && strncmp(si[i].file, ".", 1) == 0)
			totalSize -= 4;

	printf("합계 %ld\n", totalSize);
}

// 파일 타입을 리턴
char type(mode_t mode)
{
	if (S_ISREG(mode))
		return('-');
	if (S_ISDIR(mode))
		return('d');
	if (S_ISCHR(mode))
		return('c');
	if (S_ISBLK(mode))
		return('b');
	if (S_ISLNK(mode))
		return('l');
	if (S_ISFIFO(mode))
		return('p');
	if (S_ISSOCK(mode))
		return('s');
}

// 파일 사용권한을 리턴
char* perm(mode_t mode, char *perms) {

	if (mode & S_IRUSR)	perms[0] = 'r';
	if (mode & S_IWUSR)	perms[1] = 'w';
	if (mode & S_IXUSR) {
		if (mode & S_ISUID)
			perms[2] = 's';
		else
			perms[2] = 'x';
	}
	else if (mode & S_ISUID)
		perms[2] = 'S';

	if (mode & S_IRGRP)	perms[3] = 'r';
	if (mode & S_IWGRP)	perms[4] = 'w';
	if (mode & S_IXGRP) {
		if (mode & S_ISGID)
			perms[5] = 's';
		else
			perms[5] = 'x';
	}
	else if (mode & S_ISGID)
		perms[5] = 'S';

	if (mode & S_IROTH) perms[6] = 'r';
	if (mode & S_IWOTH)	perms[7] = 'w';
	if (mode & S_IXOTH) {
		if (mode & S_ISVTX)
			perms[8] = 't';
		else
			perms[8] = 'x';
	}
	else if (mode & S_ISVTX)
		perms[8] = 'T';

	return(perms);
}
