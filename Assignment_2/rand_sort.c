#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFF_SIZE 128
#define S_MODE 0664
#define S_FLAG O_RDWR|O_APPEND|O_CREAT|O_TRUNC

// Quick Sort에 쓰이는 비교 함수 
int compare(const void *a, const void *b)
{
	int num1 = *(int *)a;
	int num2 = *(int *)b;

	if(num1 < num2)
		return -1;
	else if(num1 > num2)
		return 1;
	else
		return 0;
}

int main()
{
	int num;			// 임의의 자연수
	int fd1,fd2;		// 파일 디스크립터
	int length;		// 난수 문자열의 길이
	int index;		// 버퍼의 인덱스
	int line_count = 0;	// 난수 파일의 라인 수 (즉 난수의 개수)
	int *numArr;		// 난수를 정수로 저장할 동적 할당할 배열
	char buf[BUFF_SIZE];	// read / write에 쓰일 버퍼 
	char character;		// read / write에 쓰일 문자
	
	srand(time(NULL));
	
	printf("임의의 자연수를 입력하세요 : ");
	scanf("%d", &num);
	
	// 난수 생성하여 출력할 파일 생성
	if((fd1 = open("RANDOM_NUM.txt", S_FLAG, S_MODE)) < 0){
		fprintf(stderr, "RANDOM_NUM.txt file open error!\n");
		exit(1);
	}
	
	// 난수를 정렬하여 출력할 파일 생성
	if((fd2 = open("SORTED_NUM.txt", S_FLAG, S_MODE)) < 0){
		fprintf(stderr, "SORTED_NUM.txt file open error!\n");
		exit(1);
	}
	
	// 난수를 생성하여 첫 번째 파일에 출력
	for(int i = 0; i < num; i++){
		memset(buf, 0, BUFF_SIZE);
		length = sprintf(buf, "%d\n", rand());
		write(fd1, buf, length);	
	}
	
	// 오프셋 위치를 처음으로 변경
	if(lseek(fd1, 0, SEEK_SET) < 0){
		fprintf(stderr, "lseek error!\n");
		exit(1);
	}
	
	// 난수의 개수를 셈(개행으로 난수를 구분하였음)
	while(1){
		if(read(fd1, &character, 1) > 0){
			if(character == '\n')
				line_count++;
		}
		else
			break;
	}
	
	numArr = (int *) malloc(sizeof(int) * line_count);	// 난수의 개수 만큼 정수형 배열 생성
	
	// 오프셋 위치를 처음으로 변경
	if(lseek(fd1, 0, SEEK_SET) < 0){
		fprintf(stderr, "lseek error!\n");
		exit(1);
	}
	
	// 첫 번째 파일의 난수를 읽어 정수형 배열에 난수의 수 만큼 각각 저장
	for(int i = 0; i < line_count; i++){
		index = 0;
		memset(buf, 0, BUFF_SIZE);

		while(read(fd1, &character, 1) > 0){
			if(character == '\n'){
				buf[index] = '\0';
				numArr[i] = atoi(buf);
				break;
			}
			buf[index++] = character;
		}
	}
	
	qsort(numArr, line_count, sizeof(int), compare);	// 오름차순 정렬
	
	// 정렬한 난수들을 두 번째 파일에 출력 
	for(int i = 0; i < line_count; i++){
		memset(buf, 0, BUFF_SIZE);
		length = sprintf(buf, "%d\n", numArr[i]);
		write(fd2, buf, length);
	}

	free(numArr);
	
	return 0;
}
