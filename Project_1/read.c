#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){
	char buffer[2];
	FILE *fp;
	int offset, byte, count, f_size, cur;

	if(argc == 4){
		fp = fopen(argv[1], "r");
		offset = atoi(argv[2]);
		byte = atoi(argv[3]);
	 
		if(fp == NULL){
			printf("파일을 여는 과정에서 오류 발생\n");
			return 0;
		}

		fseek(fp, 0, SEEK_END);
		f_size = ftell(fp);

		if(offset < 0 || offset > f_size-2 || byte < 0){
			printf("<오프셋>이나 <읽기 바이트 수> 입력 범위 오류\n");
			return 0;
		}
		else if(byte == 0){
			return 0;
		}
				
		fseek(fp, offset, SEEK_SET);
		cur = ftell(fp);
				
		if(byte > f_size-cur-1){
			for(int i = 0; i< f_size-cur-1; i++){
				fread(buffer, sizeof(char),1,fp);
				fwrite(buffer, sizeof(char),1,stdout);
			}
			printf("\n");
		}
		else{
			for(int i = 0; i<byte; i++){
				fread(buffer, sizeof(char),1,fp);
				fwrite(buffer, sizeof(char),1,stdout);
			}
			printf("\n");
		}
			}
	else{
		printf("명령어 입력 오류\n");
		return 0;
	}
	
	fclose(fp);
	return 0;
}
