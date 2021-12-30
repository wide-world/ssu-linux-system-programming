#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_FILE 20

enum {ALL, USER, GROUP, OTHER, PLUS, MINUS, EQUAL, READ, WRITE, EXECUTE, ID, STICKY, ERROR, EXIT, COMMA} token;
bool readOn = false;
bool writeOn = false;
bool executeOn = false;
bool idOn = false;
bool stickyOn = false;
bool isNumMode = false;
bool isError = false;
int token_index;
int result;
mode_t statmode;

void get_token(char *, int length);
void checkPerm(char *);
void changePerm(char *); 
void target(char *, int);
void oper(char *, int, char);
int mode(char *, int);

// 기호 모드 일 경우, 문자를 하나 씩 읽어서 의미를 판별
void get_token(char *p, int length)
{
	char point;
	point = p[token_index];

	if (point == 'a')
		token = ALL;
	else if (point == 'u')
		token = USER;
	else if (point == 'g')
		token = GROUP;
	else if (point == 'o')
		token = OTHER;
	else if (point == '+')
		token = PLUS;
	else if (point == '-')
		token = MINUS;
	else if (point == '=')
		token = EQUAL;
	else if (point == 'r')
		token = READ;
	else if (point == 'w')
		token = WRITE;
	else if (point == 'x')
		token = EXECUTE;
	else if (point == 's')
		token = ID;
	else if (point == 't')
		token = STICKY;
	else if (point == ',')
		token = COMMA;
	else
		token = ERROR;

	if (token_index == length)
		token = EXIT;

	token_index++;
}

// 기호 모드 일 경우, 허가 권한 바꾸는 함수 실행
void changePerm(char *perm)
{
	int length;

	length = strlen(perm);
	get_token(perm, length);
	while (token != EXIT) {
		target(perm, length);

		if (isError)
			break;
	}
}

// all, user, group, other 등의 권한을 부여하기 위한 함
void target(char *perm, int length)
{
	while (1) {
		if (token == ALL) {
			get_token(perm, length);
			oper(perm, length, 'a');
		}
		else if (token == USER) {
			get_token(perm, length);수
			oper(perm, length, 'u');
		}
		else if (token == GROUP) {
			get_token(perm, length);
			oper(perm, length, 'g');
		}
		else if (token == OTHER) {
			get_token(perm, length);
			oper(perm, length, 'o');
		}
		else {
			isError = true;
			break;
		}
		readOn = false;
		writeOn= false;
		executeOn = false;
		idOn = false;
		stickyOn = false;

		if (token == EXIT || isError)
			break;
		
		if (token == COMMA)
			get_token(perm, length);
	}
}

// user, group, other 등에 +,-,= 등의 연산자로 권한을 부여하거나 해제
void oper(char *perm, int length, char ref)
{
	mode_t temp_mode;
	int num;
	int id_sticky = 0;
	int user = 0;
	int group = 0;
	int other = 0;
	
	if (token == PLUS) {
		get_token(perm, length);
		num = mode(perm, length);
		while (token == READ || token == WRITE || token == EXECUTE || token == ID || token == STICKY) {
			num = num + mode(perm, length);
			if (token == COMMA || token == EXIT || isError)
				break;
		}

		if (ref == 'a') {
			if (idOn)
				id_sticky += 6;
			if (stickyOn)
				id_sticky += 1;
			id_sticky = id_sticky << 9;
			user = num;
			user = user << 6;
			group = num;
			group = group << 3;
			other = num;
			statmode = (statmode | id_sticky | user | group | other);
		}
		else if (ref == 'u') {
			if (idOn)
				id_sticky = 4;
			id_sticky = id_sticky << 9;
			user = num;
			user = user << 6;
			statmode = (statmode | id_sticky | user);
		}
		else if (ref == 'g') {
			if (idOn)
				id_sticky = 2;
			id_sticky = id_sticky << 9;
			group = num;
			group = group << 3;
			statmode = (statmode | id_sticky | group);
		}
		else if (ref == 'o') {
			if (stickyOn)
				id_sticky = 1;
			id_sticky = id_sticky << 9;
			other = num;
			statmode = (statmode | id_sticky | other);
		}

	}
	else if (token == MINUS) {
		get_token(perm, length);
		num = mode(perm, length);
		while (token == READ || token == WRITE || token == EXECUTE || token == ID || token == STICKY) {
			num = num + mode(perm, length);
			if (token == COMMA || token == EXIT || isError)
				break;
		}

		if (ref == 'a') {
			if (idOn)
				id_sticky += 6;
			if (stickyOn)
				id_sticky += 1;
			id_sticky = id_sticky << 9;
			user = num;
			user = user << 6;
			group = num;
			group = group << 3;
			other = num;
			statmode &= ~id_sticky;
			statmode &= ~user;
			statmode &= ~group;
			statmode &= ~other;
		}
		else if (ref == 'u') {
			if (idOn)
				id_sticky = 4;
			id_sticky = id_sticky << 9;
			user = num;
			user = user << 6;
			statmode &= ~id_sticky;
			statmode &= ~user;
		}
		else if (ref == 'g') {
			if (idOn)
				id_sticky = 2;
			id_sticky = id_sticky << 9;
			group = num;
			group = group << 3;
			statmode &= ~id_sticky;
			statmode &= ~group;
		}
		else if (ref == 'o') {
			if (stickyOn)
				id_sticky = 1;
			id_sticky = id_sticky << 9;
			other = num;
			statmode &= ~id_sticky;
			statmode &= ~other;
		}
	}
	else if (token == EQUAL) {
		get_token(perm, length);
		num = mode(perm, length);
		while (token == READ || token == WRITE || token == EXECUTE || token == ID || token == STICKY) {
			num = num + mode(perm, length);
			if (token == COMMA || token == EXIT || isError)
				break;
		}

		if (ref == 'a') {
			if (idOn)
				id_sticky += 6;
			if (stickyOn)
				id_sticky += 1;
			id_sticky = id_sticky << 9;
			user = num;
			user = user << 6;
			group = num;
			group = group << 3;
			other = num;

			statmode &= ~(7 << 9);
			statmode |= id_sticky;
			statmode &= ~(7 << 6);
			statmode |= user;
			statmode &= ~(7 << 3);
			statmode |= group;
			statmode &= ~7;
			statmode |= other;			
		}
		else if (ref == 'u') {
			if (idOn)
				id_sticky = 4;
			id_sticky = id_sticky << 9;
			user = num;
			user = user << 6;
			statmode &= ~(4 << 9);
			statmode |= id_sticky;
			statmode &= ~(7 << 6);
			statmode |= user;			
		}
		else if (ref == 'g') {
			if (idOn)
				id_sticky = 2;
			id_sticky = id_sticky << 9;
			group = num;
			group = group << 3;
			statmode &= ~(2 << 9);
			statmode |= id_sticky;
			statmode &= ~(7 << 3);
			statmode |= group;
		}
		else if (ref == 'o') {
			if (stickyOn)
				id_sticky = 1;
			id_sticky = id_sticky << 9;
			other = num;
			statmode &= ~(1 << 9);
			statmode |= id_sticky;
			statmode &= ~7;
			statmode |= other;
		}
	}
	else
		isError = true;
}

