#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	FILE *fp;
	int offset, f_size, local;
	char array[2] = {0, };

	if(argc == 4){
		fp = fopen(argv[1],"r");
		if(fp == NULL){
			printf("파일을 여는 과정에서 오류 발생\n");
			return 0;
		}

		fp = fopen(argv[1], "r+");
		offset = atoi(argv[2]);
		
		fseek(fp, 0, SEEK_END);
		f_size = ftell(fp);

		if(offset < 0 || offset > f_size-1){
			printf("offset 범위 오류\n");
			return 0;
		}
		
		fseek(fp, offset, SEEK_SET);
		local = ftell(fp);//EOF 삽입 위함

		fseek(fp,-1,SEEK_END);//test
		fread(array, sizeof(char), 1, fp);//EOF 삽입 위함

		fseek(fp, offset, SEEK_SET);

		fputs(argv[3],fp);
		
		if(local+offset >= f_size-2){//EOF 덮어쓴 후 다시 파일 EOF 삽입해서 파일 끝 알려줌.
			fputs(array,fp);
		}
	}
	else{
		printf("명령어 입력 오류\n");
		return 0;
	}

	fclose(fp);
	return 0 ;
}
