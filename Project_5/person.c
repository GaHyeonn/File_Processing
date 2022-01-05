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
int found = 0;
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

}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	memset(p, 0, sizeof(Person));
	sscanf(recordbuf, "%[^'#']#%[^'#']#%[^'#']#%[^'#']#%[^'#']#%[^'#']#", p->id, p->name, p->age, p->addr, p->phone, p->email);
}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{

}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{

}

//
// 주어진 레코드 파일(recordfp)을 이용하여 심플 인덱스 파일(idxfp)을 생성한다.
//
void createIndex(FILE *idxfp, FILE *recordfp)
{
	Person p;
	int totalPageNum; //레코드 파일의 전체 페이지 수
	int pageRecordNum; //해당 페이지에 저장된 레코드 수
	int recordOffset; //해당 레코드의 offset
	int recordLength; //해당 레코드의 length
	char *pagebuf; //페이지 버퍼
	char *recordbuf; //레코드 버퍼
	char *indexRecord; //인덱스 파일에 저장될 레코드
	int totalIndexNum = 0; //인덱스 파일에 저장되어 있는 레코드 개수
	char **index;
	long preID;
	long nextID;
	int pagenum;
	int recordnum;
	char *tempbuf;
	char *ID;
	int count = 0;
	int totalrecordNum;
	
	recordbuf = (char *)malloc(MAX_RECORD_SIZE);
	pagebuf = (char *)malloc(PAGE_SIZE);
	indexRecord = (char *)malloc(sizeof(char)*21);
	tempbuf = (char *)malloc(sizeof(char)*21);
	ID = (char *)malloc(sizeof(char)*14);
	
	memset(tempbuf, '\0', sizeof(char)*21);
	
	fread(&totalPageNum, sizeof(int), 1, recordfp);//레코드 파일의 전체 페이지 수 읽음
	fread(&totalrecordNum, sizeof(int), 1, recordfp);//레코드 파일에 존재하는 모든 레코드 수(삭제 레코드 포함)
	
	index = (char **)malloc(sizeof(char *)*totalrecordNum);
	for(int i = 0; i<totalrecordNum; i++){
		index[i] = (char *)malloc(sizeof(char)*21);
		memset(index[i], '\0', sizeof(char)*21);
	}
	
	for(pagenum = 0; pagenum<totalPageNum; pagenum++){
		readPage(recordfp, pagebuf, pagenum);
		memcpy(&pageRecordNum, pagebuf, sizeof(int));
		
		for(recordnum = 0; recordnum<pageRecordNum; recordnum++){//데이터 페이지에서 레코드 읽어옴
			memcpy(&recordOffset, pagebuf+sizeof(int)+(sizeof(int)*2*recordnum), sizeof(int));
			memcpy(&recordLength, pagebuf+sizeof(int)*2+(sizeof(int)*2*recordnum), sizeof(int));
			
			memset(recordbuf, '\0', MAX_RECORD_SIZE);
			
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+recordOffset, recordLength); //해당 레코드 읽어옴
			memset(&p, 0, sizeof(Person));
			unpack(recordbuf, &p);
			
			if(p.id[0] == '*'){//읽어온 레코드가 삭제된 경우
				continue;
			}
			else{
			memset(indexRecord, 0, sizeof(char)*21);
			memcpy(indexRecord, p.id, strlen(p.id));
			memcpy(indexRecord+13, &pagenum, sizeof(int));//페이지 번호 저장
			memcpy(indexRecord+13+sizeof(int), &recordnum, sizeof(int));//페이지 내 레코드 번호 저장
			
			memcpy(index[count++], indexRecord, sizeof(char)*21);///////수정 필요할 수도
			totalIndexNum++;
			}
		}
		
	}
	
	fseek(idxfp, 0, SEEK_SET);
	fwrite(&totalIndexNum, sizeof(int), 1, idxfp);//인덱스 파일에 저장되어 있는 레코드 개수 씀
	
	for(int i = 0; i<totalIndexNum; i++) {//오름차순 정렬
		for(int j = 0; j<totalIndexNum-i-1; j++) {
			memset(ID, '\0', sizeof(char)*14);
			memcpy(ID, index[j], sizeof(char)*13);
			preID = atol(ID);
			
			memset(ID, '\0', sizeof(char)*14);
			memcpy(ID, index[j+1], sizeof(char)*13);
			nextID = atol(ID);
			
			if(preID > nextID){            
				memcpy(tempbuf, index[j], sizeof(char)*21);
                		memcpy(index[j], index[j+1], sizeof(char)*21);
                		memcpy(index[j+1], tempbuf, sizeof(char)*21); 
                	}
		}
	} 
	
	fseek(idxfp, 4, SEEK_SET);
	for(int i = 0; i<totalIndexNum; i++){
		fwrite(index[i], sizeof(char)*21, 1, idxfp);
	}
	
	free(recordbuf);
	free(pagebuf);
	free(indexRecord);
	free(tempbuf);
	free(ID);
	for(int i = 0; i<totalrecordNum; i++){
		free(index[i]);
	}
	free(index);
	
	fclose(idxfp);
	fclose(recordfp);
	
}

