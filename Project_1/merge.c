#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
	FILE *fp1, *c_fp2, *c_fp3;
	int count;
	char *buffer;

	if(argc == 4){
		fp1 = fopen(argv[1],"w");
		c_fp2 = fopen(argv[2],"r");
		c_fp3 = fopen(argv[3],"r");

		if(fp1 == NULL || c_fp2 == NULL || c_fp3 ==NULL){
			printf("파일 여는 과정에서 오류 발생\n");
			return 0;
		}
		else{
			buffer = (char *)malloc(sizeof(char)*50);
			
			while(!feof(c_fp2)){
				memset(buffer, 0, 50);
				count = fread(buffer, sizeof(char), 50, c_fp2);
				fwrite(buffer, sizeof(char), count, fp1);
			}

			while(!feof(c_fp3)){
				memset(buffer, 0, 50);
				count = fread(buffer, sizeof(char), 50, c_fp3);
				fwrite(buffer, sizeof(char), count, fp1);
			}
		}

		free(buffer);
	}
	else{
		printf("명령어 오류 발생\n");
		return 0;
	}

	fclose(fp1); 
	fclose(c_fp2);
	fclose(c_fp3);

	return 0;
}
