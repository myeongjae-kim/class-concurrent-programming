#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXSTR 21

#define MEM 1
#define FIL 2
#define BLF 3
#define ARG 1


void EIN(void* adr, int messageselector); // Error If Null
// messageselector: 0 or any number -> default, 1 -> memory, 2 -> file 3 -> bloomfilter.

void EIF(int adr, int messageselector); // Error If False
// messageselector: 0 or any number -> default, 1-> argument.

typedef struct _Bit 
{
    unsigned int bit : 1;
} Bit;

typedef unsigned int (*hashfunc_t)(const char *);

typedef struct _BloomFilters
{
    int FILTER_SIZE;
    int NUM_HASHFUNC;
    Bit *FilterArray;
    hashfunc_t *funcs; // function pointer array for hash functions
} BloomFilters;

void insert(BloomFilters* B, char *Key);
int lookup(BloomFilters* B, char *Key);
int Hash (BloomFilters* B, unsigned int Key);
BloomFilters* CreateBloomFilter(int s, int b, int k);
void DestroyBloomFilter(BloomFilters* B);

void PrintFilterArray(BloomFilters* B);

unsigned int Hash1 (const char *Key);
unsigned int Hash2 (const char *Key);
unsigned int Hash3 (const char *Key);
unsigned int Hash4 (const char *Key);
unsigned int Hash5 (const char *Key);
unsigned int Hash6 (const char *Key);
unsigned int Hash7 (const char *Key);
unsigned int Hash8 (const char *Key);
unsigned int Hash9 (const char *Key);
unsigned int Hash10 (const char *Key);

int main(int argc, char const *argv[])
{
    FILE *fp;
    BloomFilters *B;
    int i, InputStrNum, b, k;
    float const errprob = 0.01; // bloom filter error probability
    char buff[MAXSTR];

    b = (int)(1.44 * log10(1.0 / errprob) / log10(2.0)); // b: bit per an object. // due to KI! Thanks!
    k = (int)(log(2.0) * b); // k: number of hash functions

    // b = ceil((InputStrNum * log(0.01)) / log(1.0 / (pow(2.0, log(2.0)))));
    // k = round(log(2.0) * b / InputStrNum);
    // http://hur.st/bloomfilter

    EIF(argc == 2, ARG);
    EIN(fp = fopen(argv[1], "r"), FIL);
    fscanf(fp, "%d\n", &InputStrNum);

    B = CreateBloomFilter(InputStrNum, b, k);

    for (i = 0; i < InputStrNum; ++i)
    {
        memset(buff,0,sizeof(buff));
        fscanf(fp, "%[^\n]\n", buff); // %[^\n] 뒤에 \n을 적어주지 않으면 두 번째 반복부턴 버퍼에 입력이 안됨. 파일 포인터가 이동하지 않기 때문.
        insert(B, buff);
    }
    
    while(!feof(fp))
    {
        memset(buff,0,sizeof(buff));
        fscanf(fp, "%[^\n]\n", buff);
        if(!lookup(B, buff))
        {
            printf("%s = false\n", buff);
        }
        else
        {
            printf("%s = true\n", buff);
        }
    }
    PrintFilterArray(B);
    DestroyBloomFilter(B);
    return 0;
}

void EIN(void* adr, int messageselector) // Error If Null
{       // messageselector: 0 or any number -> default, 1 -> memory, 2 -> file.
    if(!adr)
    {
        switch (messageselector) {
            case 1:
                printf("Memory cannot be allocated!\n");
                break;
            
            case 2:
                printf("File Opening Error!\n");
                break;

            case 3:
                printf("There is no bllomfilter!\n");
                break;
                
            default:
                printf("Null Address Error!\n");
                break;
        }
        exit(-1);
    }
}

void EIF(int adr, int messageselector) // Error If FALSE
{       // messageselector: 0 or any number -> default, 1-> argument.
    if(!adr)
    {
        switch (messageselector) {
            case 1:
                printf("Argument Number Error!\n");
                break;
                
            default:
                printf("FALSE! FALSE!\n");
                break;
        }
        exit(-1);
    }
}