// read, write, execute, sticky, user & group id 권한 판단
int mode(char *perm, int length)
{
	int num;
	if (token == READ && !readOn) {
		num = 4;
		readOn = true;
		get_token(perm, length);
	}
	else if (token == WRITE && !writeOn) {
		num = 2;
		writeOn = true;
		get_token(perm, length);
	}
	else if (token == EXECUTE && !executeOn) {
		num = 1;
		executeOn = true;
		get_token(perm, length);
	}
	else if (token == ID) {
		num = 0;
		idOn = true;
		get_token(perm, length);
	}
	else if (token == STICKY) {
		num = 0;
		stickyOn = true;
		get_token(perm, length);
	}
	else {
		isError = true;
		num = 0;
	}

	return num;		
}

// 모드가 숫자인지 기호인지 판단
void checkPerm(char *perm)
{
	int length = strlen(perm);
	int num;
	int id_sticky = 0;
	int user = 0;
	int group = 0;
	int other = 0;

	for (int i = 0; i < length; i++) {
		if (perm[i] >= '0' && perm[i] <= '7')
			isNumMode = true;
		else if (perm[i] == '8' || perm[i] == '9') {
			fprintf(stderr, "chmod: 잘못된 모드: '%s'\n", perm);
			isError = true;
			break;
		}
		else {
			isNumMode = false;
			break;
		}
	}

	if (isNumMode) {
		num = atoi(perm);

		if (num > 7777) {
			fprintf(stderr, "chmod: 잘못된 모드: '%s'\n", perm);
			isError = true;
		}

		if (num >= 1000) {
			id_sticky = num / 1000;
			id_sticky = id_sticky << 9;
			num %= 1000;
		}

		if (num >= 100) {
			user = num / 100;
			user = user << 6;
			num %= 100;
		}

		if (num >= 10) {
			group = num / 10;
			group = group << 3;
			num %= 10;
		}

		other = num;
		result = id_sticky + user + group + other;
	}	
}

int main(int argc, char *argv[])
{
	struct stat statbuf;

	if (argc < 3) {
		fprintf(stderr, "usage: <%s> <permission> <file1> ... <fileN>\n", argv[0]);
		exit(0);
	}

	checkPerm(argv[1]);

	for (int i = 2; i < argc; i++) {
		if (lstat(argv[i], &statbuf) < 0) {
			fprintf(stderr, "lstat error for %s\n", argv[i]);
			continue;
		}

		if (isNumMode) {
			if (chmod(argv[i], result) < 0) {
				fprintf(stderr, "chmod error for %s\n", argv[i]);
				exit(1);
			}
		}
		else {
			statmode = 0;
			statmode = statbuf.st_mode;
			changePerm(argv[1]);
			if (isError) {
				fprintf(stderr, "chmod: 잘못된 모드: '%s'\n", argv[1]);
				break;
			}

			if (chmod(argv[i], statmode) < 0) {
				fprintf(stderr, "chmod error for %s\n", argv[i]);
				exit(1);
			}
		}
	}					

	exit(0);
}