//
// 주어진 심플 인덱스 파일(idxfp)을 이용하여 주민번호 키값과 일치하는 레코드의 주소, 즉 페이지 번호와 레코드 번호를 찾는다.
// 이때, 반드시 이진 검색 알고리즘을 사용하여야 한다.
//
void binarysearch(FILE *idxfp, const char *id, int *pageNum, int *recordNum)
{
	int totalrecord;
	int low , high, middle;
	long searchid, seekid;
	char *indexbuf;
	char *seekcharId;
	int totalSeek = 0;
	
	indexbuf = (char *)malloc(sizeof(char)*21);
	seekcharId = (char *)malloc(sizeof(char)*13);
	memset(indexbuf, 0, sizeof(indexbuf));
	memset(seekcharId, 0, sizeof(seekcharId));
	
	searchid = atol(id);
	
	fread(&totalrecord, sizeof(int), 1, idxfp);
	
	low = 0;
	high = totalrecord-1;
	
	while(low <= high){
		middle = (low+high)/2;
		
		fseek(idxfp, sizeof(int)+sizeof(char)*21*middle, SEEK_SET);
		fread(indexbuf, sizeof(char)*21, 1, idxfp);
		totalSeek++;
		
		memcpy(seekcharId, indexbuf, sizeof(char)*13);
		seekid = atol(seekcharId);
		
		if(searchid < seekid){
			high = middle-1;
		}
		else if(searchid > seekid){
			low = middle+1;
		}
		else if(searchid == seekid){
			memcpy(pageNum, indexbuf+sizeof(char)*13, sizeof(int));
			memcpy(recordNum, indexbuf+sizeof(char)*13+sizeof(int), sizeof(int));
			found = 1;
			break;
		}
		
	}
	printf("#reads:%d\n", totalSeek);
	
	free(indexbuf);
	free(seekcharId);
}

int main(int argc, char *argv[])
{
	FILE *idxfp;  // 인덱스 파일의 파일 포인터
	FILE *recordfp; //레코드 파일의 파일 포인터
	char *id;
	int pageNum;
	int recordNum;
	char *pagebuf;
	Person p;
	int offset, length;
	char *recordbuf;
	
	if(!strcmp(argv[1], "i")){//인덱스 파일 생성
		if(argc != 4){
			fprintf(stderr, "usage : %s i <record file name> <indexfile name>\n", argv[0]);
			exit(1);
		}
		
		recordfp = fopen(argv[2], "r");
		idxfp = fopen(argv[3], "w");
		
		createIndex(idxfp, recordfp);
	}
	else if(!strcmp(argv[1], "b")){//이진 검색 수행
		if(argc != 5){
			fprintf(stderr, "usage : %s b <record file name> <indexfile name> <key value>\n", argv[0]);
			exit(1);
		}
		
		pagebuf = (char *)malloc(PAGE_SIZE);
		recordbuf = (char *)malloc(MAX_RECORD_SIZE);
		
		memset(pagebuf, 0, PAGE_SIZE);
		memset(recordbuf, 0, MAX_RECORD_SIZE);
		memset(&p, 0, sizeof(Person));
		
		recordfp = fopen(argv[2], "r");
		idxfp = fopen(argv[3], "r");
		
		binarysearch(idxfp, argv[4], &pageNum, &recordNum);
		
		if(found == 0){
			printf("no persons\n");
		}
		else{
			readPage(recordfp, pagebuf, pageNum);
			memcpy(&offset, pagebuf+sizeof(int)+sizeof(int)*2*recordNum, sizeof(int));
			memcpy(&length, pagebuf+sizeof(int)*2+sizeof(int)*2*recordNum, sizeof(int));
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, length);
			
			unpack(recordbuf, &p);
			
			printf("id=%s\n", p.id);
			printf("name=%s\n", p.name);
			printf("age=%s\n", p.age);
			printf("addr=%s\n", p.addr);
			printf("phone=%s\n", p.phone);
			printf("email=%s\n", p.email);
		}
		
		free(pagebuf);
		free(recordbuf);
		fclose(recordfp);
		fclose(idxfp);
		
	}
	else{
		fprintf(stderr, "Option error : i(인덱스 파일 생성), b(binary search)\n");
		exit(1);
	}

	return 0;
}
