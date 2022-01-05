#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
//필요하면 header file 추가 가능

#define SUFFLE_NUM	10000	// 이 값은 마음대로 수정할 수 있음.

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);
// 필요한 함수가 있으면 더 추가할 수 있음.

int main(int argc, char **argv)
{	struct timeval start, end;
	int *read_order_list;
	int num_of_records; // 레코드 파일에 저장되어 있는 전체 레코드의 수
	int count, num;
	char *buffer;
	long int time;
	FILE *fp;

	if(argc != 2){
		printf("argc error\n");
		return -1;
	}

	if((fp = fopen(argv[1], "r")) == NULL){
		printf("open error\n");
		return -1;
	}

	if((count = fread(&num_of_records, sizeof(int), 1, fp)) < 0){
		printf("read error\n");
		return -1;
	}

	read_order_list = (int *)malloc(sizeof(int)*num_of_records);
	// 이 함수를 실행하면 'read_order_list' 배열에는 읽어야 할 레코드 번호들이 나열되어 저장됨
	GenRecordSequence(read_order_list, num_of_records);

	buffer = (char *)malloc(sizeof(char)*250);
	// 'read_order_list'를 이용하여 random 하게 read 할 때 걸리는 전체 시간을 측정하는 코드 구현
	
	if((count = gettimeofday(&start, NULL)) < 0){
		printf("gettimeofday error\n");
		return -1;
	}
	for(int i = 0; i<num_of_records; i++){
		memset(buffer, 0, 250);
		num = read_order_list[i] * 250;
		fseek(fp, num +4, SEEK_SET);
		if(fread(buffer, 250, 1, fp) < 0){
			printf("read error\n");
			return -1;
		}
	}

	if((count = gettimeofday(&end, NULL)) < 0){
		printf("gettimeofday error\n");
		return -1;
	}
	
	time = ((end.tv_sec - start.tv_sec)*1000000)+(end.tv_usec - start.tv_usec);

	printf("#records: %d elapsed_time: %ld us\n", num_of_records , time);

	fclose(fp);
	free(buffer);
	free(read_order_list);

	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}
