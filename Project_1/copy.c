#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){
	char array[10]={0,};
	char *buffer;
	int total_size, b_size, repeat;
	FILE *original_fp, *copy_fp;
	
	if(argc == 3){
		original_fp = fopen(argv[1],"rb");
		copy_fp = fopen(argv[2],"wb");

		if(original_fp == NULL || copy_fp == NULL){
			printf("파일을 여는 과정에서 오류 발생\n");
			return 0;
		}

		fseek(original_fp, 0,SEEK_END);
		total_size = ftell(original_fp);
		repeat = total_size/10;
		b_size = total_size%10;
		
		fseek(original_fp, 0, SEEK_SET);
		buffer = (char *)malloc(sizeof(char)*b_size);

		for(int i = 0; i<repeat; i++){
			fread(array, sizeof(char)*10, 1, original_fp);
			fwrite(array,sizeof(char)*10, 1, copy_fp);
		}

		fread(buffer, sizeof(char)*b_size, 1, original_fp);
		fwrite(buffer, sizeof(char)*b_size, 1, copy_fp);
	}
	else{
		printf("입력 오류 발생\n");
		return 0;
	}

	fclose(original_fp);
	fclose(copy_fp);
	free(buffer);

	return 0;
}