void insert(BloomFilters* B, char *Key)
{
    int idx;

    EIN(B, BLF);
    printf("%s\n", Key);
    for (int i = 0; i < B -> NUM_HASHFUNC; ++i)
    {
        B -> FilterArray[idx = Hash(B, B -> funcs[i](Key))].bit = 1;
        printf("a[%d] = %d\n",idx, B -> FilterArray[idx].bit);
    }
}
int lookup(BloomFilters* B, char *Key)
{
    EIN(B, BLF);
    for (int i = 0; i < B -> NUM_HASHFUNC; ++i)
    {
        if(B -> FilterArray[Hash(B, B -> funcs[i](Key))].bit != 1)
            return 0;
    }
    return 1;
}
int Hash (BloomFilters* B, unsigned int Key)
{
    EIN(B, BLF);
    return Key % B -> FILTER_SIZE;
}
BloomFilters* CreateBloomFilter(int s, int b, int k)
{ // s : number of strings, b = bits per an object, k = number of hash function
    BloomFilters* B;
    int i;

    EIN(B = (BloomFilters*)malloc(sizeof(BloomFilters)), MEM);
    B -> FILTER_SIZE = s * b;
    B -> NUM_HASHFUNC = k;
    EIN(B -> FilterArray = (Bit*)calloc(B -> FILTER_SIZE,sizeof(Bit)), MEM);

    EIN(B -> funcs = (hashfunc_t*)malloc(B -> NUM_HASHFUNC *sizeof(hashfunc_t)), MEM);
    
    switch(B -> NUM_HASHFUNC)
    {
        default:
        case 10:
            B -> funcs[9] = Hash10;
        case 9:
            B -> funcs[8] = Hash9;
        case 8:
            B -> funcs[7] = Hash8;
        case 7:
            B -> funcs[6] = Hash7;
        case 6:
            B -> funcs[5] = Hash6;
        case 5:
            B -> funcs[4] = Hash5;
        case 4:
            B -> funcs[3] = Hash4;
        case 3:
            B -> funcs[2] = Hash3;
        case 2:
            B -> funcs[1] = Hash2;
        case 1:
            B -> funcs[0] = Hash1;
            break;
    }
    return B;
}

void DestroyBloomFilter(BloomFilters* B)
{
    EIN(B, BLF);
    free(B -> funcs);
    free(B -> FilterArray);
    free(B);
}

void PrintFilterArray(BloomFilters* B)
{
    EIN(B, BLF);
    for (int i = 0; i < B -> FILTER_SIZE; ++i)
    {
        printf("%d", B -> FilterArray[i].bit);
    }
    putchar('\n');
}

unsigned int Hash1 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal += *Key + 31;
    return HashVal;
}
unsigned int Hash2 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal += *Key * 31;
    return HashVal;
}
unsigned int Hash3 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal += (*Key + 31) *11;
    return HashVal;
}
unsigned int Hash4 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal = (HashVal >> 5 ) + *Key++;
    return HashVal;
}
unsigned int Hash5 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal = (HashVal << 5 ) + *Key++;
    return HashVal;
}
unsigned int Hash6 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal = (HashVal << 5 ) + (HashVal >> 5 ) + *Key++ ;
    return HashVal;
}
unsigned int Hash7 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal += (*Key * 11) +31;
    return HashVal;
}
unsigned int Hash8 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal = ((HashVal << 5 ) + (HashVal >> 5 )) *11 + *Key++ ;
    return HashVal;
}
unsigned int Hash9 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal = (HashVal >> 5 )*11 + *Key++;
    return HashVal;
}
unsigned int Hash10 (const char *Key)
{
    int HashVal=0;
    for(;*Key !='\0'; Key ++)
        HashVal = (HashVal << 5 ) *11 + *Key++;
    return HashVal;
}
