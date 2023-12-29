#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
using namespace std;
#define ll long long

// some important contents(thanks to https://www.codecademy.com/learn/computer-architecture/modules/cache/cheatsheet)
// cache memory(what cache contain)
// cache hit/miss
// replacememt(LRU)
// associativity(map method: for each memory loaction(chunk),map it to one, or some or any loaction in cache.)
// n-way set-associative means n blocks per set
// consider the cache has C blocks(total size), then the number of sets is C/n,while in our simulator, we have (2^s) sets(cache index is s bits) and each set has e lines/blocks, so the total number of blocks in fact is e*(2^s)
// thus, we can imagin if s=0, then this is full associative. On the other hand, if e=1, this is direct map
// write back(don't care in this lab)

//(thanks to https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec15.pdf)
// three of four important problems
// 1. address map
// ( using tag(supply the rest of the adress bits) to determine find or not)
// 2. determine miss or hit
// 3. whether eviction and which one to replace
//

short to_num(char p){
    if('0'<=p&&p<='9'){
        return p-48;
    }
    else if('a'<=p&&p<='f'){
        return p-'a'+10;
    }
    else if('A'<=p&&p<='F'){
        return p-'A'+10;
    }
    else{
        cout<<p<<endl;
        cout<<"bad address"<<endl;
        exit(0);
    }
}

typedef struct CacheLine_ {
	int valid, time_stamp;// valid bit:when init, zero. after load data, 1.
    ll tag;// tag corresponds to the main memory loaction
} CacheLine;

typedef struct Cache_ {
	int S, E, B;
	CacheLine **line;
    //the cache has (2^s)sets, each set has e blocks
    //(each block contains an entry(one byte(not care ) and ) from memory)
    //especialy, each block contain (2^b) bits
    //
} Cache;

ll time_stamp=0;
int hit_cnt = 0;
int miss_cnt = 0;
int eviction_cnt = 0;

int verbose = 0;

char trace_file_name[1000];
Cache *cache = NULL;//this is the cache
class Trace{
public:
    //1:I(ignore instruction load) 2:L 3:S 4:M
    ll add;  //ignore size
    int index,tag;
    void init(){index=tag=add=0;}
    Trace(){init();}
    void get_trace(char* single){
        init();
        char *p=single+3;
        while(*p!=',') add=(add<<4)+to_num(*(p++));
        get_set_index_tag();
        work(single);
    }

    void get_set_index_tag(){
        int b = cache->B,s=cache->S;
        ll addt = add;
        addt/=b;
        index = addt%s;
        tag = addt/s;
    }

    int get_eviction_index(){
        int e = cache->E;
        ll last_use=(1LL<<60);
        int last_use_index=-1;
        for(int i=0;i<e;i++){
            if(cache->line[index][i].time_stamp<last_use){
                last_use = cache->line[index][i].time_stamp;
                last_use_index = i; 
            }
        }
        return last_use_index;
    }

    int find(){
        int e = cache->E;
        for(int i=0;i<e;i++){
            if(cache->line[index][i].valid==1&&cache->line[index][i].tag==tag){//有效且找到
                return i;
            }
        }
        return -1;
    }

    int check_empty(){
        int e = cache->E;
         for(int i=0;i<e;i++){
            if(cache->line[index][i].valid==0){//empty
                return i;
            }
        }
        return -1;
    }

    void load(){
        int empty_index = check_empty();
        if(empty_index!=-1){//有空的
            cache->line[index][empty_index].valid=1; 
            cache->line[index][empty_index].tag=tag;
            cache->line[index][empty_index].time_stamp=time_stamp++;
            return;
        }
        else{
            if(verbose) cout<<"eviction ";
            eviction_cnt++;
            int evi_index = get_eviction_index();
            cache->line[index][evi_index].tag=tag;
            cache->line[index][evi_index].time_stamp=time_stamp++;           
        }   
    }

