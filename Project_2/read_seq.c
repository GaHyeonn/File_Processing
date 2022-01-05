#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int main(int argc, char **argv){
	struct timeval start, end;
	FILE *fp;
	char *buffer;
	int num, count;
	long int time;
	if(argc != 2){
		printf("instruction error\n");
		return -1;
	}

	if((fp = fopen(argv[1], "r")) == NULL){
		printf("open error for %s\n", argv[1]);
		return -1;
	}

	if((count = fread(&num, sizeof(int), 1, fp)) < 0){
		printf("read error for header record\n");
		return -1;
	}
	
	buffer = (char *)malloc(sizeof(char)*250);

	if((count = gettimeofday(&start, NULL)) < 0){
		printf("gettimeofday error\n");
		return -1;
	}

	for(int i = 0; i<num; i++){
		memset(buffer, 0, sizeof(buffer));
		if(fread(buffer, 250, 1, fp ) < 0){
			printf("read error\n");
			return -1;
		}
	}

	if((count = gettimeofday(&end, NULL)) < 0){
		printf("gettimeofday error\n");
		return -1;
	}
	
	time = ((end.tv_sec - start.tv_sec)*1000000)+(end.tv_usec - start.tv_usec);

	printf("#records: %d elapsed_time: %ld us\n", num, time);

	fclose(fp);
	free(buffer);
	return 0;
}
