#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFF_SIZE 1024
#define S_MODE 0644

int main(){	
	int fd;			// 파일 디스크립터
	int num;			// 정수
	int index = 0;		// 기호를 저장할 버퍼의 index
	int length;		// 출력할 파일에 출력할 기호의 총 길이
	char ch;			// 기호
	char *fname;		// 출력할 파일 이름
	char buf[BUFF_SIZE];	// 버퍼

	printf("""정수, 기호, 출력할 파일이름""을 입력해주세요 : ");
	scanf("%d, %c, %s", &num, &ch, fname);

	if(num > 0){		// 양수일 경우
		for(int i = 1; i <= num; i++){
			for(int j = 1; j <= i; j++){
				buf[index++] = ch;
			}
			buf[index++] = '\n';
		}
	}
	else if(num < 0){	// 음수일 경우
		for(int i = num; i < 0; i++){
			for(int j = i; j < 0; j++){
				buf[index++] = ch;
			}
			buf[index++] = '\n';
		}
	}
	
	buf[index] = '\0';
	length = strlen(buf);
	
	if((fd = open(fname, O_RDWR|O_CREAT|O_TRUNC, S_MODE)) < 0){ // 파일 생성
		fprintf(stderr, "[ERROR] %s file open error!\n", fname);
		exit(1);
	}
	
	if(write(fd, buf, length) < 0){ // 파일에 출력
		fprintf(stderr, "[ERROR] write error!\n");
		exit(1);
	}
	return 0;
}
