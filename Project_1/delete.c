#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv){
	char *buffer;
	int f_size, local, b_size, final;
	int byte, offset, fp;

	if(argc == 4){
		fp = open(argv[1],O_RDWR);
		
		if(fp < 0){
			printf("파일 여는 과정에서 오류 발생\n");
			return 0;
		}
		
		offset = atoi(argv[2]);
		byte = atoi(argv[3]);

		if(offset < 0 || byte < 0){
			printf("<오프셋> 이나 <삭제 바이트 수> 입력 오류\n");
			return 0;
		}

		lseek(fp, 0, SEEK_END);
		f_size = lseek(fp, 0, SEEK_CUR);
		
		if(offset > f_size){
			return 0;
		}
		
		lseek(fp, offset+byte, SEEK_SET);
		local = lseek(fp, 0, SEEK_CUR);
		b_size = f_size - local;

		buffer = (char *)malloc(sizeof(char)*b_size);

		read(fp, buffer, sizeof(char)*b_size);

		lseek(fp, offset, SEEK_SET);

		write(fp, buffer, sizeof(char)*b_size);
			
		final = lseek(fp, 0, SEEK_CUR);

		ftruncate(fp, final);
	}	
	else{
		printf("명령어 입력 오류\n");
		return 0;
	}

	close(fp);
	free(buffer);

	return 0;
}
