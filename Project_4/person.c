#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.
//
// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//

void readPage(FILE *fp, char *pagebuf, int pagenum);
void writePage(FILE *fp, const char *pagebuf, int pagenum);
void pack(char *recordbuf, const Person *p);
void unpack(const char *recordbuf, Person *p);
void add(FILE *fp, const Person *p);
void delete(FILE *fp, const char *id);

void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	memset(pagebuf, 0, PAGE_SIZE);
	fseek(fp, sizeof(int)*4 + pagenum*PAGE_SIZE, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, sizeof(int)*4 + pagenum*PAGE_SIZE, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	memset(recordbuf, 0, sizeof(recordbuf));
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->id, p->name, p->age, p->addr, p->phone, p->email);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	memset(p, 0, sizeof(Person));
	sscanf(recordbuf, "%[^'#']#%[^'#']#%[^'#']#%[^'#']#%[^'#']#%[^'#']#", p->id, p->name, p->age, p->addr, p->phone, p->email);
	//sscanf(recordbuf, "%s#%s#%s#%s#%s#%s#", p->id, p->name, p->age, p->addr, p->phone, p->email);
	
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{
	char *recordbuf;
	char *pagebuf;
	int *metadata;
	int packlen;//packing한 레코드 사이즈
	int ptotalrecord = 0;//페이지에 저장된 record 수
	int pagenum;//페이지 넘버
	int offset = 0;
	int length = 0;
	int prepage;
	int prerecord;
	int newprepage;
	int newprerecord;
	int count = 0;//삭제 레코드 순회할 때 제일 최근에 삭제된 레코드인지 확인
	int changepage;
	int preoffset;
	
	recordbuf = (char *)malloc(sizeof(char) * MAX_RECORD_SIZE);
	pagebuf = (char *)malloc(PAGE_SIZE);
	metadata = (int *)malloc(sizeof(int) * 4);
	
	memset(recordbuf, 0, sizeof(recordbuf));
	memset(pagebuf, 0, PAGE_SIZE);
	memset(metadata, 0, sizeof(int));
	
	pack(recordbuf, p);
	if((packlen = strlen(recordbuf)) > MAX_RECORD_SIZE){//packing한 data 사이즈 확인
		fprintf(stderr, "packing data size over\n");
		exit(1);
	}
	
	fseek(fp,0,SEEK_SET);
	fread(metadata, sizeof(int)*4, 1, fp);//헤더 레코드에서 메타데이터 읽어오기
	
	prepage = metadata[2];
	prerecord = metadata[3];
	
	if((prepage == -1) && (prerecord == -1)){//삭제된 데이터가 없는 경우
		if(metadata[0] == 0){//레코드 파일에 아무것도 안쓰여 있는 경우
			pagenum = 0;
			ptotalrecord++;
			offset = 0;
			
			metadata[0] = 1;
			metadata[1] = 1;
			
			memcpy(pagebuf, &ptotalrecord, sizeof(int));
			memcpy(pagebuf+sizeof(int), &offset, sizeof(int));
			memcpy(pagebuf+(sizeof(int)*2), &packlen, sizeof(int));
			memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, MAX_RECORD_SIZE);
			
			fseek(fp, 0, SEEK_SET);
			fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터 갱신
			writePage(fp, pagebuf, pagenum);//데이터 페이지 갱신
			
		}
		else{//데이터 페이지 존재
			pagenum = metadata[0]-1;
			readPage(fp, pagebuf, pagenum);
			memcpy(&ptotalrecord, pagebuf, sizeof(int));
			
			if(ptotalrecord >= ((HEADER_AREA_SIZE-sizeof(int))/(sizeof(int)*2))){
			//페이지에 저장할 수 있는 최대 레코드 수가 꽉찬 경우(새 페이지 생성)
				memset(pagebuf, 0, PAGE_SIZE);
				ptotalrecord = 1;
				offset = 0;
				pagenum++;
				
				metadata[0]++;
				metadata[1]++;
				
				memcpy(pagebuf, &ptotalrecord, sizeof(int));
				memcpy(pagebuf+sizeof(int), &offset, sizeof(int));
				memcpy(pagebuf+(sizeof(int)*2), &packlen, sizeof(int));
				memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, packlen);
				
				fseek(fp, 0, SEEK_SET);
				fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터
				writePage(fp, pagebuf, pagenum);//데이터 페이지
			}
			else{//페이지에 레코드 수가 꽉차지 않은 경우
				memcpy(&offset, pagebuf+sizeof(int)+((ptotalrecord-1)*sizeof(int)*2) , sizeof(int));
				memcpy(&length, pagebuf+(sizeof(int)*2)+((ptotalrecord-1)*sizeof(int)*2), sizeof(int));
				
				if(packlen > DATA_AREA_SIZE-(offset+length)){///////test해보기!!!!
				//데이터 영역이 모자른 경우(새 페이지 생성)
					ptotalrecord = 1;
					offset = 0;
					pagenum++;
					
					metadata[0]++;
					metadata[1]++;
					
					memcpy(pagebuf, &ptotalrecord, sizeof(int));
					memcpy(pagebuf+sizeof(int), &offset, sizeof(int));
					memcpy(pagebuf+(sizeof(int)*2), &packlen, sizeof(int));
					memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, packlen);
					
					fseek(fp, 0, SEEK_SET);
					fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터
					writePage(fp, pagebuf, pagenum);//데이터 페이지
				}
				else{//데이터 영역이 충분한 경우
					ptotalrecord++;
					offset = offset+length;
					
					metadata[1]++;
					
					memcpy(pagebuf, &ptotalrecord, sizeof(int));
					memcpy(pagebuf+sizeof(int)+(sizeof(int)*2*(ptotalrecord-1)), &offset, sizeof(int));
					memcpy(pagebuf+(sizeof(int)*2)+(sizeof(int)*2*(ptotalrecord-1)), &packlen, sizeof(int));
					memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, packlen);
					
					fseek(fp, 0, SEEK_SET);
					fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터
					writePage(fp, pagebuf, pagenum);//데이터 페이지	
				}
			}
		}
		
	}
	else{//삭제된 데이터가 있는 경우->right size 검사 수행(없다면 새 페이지 생성)
		count = 0;
		fseek(fp, 0, SEEK_SET);
		fread(metadata, sizeof(int)*4, 1, fp);
		prepage = metadata[2];
		prerecord = metadata[3];
		
		while(1){
			readPage(fp, pagebuf, prepage);
			memcpy(&offset, pagebuf+sizeof(int)+(sizeof(int)*2*prerecord), sizeof(int));
			memcpy(&length, pagebuf+(sizeof(int)*2)+(sizeof(int)*2*prerecord), sizeof(int));
		
			if(packlen <= length){//rigth size 적합
				if(count == 0){//맨처음 데이터 지우는 경우 헤더파일 수정
					memcpy(&newprepage, pagebuf+HEADER_AREA_SIZE+offset+sizeof(char), sizeof(int));
					memcpy(&newprerecord, pagebuf+HEADER_AREA_SIZE+offset+sizeof(char)+sizeof(int), sizeof(int));///메타데이터 업그레이드
					metadata[2] = newprepage;
					metadata[3] = newprerecord;
		
					memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, packlen);//데이터 갱신
					
					fseek(fp, 0, SEEK_SET);
					fwrite(metadata, sizeof(int)*4, 1, fp);
					writePage(fp, pagebuf, prepage);
				}
				else{//중간 삭제의 경우(수정함)
					memcpy(&newprepage, pagebuf+HEADER_AREA_SIZE+offset+sizeof(char), sizeof(int));
					memcpy(&newprerecord, pagebuf+HEADER_AREA_SIZE+offset+sizeof(char)+sizeof(int), sizeof(int));
					
					memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, packlen);//데이터 갱신
					writePage(fp, pagebuf, prepage);
					
					readPage(fp, pagebuf, changepage);
					memcpy(pagebuf+HEADER_AREA_SIZE+preoffset+sizeof(char), &newprepage, sizeof(int));
					memcpy(pagebuf+HEADER_AREA_SIZE+preoffset+sizeof(char)+sizeof(int), &newprerecord, sizeof(int));
					writePage(fp, pagebuf, changepage);
					
				}
				
				free(recordbuf);
				free(pagebuf);
				free(metadata);
				
				return;
			}
			else{
				changepage = prepage;
				preoffset = offset;
				
				memcpy(&prepage, pagebuf+HEADER_AREA_SIZE+offset+sizeof(char), sizeof(int));
				memcpy(&prerecord, pagebuf+HEADER_AREA_SIZE+offset+sizeof(char)+sizeof(int), sizeof(int));
				count++;
				
				if(prepage == -1 && prerecord == -1)
					break;
			}
		}
			
			if(prepage == -1 && prerecord == -1 ){//right size 없음->새 페이지 생성
				pagenum = metadata[0]-1;
				readPage(fp, pagebuf, pagenum);
				memcpy(&ptotalrecord, pagebuf, sizeof(int));
				
				if(ptotalrecord >= ((HEADER_AREA_SIZE-sizeof(int))/(sizeof(int)*2))){
				//페이지에 저장할 수 있는 최대 레코드 수가 꽉찬 경우(새 페이지 생성)
					memset(pagebuf, 0, PAGE_SIZE);
					ptotalrecord = 1;
					offset = 0;
					pagenum++;
					
					metadata[0]++;
					metadata[1]++;
					
					memcpy(pagebuf, &ptotalrecord, sizeof(int));
					memcpy(pagebuf+sizeof(int), &offset, sizeof(int));
					memcpy(pagebuf+(sizeof(int)*2), &packlen, sizeof(int));
					memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, packlen);
					
					fseek(fp, 0, SEEK_SET);
					fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터
					writePage(fp, pagebuf, pagenum);//데이터 페이지
				}
				else{//페이지에 레코드 수가 꽉차지 않은 경우
					memcpy(&offset, pagebuf+sizeof(int)+((ptotalrecord-1)*sizeof(int)*2) , sizeof(int));
					memcpy(&length, pagebuf+(sizeof(int)*2)+((ptotalrecord-1)*sizeof(int)*2), sizeof(int));
					
					if(packlen > DATA_AREA_SIZE-(offset+length)){
					//데이터 영역이 모자른 경우(새 페이지 생성)
						ptotalrecord = 1;
						offset = 0;
						pagenum++;
						
						metadata[0]++;
						metadata[1]++;
						
						memcpy(pagebuf, &ptotalrecord, sizeof(int));
						memcpy(pagebuf+sizeof(int), &offset, sizeof(int));
						memcpy(pagebuf+(sizeof(int)*2), &packlen, sizeof(int));
						memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, packlen);
						
						fseek(fp, 0, SEEK_SET);
						fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터
						writePage(fp, pagebuf, pagenum);//데이터 페이지
					}
					else{//데이터 영역이 충분한 경우
						ptotalrecord++;
						offset = offset+length;
						
						metadata[1]++;
						
						memcpy(pagebuf, &ptotalrecord, sizeof(int));
						memcpy(pagebuf+sizeof(int)+(sizeof(int)*2*(ptotalrecord-1)), &offset, sizeof(int));
						memcpy(pagebuf+(sizeof(int)*2)+(sizeof(int)*2*(ptotalrecord-1)), &packlen, sizeof(int));
						memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, packlen);
						
						fseek(fp, 0, SEEK_SET);
						fwrite(metadata, sizeof(int)*4, 1, fp);//헤더 데이터
						writePage(fp, pagebuf, pagenum);//데이터 페이지	
					}
				}
		}
		
	}
	
	free(recordbuf);
	free(pagebuf);
	free(metadata);
	
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	Person *p;
	char *recordbuf;
	char *pagebuf;
	int *metadata;
	int packlen;//packing한 레코드 사이즈
	int ptotalrecord = 0;//페이지에 저장된 record 수
	int pagenum;//페이지 넘버
	int offset;
	int length;
	int totalpage;//전체 datapage 수
	int prepage;
	int prerecord;
	char deletemark = '*';
	
	recordbuf = (char *)malloc(sizeof(char) * MAX_RECORD_SIZE);
	pagebuf = (char *)malloc(PAGE_SIZE);
	metadata = (int *)malloc(sizeof(int) * 4);
	p = (Person *)malloc(sizeof(Person));
	
	memset(p, 0, sizeof(Person));
	memset(recordbuf, 0, sizeof(recordbuf));
	memset(pagebuf, 0, PAGE_SIZE);
	memset(metadata, 0, sizeof(metadata));
	
	fseek(fp, 0, SEEK_SET);
	fread(metadata, sizeof(int)*4, 1, fp);
	totalpage = metadata[0];
	
	if(metadata[2] == -1 && metadata[3] == -1){
		prepage = -1;
		prerecord = -1;
	}
	else{
		prepage = metadata[2];
		prerecord = metadata[3];
	}
	
	for(int pn = 0 ; pn < totalpage; pn++){
		readPage(fp, pagebuf, pn);
		
		memcpy(&ptotalrecord, pagebuf, sizeof(int));
		
		for(int r = 0; r < ptotalrecord; r++){
			memcpy(&offset, pagebuf+sizeof(int)+(sizeof(int)*2*r), sizeof(int));
			memcpy(&length, pagebuf+(sizeof(int)*2)+(sizeof(int)*2*r), sizeof(int));
			
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, length);
			unpack(recordbuf, p);
			
			if((p->id[0] != '*') && (!strcmp(p->id, id))){//삭제 해야하는 데이터인 경우
				memcpy(pagebuf+HEADER_AREA_SIZE+offset, &deletemark, sizeof(char));
				memcpy(pagebuf+HEADER_AREA_SIZE+offset+sizeof(char), &prepage, sizeof(int));
				memcpy(pagebuf+HEADER_AREA_SIZE+offset+sizeof(char)+sizeof(int), &prerecord, sizeof(int));
				
				metadata[2] = pn;
				metadata[3] = r;
				
				fseek(fp, 0, SEEK_SET);
				fwrite(metadata, sizeof(int)*4, 1, fp);
				writePage(fp, pagebuf, pn);
				
				free(recordbuf);
				free(pagebuf);
				free(metadata);
				free(p);
				return;
			}
		}
	}
	
	free(recordbuf);
	free(pagebuf);
	free(metadata);
	free(p);
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	Person *p;
	int metadata[4]={0,0,-1,-1};
	
	p = (Person *)malloc(sizeof(Person));
	memset(p, 0, sizeof(Person));
	
	fp = fopen(argv[2], "r+");
	if(fp == NULL){//레코드 파일이 없는 경우
		if((fp = fopen(argv[2], "w+")) == NULL){//레코드 파일 생성
			fprintf(stderr, "file open error for %s\n", argv[2]);
			exit(1);
		}
		else{//레코드 파일 생성과 동시에 공간을 할당 받고, 메타데이터 값을 초기화
			fwrite(metadata, sizeof(int)*4, 1, fp);
		}
	}
	
	if(!strcmp(argv[1], "a")){//레코드 삽입하는 경우
		if(argc != 9){
			fprintf(stderr, "usage : %s a filename \"id\" \"name\" \"age\" \"addr\" \"phone\" \"email\"\n", argv[0]);
			exit(1);
		}
		if(strlen(argv[3]) > 14){
			fprintf(stderr, "id max_length exceeded\n");
			exit(1);
		}
		if(strlen(argv[4]) > 18){
			fprintf(stderr, "name max_length exceeded\n");
			exit(1);
		}
		if(strlen(argv[5]) > 4){
			fprintf(stderr, "age max_length exceeded\n");
			exit(1);
		}
		if(strlen(argv[6]) > 22){
			fprintf(stderr, "addr max_length exceeded\n");
			exit(1);
		}
		if(strlen(argv[7]) > 16){
			fprintf(stderr, "phone max_length exceeded\n");
			exit(1);
		}
		if(strlen(argv[8]) > 26){
			fprintf(stderr, "email max_length exceeded\n");
			exit(1);
		}

		strcpy(p->id, argv[3]);
		strcpy(p->name, argv[4]);
		strcpy(p->age, argv[5]);
		strcpy(p->addr, argv[6]);
		strcpy(p->phone, argv[7]);
		strcpy(p->email, argv[8]);
		
		add(fp, p);
	
	}
	else if(!strcmp(argv[1], "d")){//레코드 삭제 하는 경우
		if(argc != 4){
			fprintf(stderr, "usage : %s d filename \"id\" \n", argv[0]);
			exit(1);
		}
		if(strlen(argv[3]) > 14){
			fprintf(stderr, "id max_length exceeded\n");
			exit(1);
		}
	
		delete(fp, argv[3]);
		
	}
	else{
		fprintf(stderr, "option error(add => a, delete => d)\n");
		exit(1);
	}
	
	free(p);
	fclose(fp);
	return 1;
}
