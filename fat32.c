#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>


FILE *fp;

#define BPB_BytsPerSec_Offset 11
#define BPB_BytsPerSec_Size 2

#define BPB_SecPerClus_Offset 13
#define BPB_SecPerClus_Size 1

#define BPB_RsvdSecCnt_Offset 24
#define BPB_RsvdSecCnt_Size 2

#define BPB_NumFATs_Offset 16
#define BPB_NumFATs_Size 1

#define BPB_RootEntCnt_Offset 17
#define BPB_RootEntCnt_Size 2

#define BPB_FATSz32_Offset 36
#define BPB_Fatsz32_Size 4

#define BS_Vollab_Offset 71
#define BS_Vollab_Size 11

struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t Dir_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused[4];
};

struct DirectoryEntry dir[16];

uint16_t BPB_BytsPerSec;
uint8_t BPB_SecPerClus;
uint16_t BPB_RsvdSecCnt;
uint8_t BPB_NumFATs;
uint32_t BPB_FATSz32;
uint16_t BPB_RootEntCnt;
uint32_t RootClusAddress;
char BS_Vollab[11];


int16_t nextLb (uint32_t sector)
{
  uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + sector*4;
  int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val, 2,1,fp);
  return val;
}

int LBAToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}


int main()
{
  fp = fopen("fat32.img", "r");
  if(fp == NULL)
  {
    perror("Error opening file : ");
  }
//BPB_BytsPerSec
fseek(fp, BPB_BytsPerSec_Offset, SEEK_SET);
fread(&BPB_BytsPerSec, BPB_BytsPerSec_Size, 1, fp);
//BPB_SecPerClus
fseek(fp,BPB_SecPerClus_Offset, SEEK_SET);
fread(&BPB_SecPerClus, BPB_SecPerClus_Size, 1 , fp);
//BPB_RsvdSecCnt
fseek(fp, BPB_RsvdSecCnt_Offset, SEEK_SET);
fread(&BPB_RsvdSecCnt, BPB_RsvdSecCnt_Size, 1, fp);
//BPB_NumFATs
fseek(fp, BPB_NumFATs_Offset, SEEK_SET);
fread(&BPB_NumFATs, BPB_NumFATs_Size, 1, fp);
//BPB_RootEntCnt
fseek(fp, BPB_RootEntCnt_Offset, SEEK_SET);
fread(&BPB_RootEntCnt, BPB_RootEntCnt_Size, 1, fp);
//BPB_FATSz32
fseek(fp, BPB_FATSz32_Offset, SEEK_SET);
fread(&BPB_FATSz32, BPB_Fatsz32_Size, 1, fp);
//BS_VolLab
fseek(fp, BS_Vollab_Offset, SEEK_SET);
fread(&BS_Vollab, BS_Vollab_Size, 1, fp);

//Calculating the address of the root directory
RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
printf("Root cluster Address %x\n", RootClusAddress);
fseek(fp, RootClusAddress, SEEK_SET);
int i;
for(i = 0; i < 16; i++){
  fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
}

for(i = 0; i < 16; i++){
  printf("%s\n", dir[i].DIR_Name);
}

return 0;
}
