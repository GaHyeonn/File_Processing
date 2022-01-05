#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	FILE *fp;
	int offset;
	long int f_size, location, b_size;
	char *buffer;
	
	if(argc == 4){

		fp = fopen(argv[1],"r");
		if(fp == NULL){
			printf("파일 여는 과정에서 오류 발생\n");
			return 0;
		}

		fp = fopen(argv[1],"r+");
		offset = atoi(argv[2]);
		
		fseek(fp, 0, SEEK_END);
		f_size = ftell(fp);

		if(offset < 0 || offset > f_size-1 ){
			printf("offset 입력 범위 오류\n");
			return 0;
		}
	
		fseek(fp, offset+1, SEEK_SET);
		location = ftell(fp);
		
		b_size = f_size - location;
		buffer = (char *)malloc(sizeof(char)*b_size);
		
		fread(buffer, sizeof(char), b_size, fp);
		fseek(fp, offset+1, SEEK_SET);
		fputs(argv[3],fp);
		fwrite(buffer, sizeof(char), b_size, fp);
	}
	else{
		printf("명령어 입력 오류\n");
		return 0;
	}

	free(buffer);
	fclose(fp);
	
	return 0;
}
