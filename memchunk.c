#include<stdio.h>
#include<netinet/in.h>
#include<stdlib.h>

size_t map[4*1024*1024];

#define nearest8(x) (((((x)-1)>>3)<<3)+8) //nearest greatest multiple of 8

void *start=NULL;
void *top=NULL;
void *end=NULL;
size_t offset=0;// also gives total chunks size allocated
size_t chunk_count=0;
size_t chunk_free_count=0;
size_t total_chunk_free_size=0;
size_t endian=0;

typedef struct _chunk chunk;
struct _chunk {
  size_t size:29;
  size_t s1:1;
  size_t s2:1;
  size_t prev_inuse:1;
  char data[1];
};
void HANDLE_ENDIAN(void *addr) {
  if(endian) {
  unsigned int value=*(int*)addr;
  *(int*)addr=htonl(value);
  }
}
void *c_malloc(size_t size) {
  size=nearest8(size);
  map[size]++;
  chunk *c=(chunk *)top;
  c->size=size;
  c->s1=0;
  c->s2=0;
  c->prev_inuse=1;
  top=(char*)top+size+4; //top pointing to next free chunk
  ((chunk *)top)->prev_inuse=1;
  return (void*)c->data;
}
void chunk_details() {
  printf("\nOffset: Size(Bytes): Prev_Chunk_inUse\n");
  printf("=====================================\n");
  chunk *next=(chunk*)start;
  HANDLE_ENDIAN(next);
  do {
    printf("%6d: %11d: %16d\n",offset,next->size,next->prev_inuse);
    offset+=next->size;
    next=(chunk*)((char*)next+next->size+4);
    HANDLE_ENDIAN(next);
    chunk_count++;
  }while(next!=top);
}
void c_free(size_t size) {
  size=nearest8(size);
  map[size]--;
  chunk *next=(chunk*)start;
  HANDLE_ENDIAN(next);
  do{
    chunk *temp=(chunk*)((char*)next+next->size+4);
    if(next->size==size && temp->prev_inuse!=0) {
      total_chunk_free_size+=next->size;
      chunk_free_count++;
    }
    if(next->size==size && (temp!=top) && temp->prev_inuse!=0) {
      temp->prev_inuse=0;
      break;
    }
    next=(chunk *)((char*)next+next->size+4);
    HANDLE_ENDIAN(next);
  }while(next!=top);
}
void disp_used_free() {
  printf("\nTotal_Mem_Chunks_Used  : Total_Freed_Chunks\n");
  printf("===========================================\n");
  printf("%22d : %18d\n",offset,total_chunk_free_size);
  printf("\nTotal_inUse_Chunks_Mem : Total_inUse_Chunks\n");
  printf("===========================================\n");
  size_t total_inUse_chunks_mem=offset-total_chunk_free_size;
  size_t total_inUse_chunks=chunk_count-chunk_free_count;
  printf("%22d : %18d\n",total_inUse_chunks_mem,total_inUse_chunks);
  printf("\nChunks_with_same_Memory\n");
  printf("=========================\n");
  size_t t;
  for(t=0;t<4*1024*1024;t++) {
    if(map[t]!=0) {
      printf("%10d: %10d\n",t,map[t]);
    }
  }
}
int main() {
  start=malloc(4*1024*1024); // 4 MB of system Heap Space
  end=(char*)start+(4*1024*1024);
  top=start;
  printf("Little_Endian(0) or Big_Endian(1): ");
  int check=scanf("%d",&endian);
  while(check==0 || (endian<0 || endian>1)) {
    while(getchar()!='\n');
    printf("Invalid, Try again: ");
    check=scanf("%d",&endian);
  }
  printf("No of chunks: ");
  size_t n=0,i=0,s=0;
  scanf("%d",&n);
  for(i=0;i<n;i++) {
    printf("Chunk %d size: ",i+1);
    scanf("%d",&s);
    c_malloc(s);
  }
  printf("No of Chunks to Free: ");
  scanf("%d",&n);
  for(i=0;i<n;i++) {
    printf("Chunk %d size: ",i+1);
    scanf("%d",&s);
    c_free(s);
  }
  if(top!=start) {
    chunk_details();
    disp_used_free();
    //    getChunksWise();
  }
  return 0;
}
