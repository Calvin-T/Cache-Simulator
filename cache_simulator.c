#include<stdlib.h>
#include<stdio.h>
#include<string.h>


struct Queue{
	unsigned long front, rear, size, associativity;
	unsigned long* array;
};

struct Queue initQueue(unsigned long associativity){
	struct Queue queue;
	queue.associativity = associativity;
	queue.front = 0 ;
	queue.rear = associativity - 1;
	queue.size = 0;
	queue.array = (unsigned long*)malloc(associativity*sizeof(unsigned long));
	return queue;
}

int isFull(struct Queue* queue,unsigned long index){
	return queue[index].size == queue[index].associativity;
}

void enqueue(struct Queue* queue,unsigned long tag,unsigned long index){
	if(isFull(queue,index)){
		return;
	}
	queue[index].rear = (queue[index].rear + 1)%queue[index].associativity;
	queue[index].array[queue[index].rear] = tag;
	queue[index].size += 1;
}
unsigned long dequeue(struct Queue* queue,unsigned long index){
	if(queue[index].size == 0){
		return 0;
	}
	unsigned long temp = queue[index].array[queue[index].front];
	queue[index].front = (queue[index].front + 1)%queue[index].associativity;
	queue[index].size -= 1;
	return temp;
}

int search(struct Queue* queue,unsigned long tag,unsigned long index){
	int i;
	for(i = 0;i<queue[index].associativity;i++){
		if(queue[index].array[i]==tag){
			return 1;
		}
	}
	return 0;
}

int findLog(int x){
	int log = 0;
	while(x > 1){
		x/=2;
		log++;
	}
	return log;
}

unsigned long int getIndex(unsigned long int address, int blockBits, unsigned long int indexBits){
	unsigned long int mask = ((1 << indexBits) - 1) << blockBits;
	unsigned long int index = (address & mask) >> blockBits;
	return index;
}

unsigned long int getTag(unsigned long int address, int blockBits, unsigned long int indexBits){
	unsigned long int index = address >> (blockBits+indexBits);
	return index;
}

