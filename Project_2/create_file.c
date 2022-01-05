#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// 필요하면 hearder file을 추가할 수 있음

int main(int argc, char **argv)
{ // 사용자로부터 입력 받은 레코드 수만큼의 레코드를 레코드 파일에 저장하는 코드를 구현
	int fd;
	char *buffer;
	int record_num, count;
	
	if(argc != 3){
		fprintf(stderr, "명령어 입력 오류\n");
		exit(1);
	}

	if((record_num = atoi(argv[1])) < 0){
		fprintf(stderr, "레코드 수 음수 입력 오류\n");
		exit(1);
	}

	if((fd = open(argv[2], O_WRONLY | O_CREAT |O_TRUNC, 0640)) < 0){
		fprintf(stderr, "%s 파일 오픈 오류\n", argv[2]);
		exit(1);
	}

	buffer = (char *)malloc(sizeof(char)*250);
	
	
	if((count = write(fd, &record_num, sizeof(int))) < sizeof(int)){
		fprintf(stderr, "헤더 레코드 쓰기 오류\n");
		exit(1);
	}

	for(int i = 0; i<record_num; i++){
		memset(buffer, 0, 250*sizeof(char));
	strcpy(buffer, "20192983 Ga HYeon Park Soongsil Univ 3grade file processing task2");
		if(( count = write(fd, buffer, 250)) !=  250){
			fprintf(stderr, "레코드 쓰기 오류\n");
			exit(1);
		}
		
	}

	free(buffer);
	close(fd);
	return 0;
}
