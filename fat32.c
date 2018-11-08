/*Name = Aayush Karki Shaili Balampaki
 Student id = 1001365863  1001428149
 project = 3
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_COMMAND_SIZE 100


FILE *fp;

#define WHITESPACE " \t\n"
#define MAX_NUM_ARGUMENTS 10
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

#define BPB_RootClus_Offset 44
#define BPB_RootClus_Size 4

struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;

};

struct DirectoryEntry dir[16];

uint16_t BPB_BytsPerSec;
uint8_t BPB_SecPerClus;
uint16_t BPB_RsvdSecCnt;
uint8_t BPB_NumFATs;
uint32_t BPB_FATSz32;
uint16_t BPB_RootEntCnt;
uint32_t RootClusAddress;
uint32_t BPB_RootClus;

char BS_Vollab[11];


int16_t nextLb (uint32_t sector)
{
  uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + sector*4;
  int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val, 2,1,fp);
  return val;
}

//returns the address of the given sector
int LBAToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

//This function returns whether there is the file of the given filename in the given address
//It takes name of the file and current address as the parameter
//returns position of the file if there is file
//returns -1 if there is no file of the given filename
int filenametoPosition(char* name[], uint32_t currentAddress){
  fseek(fp, currentAddress, SEEK_SET);
  int i;
  for(i = 0; i < 16; i++){
    fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
  }
  char expanded_name[12];
  memset( expanded_name, ' ', 12 );
  char *token = strtok( name[1], "." );
  strncpy( expanded_name, token, strlen( token ) );
  token = strtok( NULL, "." );
  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }
  expanded_name[11] = '\0';
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }
  i = 0;
  while(1){
  if( strncmp( expanded_name, dir[i].DIR_Name, 11 ) == 0 )
  {
    return i;
  }
  i++;
  if ( i == 16){  //cause there are only 16 entries
    break;
  }
}
return -1;
}


int main()
{
  int dataSecNum;
  fp = fopen("fat32.img", "r+");
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
//BPB_RootClus
fseek(fp, BPB_RootClus_Offset, SEEK_SET);
fread(&BPB_RootClus, BPB_RootClus_Size,1,fp);
//Calculating the address of the root directory
RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
//going to that address
fseek(fp, RootClusAddress, SEEK_SET);
int i;
for(i = 0; i < 16; i++){
  fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
}

  char * cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
  uint32_t pointCurrentAddress = RootClusAddress;
    while(1){
   printf("mfs> ");
   while(!fgets(cmd_str,MAX_COMMAND_SIZE, stdin));
   int token_count = 0;
   char * token[MAX_NUM_ARGUMENTS];
   char *arg_str;
   char *working_str = strdup(cmd_str);
   char *working_root = working_str;

   //this will parse the user command string and store into token array
   while(((arg_str = strsep(&working_str, WHITESPACE)) != NULL) && (token_count < MAX_NUM_ARGUMENTS) )
   {
     token[token_count] = strndup(arg_str, MAX_COMMAND_SIZE);
     if(strlen(token[token_count]) == 0){
       token[token_count] = NULL;
     }
     token_count++;
   }

   //we can exit the prompt by entering exit
   if (strcmp(token[0],"exit")  == 0) exit(0);

   //here to open the file, first of all we will get the position of file and go to that address using fseek
   if(strcmp(token[0], "open") == 0)
   {
    int addres_given_sec = filenametoPosition(token, pointCurrentAddress);
    if (addres_given_sec!=-1)
    {
    uint32_t cluster_number=dir[addres_given_sec].DIR_FirstClusterLow;
    pointCurrentAddress=LBAToOffset( cluster_number);
    fseek(fp, pointCurrentAddress, SEEK_SET);
   }
  }

    //to close the file we will point our pointCurrentAddress to the RootClusAddress
   if(strcmp(token[0],"close")==0)
   {
      if(pointCurrentAddress==RootClusAddress)
      {
        printf("Error:File system not open.\n");
      }
      else
      {
        pointCurrentAddress=RootClusAddress;
         fseek(fp, pointCurrentAddress, SEEK_SET);
    }
   }

   //It gives us the information of fat32 image
   if(strcmp(token[0],"info")== 0){
     printf("BPB_BytsPerSec == %-10d   %-13x\n", BPB_BytsPerSec, BPB_BytsPerSec );
     printf("BPB_SecPerClus == %-10d   %-13x\n", BPB_SecPerClus, BPB_SecPerClus );
     printf("BPB_RsvdSecCnt == %-10d   %-13x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
     printf("BPB_NumFATs    == %-10d   %-13x\n", BPB_NumFATs, BPB_NumFATs);
     printf("BPB_FATSz32    == %-10d   %-13x\n", BPB_FATSz32, BPB_FATSz32);
   }

   //this will give us statistics of the given file name
   if (strcmp(token[0], "stat") == 0){
     int val = filenametoPosition(token, pointCurrentAddress);
     if(val != -1){
     printf("File or directory name      %-6s\n",dir[val].DIR_Name);
     printf("SIze                        %-6d\n", dir[val].DIR_FileSize);
     printf("Atrribute                   %-6x\n", dir[val].DIR_Attr);
     printf("low bit Cluster address     %-6d\n", dir[val].DIR_FirstClusterLow);
     printf("high bit of cluster address %-6d\n", dir[val].DIR_FirstClusterHigh);
   }else{
     printf("Error: File not FOund\n");
   }
   }

   //will list the files in the directory
   if(strcmp(token[0], "ls") == 0){
     fseek(fp, pointCurrentAddress, SEEK_SET);
     for(i = 0; i < 16; i++){
       fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
     }
     if (token[1] == NULL){
       token[1] = ".";
     }
     if( (strcmp(token[1], ".") == 0)){
     for(i = 0; i < 16; i++){
       if(dir[i].DIR_Attr != 0x0f ){
         printf("%s\n", dir[i].DIR_Name);
       }
     }
   }else {
     uint32_t current_address = pointCurrentAddress;
     if (strcmp(token[1], "..") == 0){
     pointCurrentAddress = RootClusAddress;
     fseek(fp, pointCurrentAddress, SEEK_SET);
     for(i = 0; i < 16; i++){
       fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
     }
       for(i = 0; i < 16; i++){
         if(dir[i].DIR_Attr != 0x0f ){
           printf("%s\n", dir[i].DIR_Name);
         }
       }
     }
     pointCurrentAddress = current_address;
   }
 }

 //this will only support absolute paths
   if (strcmp(token[0], "cd") == 0){
     if(token[1] != NULL){
   int position_file = filenametoPosition(token, pointCurrentAddress);
   uint32_t fileaddress = LBAToOffset(dir[position_file].DIR_FirstClusterLow);
   pointCurrentAddress = fileaddress;
 }else{
   if (pointCurrentAddress == RootClusAddress){
     pointCurrentAddress = RootClusAddress;
   }else{
     pointCurrentAddress = RootClusAddress;
   }
 }
}

  //will print the volume
   if(strcmp(token[0], "volume") == 0){
     if(BS_Vollab == NULL){
       printf("Error: Volume name not found");
     }else{
    printf("%s\n", BS_Vollab);
   }
  }


   if(strcmp(token[0], "read") == 0){
     int position_file = filenametoPosition(token, pointCurrentAddress);
     uint32_t address_file = LBAToOffset(dir[position_file].DIR_FirstClusterLow);
     int position_to_read = atoi(token[2]);
     int num_bytes = atoi(token[3]);
     char array[100];
     fseek(fp, address_file + position_to_read, SEEK_SET);
     for(i = 0; i < num_bytes; i++){
       fread(&array[i], sizeof(char),1,fp);
     }
     for(i = 0; i < num_bytes; i++){
       printf("%d\n", array[i]);
     }
   }

   if(strcmp(token[0], "get") == 0){
     //returns the position of file in directory
     int position_file = filenametoPosition(token, RootClusAddress);
     //if it return -1, file not there
     if(position_file == -1){
       printf("Error:File not Found\n");
     }
     else
     {
     //pointCurrentAddress is pointing to the address of the directory or sub directory we are on righ now
     fseek(fp, pointCurrentAddress, SEEK_SET);
     fwrite(&dir[position_file],sizeof(struct DirectoryEntry),1, fp);
   }
 }
}


return 0;
}