    void read_or_write_cache(){
        int block_number = find();
        if(block_number!=-1){
            hit_cnt++;
            if(verbose) cout<<"hit ";
            cache->line[index][block_number].time_stamp=time_stamp++;
            return;
        }
        else{
        
            if(verbose) cout<<"miss ";
            miss_cnt++;
            load();
        }
    }
/*
    void write_cache(){
        int block_number = find();
        if(block_number != -1){
            hit_cnt++;
            if(verbose) cout<<"hit ";
            cache->line[index][block_number].time_stamp=time_stamp++;
            return; 
        }
        else{
            if(verbose) cout<<"miss ";
            miss_cnt++;
            load();           
        }
    }
*/
    void modify_cache(){
        read_or_write_cache();
        read_or_write_cache();
    }

    void work(char *single){
        if(single[0]=='I'){//I
            return; 
        }
        else{
            switch(single[1]){
                case 'M':
                    modify_cache();
                    break;
                 case 'L':
                 case 'S':
                    read_or_write_cache();
                    break;
                default:
                    printf("memory trace bad formatted");
                    exit(-1);
            }
        }
    }
}single_trace;
                   //


//LRU: The data that has not been accessed for the longest period of time will be replaced.

void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

void InitCache(int s, int E, int b) {
	int S = 1 << s;
	int B = 1 << b;
	cache = (Cache *)malloc(sizeof(Cache));
	cache->S = S;
	cache->E = E;
	cache->B = B;
	cache->line = (CacheLine **)malloc(sizeof(CacheLine *) * S);
	for (int i = 0; i < S; i++) {
		cache->line[i] = (CacheLine *)malloc(sizeof(CacheLine) * E);
		for (int j = 0; j < E; j++) {
			cache->line[i][j].valid = 0;
			cache->line[i][j].tag = -1;
			cache->line[i][j].time_stamp = 0;
		}
	}
}

void FreeCache() {
  for (int i = 0, s = cache->S; i < s; i++) {
    free(cache->line[i]);
  }
  free(cache->line);
  free(cache);
}

void PrintNoArg(char cmdName[]) {
    printf("%s: Missing required command line argument\n",cmdName);
}

void PrintHelp() {
	printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("-h         Print this help message.\n");
	printf("-v         Optional verbose flag.\n");
	printf("-s <num>   Number of set index bits.\n");
	printf("-E <num>   Number of lines per set.\n");
	printf("-b <num>   Number of block offset bits.\n");
	printf("-t <file>  Trace file.\n\n");
	printf("Examples:\n");
	printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int main(int argc, char *argv[]) {
	int s, E, b;
	char op;
    bool init_s=false, init_E=false, init_b=false, init_t = false;

	while (EOF != (op = getopt(argc, argv, "hvs:E:b:t:"))) {
		switch (op) {
			case 'h':
				PrintHelp();
				exit(0);
			case 'v':
				verbose = 1;
				break;
			case 's':
                init_s=true;
				s = atoi(optarg);
				break;
			case 'E':
                init_E=true;
				E = atoi(optarg);
				break;
			case 'b':
                init_b=true;
    			b = atoi(optarg);
				break;
			case 't':
                init_t=true;
				strcpy(trace_file_name, optarg);
				break;
			default:
				PrintHelp();
				exit(-1);
		}
	}
    if(!(init_s&&init_E&&init_b&&init_t)){//仿真一下没有输入的参数
        PrintNoArg(argv[0]);
        PrintHelp();
        return 0;
    }   
	InitCache(s, E, b);
    freopen(trace_file_name,"r",stdin);
    char single[1000];
	while(cin.getline(single,1000)){
        if(single[0]!='I'&&single[1]!='M'&&single[1]!='L'&&single[1]!='S')
            continue;
        if (verbose&&single[0]==' '){
            cout<<(single+1)<<" ";
        }
        
        single_trace.get_trace(single);
        if(verbose&&single[0]==' ')cout<<endl;
    }
	FreeCache();
	printSummary(hit_cnt, miss_cnt, eviction_cnt);
	return 0;
}
