#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// �ʿ��ϸ� hearder file�� �߰��� �� ����

int main(int argc, char **argv)
{ // ����ڷκ��� �Է� ���� ���ڵ� ����ŭ�� ���ڵ带 ���ڵ� ���Ͽ� �����ϴ� �ڵ带 ����
	int fd;
	char *buffer;
	int record_num, count;
	
	if(argc != 3){
		fprintf(stderr, "��ɾ� �Է� ����\n");
		exit(1);
	}

	if((record_num = atoi(argv[1])) < 0){
		fprintf(stderr, "���ڵ� �� ���� �Է� ����\n");
		exit(1);
	}

	if((fd = open(argv[2], O_WRONLY | O_CREAT |O_TRUNC, 0640)) < 0){
		fprintf(stderr, "%s ���� ���� ����\n", argv[2]);
		exit(1);
	}

	buffer = (char *)malloc(sizeof(char)*250);
	
	
	if((count = write(fd, &record_num, sizeof(int))) < sizeof(int)){
		fprintf(stderr, "��� ���ڵ� ���� ����\n");
		exit(1);
	}

	for(int i = 0; i<record_num; i++){
		memset(buffer, 0, 250*sizeof(char));
	strcpy(buffer, "20192983 Ga HYeon Park Soongsil Univ 3grade file processing task2");
		if(( count = write(fd, buffer, 250)) !=  250){
			fprintf(stderr, "���ڵ� ���� ����\n");
			exit(1);
		}
		
	}

	free(buffer);
	close(fd);
	return 0;
}