int main(int argc, char** argv){

	FILE* fp;
	fp = fopen(argv[5],"r'");
	int policy;
	if(strcmp(argv[3],"fifo")==0){
		//printf("FIFO\n");
		policy = 0;
	} else {
		//printf("LRU\n");
		policy = 1;
	}
	unsigned long int cacheSize = atoi(argv[1]);
	unsigned long int blockSize = atoi(argv[4]);
	unsigned long int associativity,sets;
	if(strcmp(argv[2],"direct")==0){
		associativity = 1;
		sets = cacheSize/blockSize;
	} else if(strcmp(argv[2],"assoc")==0){
		sets = 1;
		associativity = cacheSize/blockSize;
	} else{
		associativity = atoi(argv[2]+6);
		sets = cacheSize / (associativity*blockSize);
	}

	int numOfBlockBits = findLog(blockSize);
	int numOfIndexBits = findLog(sets);



	struct Queue* cache =malloc(sets*sizeof(struct Queue));
	int i;
	for(i = 0;i<sets;i++){
		cache[i] = initQueue(associativity);
	}

	char* line = malloc(50*sizeof(char));
	char* op = malloc(2*sizeof(char));
	unsigned long int address;

	int reads = 0;
	int writes = 0;
	int hits = 0;
	int misses = 0;

	while(!feof(fp)){
		fgets(line,50,fp);
		if(strcmp(line,"#eof\n")==0){
			break;
		}
		sscanf(line,"%*s %s %lx",op,&address);

		unsigned long indexBits = getIndex(address,numOfBlockBits,numOfIndexBits);
		unsigned long tagBits = getTag(address,numOfBlockBits,numOfIndexBits);

		if(strcmp(op,"R")==0){
			if(search(cache,tagBits,indexBits)==0){
				misses++;
				reads++;
				if(isFull(cache,indexBits)){
					dequeue(cache,indexBits);
				}
				enqueue(cache,tagBits,indexBits);
			} else {
				hits++;
				if(policy == 1){
					int i,j;
					unsigned long temp;
					i = 0;
					while(i<associativity){
						if(cache[indexBits].array[i] == tagBits){
							break;
						}
						i++;
					}
					if(i == 0){
						temp = dequeue(cache,indexBits);
						enqueue(cache,temp,indexBits);
					} else if(i != 0 && i != cache[indexBits].size - 1){
						for(j = i;j<(cache[indexBits].size)-1;j++){
							cache[indexBits].array[j] = cache[indexBits].array[j+1];
						}
						j++;
						cache[indexBits].array[j] = tagBits;
					}
				}
			}
		} else {
			writes++;
			if(search(cache,tagBits,indexBits)==0){
				misses++;
				reads++;
				if(isFull(cache,indexBits)){
					dequeue(cache,indexBits);
				}
				enqueue(cache,tagBits,indexBits);
			} else {
				hits++;
				if(policy == 1){
					int i,j;
					unsigned long temp;
					i = 0;
					while(i<associativity){
						if(cache[indexBits].array[i] == tagBits){
							break;
						}
						i++;
					}
					if(i == 0){
						temp = dequeue(cache,indexBits);
						enqueue(cache,temp,indexBits);
					} else if(i != 0 && i != cache[indexBits].size - 1){
						for(j = i;j<(cache[indexBits].size)-1;j++){
							cache[indexBits].array[j] = cache[indexBits].array[j+1];
						}
						j++;
						cache[indexBits].array[j] = tagBits;
					}
				}
			}
		}


	}

	printf("no-prefetch\n");
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);

	rewind(fp);

	for(i = 0;i<sets;i++){
		cache[i] = initQueue(associativity);
	}

	reads = 0;
	writes = 0;
	hits = 0;
	misses = 0;

	while(!feof(fp)){
		fgets(line,50,fp);
		if(strcmp(line,"#eof\n")==0){
			break;
		}
		sscanf(line,"%*s %s %lx",op,&address);

		unsigned long indexBits = getIndex(address,numOfBlockBits,numOfIndexBits);
		unsigned long tagBits = getTag(address,numOfBlockBits,numOfIndexBits);
		unsigned long prefetchAdd;

		if(strcmp(op,"R")==0){
			if(search(cache,tagBits,indexBits)==0){
				misses++;
				reads++;
				if(isFull(cache,indexBits)){
					dequeue(cache,indexBits);
				}
				enqueue(cache,tagBits,indexBits);
				//PREFETCH
				prefetchAdd = address+blockSize;
				unsigned long int preIndexbit = getIndex(prefetchAdd,numOfBlockBits,numOfIndexBits);
				unsigned long int preTagbit = getTag(prefetchAdd,numOfBlockBits,numOfIndexBits);
				if(search(cache,preTagbit,preIndexbit)==0){
					reads++;
					if(isFull(cache,preIndexbit)){
						dequeue(cache,preIndexbit);
					}
					enqueue(cache,preTagbit,preIndexbit);
				} else {
					if(policy == 1){
						int i,j;
						unsigned long temp;
						i = 0;
						while(i<associativity){
							if(cache[preIndexbit].array[i] == preTagbit){
								break;
							}
							i++;
						}
						if(i == 0){
							temp = dequeue(cache,preIndexbit);
							enqueue(cache,temp,preIndexbit);
						} else if(i != 0 && i != cache[preIndexbit].size - 1){
							for(j = i;j<(cache[preIndexbit].size)-1;j++){
								cache[preIndexbit].array[j] = cache[preIndexbit].array[j+1];
							}
							j++;
							cache[preIndexbit].array[j] = preTagbit;
						}
					}
				}

			} else {
				hits++;
				if(policy == 1){
					int i,j;
					unsigned long temp;
					i = 0;
					while(i<associativity){
						if(cache[indexBits].array[i] == tagBits){
							break;
						}
						i++;
					}
					if(i == 0){
						temp = dequeue(cache,indexBits);
						enqueue(cache,temp,indexBits);
					} else if(i != 0 && i != cache[indexBits].size - 1){
						for(j = i;j<(cache[indexBits].size)-1;j++){
							cache[indexBits].array[j] = cache[indexBits].array[j+1];
						}
						j++;
						cache[indexBits].array[j] = tagBits;
					}
				}
			}
		} else {
			writes++;
			if(search(cache,tagBits,indexBits)==0){
				misses++;
				reads++;
				if(isFull(cache,indexBits)){
					dequeue(cache,indexBits);
				}
				enqueue(cache,tagBits,indexBits);
				//PREFETCH
				prefetchAdd = address+blockSize;
				unsigned long int preIndexbit = getIndex(prefetchAdd,numOfBlockBits,numOfIndexBits);
				unsigned long int preTagbit = getTag(prefetchAdd,numOfBlockBits,numOfIndexBits);
				if(search(cache,preTagbit,preIndexbit)==0){
					reads++;
					if(isFull(cache,preIndexbit)){
						dequeue(cache,preIndexbit);
					}
					enqueue(cache,preTagbit,preIndexbit);
				} else {
					if(policy == 1){
						int i,j;
						unsigned long temp;
						i = 0;
						while(i<associativity){
							if(cache[preIndexbit].array[i] == preTagbit){
								break;
							}
							i++;
						}
						if(i == 0){
							temp = dequeue(cache,preIndexbit);
							enqueue(cache,temp,preIndexbit);
						} else if(i != 0 && i != cache[preIndexbit].size - 1){
							for(j = i;j<(cache[preIndexbit].size)-1;j++){
								cache[preIndexbit].array[j] = cache[preIndexbit].array[j+1];
							}
							j++;
							cache[preIndexbit].array[j] = preTagbit;
						}
					}
				}
			} else {
				hits++;
				if(policy == 1){
					int i,j;
					unsigned long temp;
					i = 0;
					while(i<associativity){
						if(cache[indexBits].array[i] == tagBits){
							break;
						}
						i++;
					}
					if(i == 0){
						temp = dequeue(cache,indexBits);
						enqueue(cache,temp,indexBits);
					} else if(i != 0 && i != cache[indexBits].size - 1){
						for(j = i;j<(cache[indexBits].size)-1;j++){
							cache[indexBits].array[j] = cache[indexBits].array[j+1];
						}
						j++;
						cache[indexBits].array[j] = tagBits;
					}
				}
			}
		}


	}

	printf("with-prefetch\n");
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);

	return 0;


}



