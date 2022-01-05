/*주의사항
1. blockmap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
2. blockmap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(blockmap.h에 추가하면 안됨)*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "blockmap.h"
/* 필요한 경우 헤더 파일을 추가하시오.*/
int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);
void ftl_print();

int mappingTable[BLOCKS_PER_DEVICE];//mapping table
int freeBlock;

/* flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
file system에 의해 반드시 먼저 호출이 되어야 한다.*/

void ftl_open()
{	int num, ppn = 0, ii, jj;
	char pagebuf[PAGE_SIZE];
	/* address mapping table 초기화 또는 복구
	free block's pbn 초기화
    	address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일 */
    	memset(pagebuf, 0xff, PAGE_SIZE);
    	
    	for(int i = 0; i < BLOCKS_PER_DEVICE; i++){
    		mappingTable[i] = -1;
    	}
    	
    	for(int j = 0; j < BLOCKS_PER_DEVICE; j++){
    		ppn = PAGES_PER_BLOCK * j;
    		dd_read(ppn, pagebuf);
    		
    		memcpy(&num, pagebuf + SECTOR_SIZE, sizeof(int));
    		
    		if(num >= 0){
    			mappingTable[num] = j;
    		}
    	}
    	
    	for(ii = DATABLKS_PER_DEVICE; ii >= 0; ii--){
    		for(jj = 0; jj < BLOCKS_PER_DEVICE; jj++){
    			if(mappingTable[jj] == ii)
    				break;
    		}
    		
    		if(jj ==BLOCKS_PER_DEVICE){
    			freeBlock = ii;
    			break;
    		}
    	}
    		
    	mappingTable[DATABLKS_PER_DEVICE] = freeBlock;
    			
	return;
}

/* 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
(즉, 이 함수에서 메모리를 할당 받으면 안됨) */

void ftl_read(int lsn, char *sectorbuf)
{
	int lbn, offset, pbn, ppn;
	char pagebuf[PAGE_SIZE];
	
	lbn = lsn / PAGES_PER_BLOCK;
	offset = lsn % PAGES_PER_BLOCK;
	pbn = mappingTable[lbn];
	ppn = pbn * PAGES_PER_BLOCK + offset;
	
	memset(pagebuf, 0xff, PAGE_SIZE);
	
	if((dd_read(ppn, pagebuf)) == -1){
		fprintf(stderr, "ftl_read() > dd_read() error\n");
		exit(1);
	}
	
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	
	return;
}

/* 이 함수를 호출하는 쪽(file system)에서 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 함
(즉, 이 함수에서 메모리를 할당 받으면 안됨) */

void ftl_write(int lsn, char *sectorbuf)
{
	int lbn, offset, pbn, ppn, check, orippn, chpbn, chlbn;
	char pagebuf[PAGE_SIZE], tmpbuf[PAGE_SIZE], sparebuf[SPARE_SIZE], sparetmp[SPARE_SIZE];
	int setpbn;
	
	memset(pagebuf, 0xff, PAGE_SIZE);
	memset(tmpbuf, 0xff, PAGE_SIZE);
	memset(tmpbuf, 0xff, SPARE_SIZE);
	memset(sparebuf, 0xff, SPARE_SIZE);
	
	lbn = lsn / PAGES_PER_BLOCK;
	offset = lsn % PAGES_PER_BLOCK;
	pbn = mappingTable[lbn];
	ppn = pbn * PAGES_PER_BLOCK + offset;
	
	if(pbn < 0){//최초로 쓰는 경우
		int ok = 1,i;
		for(setpbn = DATABLKS_PER_DEVICE; setpbn >= 0; setpbn--){
			for(i = 0; i < BLOCKS_PER_DEVICE; i++){
				if(mappingTable[i] == setpbn)
					break;
			}
			if(i == BLOCKS_PER_DEVICE){///////수정했음
					mappingTable[lbn] = setpbn;
					break;
			}
		}
		
		chpbn= mappingTable[lbn];
		
		
		memcpy(tmpbuf + SECTOR_SIZE, &lbn, sizeof(int));
		
		if(offset != 0){
			dd_write(chpbn * PAGES_PER_BLOCK, tmpbuf);//첫페이지에 lbn저장
		}
	}
	else{//최초로 쓰지 않는 경우(갱신하는 경우와 갱신 아닌 경우 구별)
		dd_read(ppn, tmpbuf);
		memcpy(&check, tmpbuf + SECTOR_SIZE + sizeof(int), sizeof(int));
		
		if(check >= 0){//갱신하는 경우
			chpbn = mappingTable[DATABLKS_PER_DEVICE];
			for(int i = 0; i<PAGES_PER_BLOCK; i++){//블럭을 프리블럭으로 복사
				memset(tmpbuf, 0xff, PAGE_SIZE);
				memset(tmpbuf, 0xff, SPARE_SIZE);
	
				orippn = pbn * PAGES_PER_BLOCK + i;
				
				if(ppn != orippn){
					dd_read(orippn, tmpbuf);
					memcpy(&check, tmpbuf + SECTOR_SIZE + 4, sizeof(int));
					
					
					if(check >= 0){
						memcpy(tmpbuf + SECTOR_SIZE, &lbn, sizeof(int));
						dd_write(chpbn * PAGES_PER_BLOCK + i, tmpbuf);
					}
					else if(check < 0 && i == 0){
						memcpy(tmpbuf + SECTOR_SIZE, &lbn, sizeof(int));
						dd_write(chpbn * PAGES_PER_BLOCK, tmpbuf);
					}
				}
			}
			
			if(dd_erase(pbn) == -1){
				fprintf(stderr, "dd_erase() error\n");
				exit(1);
			}
			
			mappingTable[DATABLKS_PER_DEVICE] = mappingTable[lbn];
			mappingTable[lbn] = chpbn;
			freeBlock = mappingTable[DATABLKS_PER_DEVICE];
			}
	}
	
	memcpy(pagebuf,sectorbuf,SECTOR_SIZE);
	memcpy(sparebuf, &lbn, sizeof(int));
	memcpy(sparebuf + sizeof(int), &lsn, sizeof(int));
	memcpy(pagebuf + SECTOR_SIZE, sparebuf, SPARE_SIZE);
	
	dd_write(chpbn * PAGES_PER_BLOCK + offset, pagebuf);
	
	return;
}

void ftl_print()
{
	printf("lbn pbn\n");
	for(int i =0; i < DATABLKS_PER_DEVICE; i++){
		printf("%d %d\n",i, mappingTable[i]);
	}
	
	printf("free block's pbn=%d\n",mappingTable[DATABLKS_PER_DEVICE]);
	
	return;
}
