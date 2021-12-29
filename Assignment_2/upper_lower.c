#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define S_MODE 0644
#define BUFF_SIZE 1024

int main(){
	int fd1, fd2;		// 파일 디스크립터
	int fname_size;		// 입력 파일 이름의 길이
	int length;		// 입력 파일의 크기
	char f1[100];		// 입력 파일 이름
	char f2[100];		// 출력 파일 이름
	char buf[BUFF_SIZE];	// 입력 파일내용을 read한 문자열
	char print[BUFF_SIZE];	// buf를 대문자는 소문자로, 소문자는 대문자로 변경한 문자열

	printf("파일 이름을 입력하세요 : ");
	scanf("%s", f1);
	fname_size = strlen(f1);
	
	// 입력파일 f1의 대소문자 변경하여 출력파일 f2의 이름을 저장  
	for(int i = 0; i < fname_size; i++){
		if(f1[i] >= 'a' && f1[i] <= 'z')
			f2[i] = (f1[i] - 'a') + 'A';
		else if(f1[i] >= 'A' && f1[i] <= 'Z')
			f2[i] = (f1[i] - 'A') + 'a';
		else
			f2[i] = f1[i];
	}
	f2[fname_size] = '\0';

	// 입력 파일 열기
	if((fd1 = open(f1, O_RDONLY)) < 0){
		fprintf(stderr, "%s file open error!\n", f1);
		exit(1);
	}

	// 출력 파일 생성 및 열기
	if((fd2 = open(f2, O_RDWR|O_CREAT|O_TRUNC, S_MODE)) < 0){
		fprintf(stderr, "%s file open error!\n", f2);
		exit(1);
	}
	
	// 입력 파일 내용 읽기
	if((length = read(fd1, buf, BUFF_SIZE)) < 0){
		fprintf(stderr, "%s file read error!\n", f1);
		exit(1);
	}

	// 입력 파일 내용 대소문자 변경해서 새로운 배열에 저장
	for(int i = 0; i < length; i++){
		if(buf[i] >= 'a' && buf[i] <= 'z')
			print[i] = (buf[i] - 'a') + 'A';
		else if(buf[i] >= 'A' && buf[i] <= 'Z')
			print[i] = (buf[i] - 'A') + 'a';
		else
			print[i] = buf[i];
	}
	print[length]= '\0';

	// 변경한 내용을 출력 파일에 쓰기
	write(fd2, print, length);

	return 0;
}
