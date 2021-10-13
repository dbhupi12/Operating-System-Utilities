#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<limits.h>



int main(int argc, char **argv){
    srand(5635);
    int max_frames = atoi(argv[2]);
    int verbose_MOD = 0;
    if(argc > 4 && strcmp(argv[4], "-verbose") == 0) {
        verbose_MOD = 1;
    }
    int *vpn_table = (int*)malloc((1<<20)*sizeof(int));
    for(int i = 0 ; i < (1<<20); i++){
        vpn_table[i] = 0;
    }
    int pfn_table[max_frames];
    for(int i = 0 ; i < max_frames; i++){
        pfn_table[i] = 0;
    }
    char read[20];
    /* 
    below varibales are for implementing different policies
    */
    int fifo_cnt = 0; //FIFO
    int time[max_frames]; //LRU
    memset(time,-1,sizeof(time));
    int ctime = 0; //LRU
   //  FOR OPT
    int **arr = (int **)malloc((1<<20)*sizeof(int *));
    int *size = (int *)malloc((1<<20)*sizeof(int));
    int *curr_opt_index = (int *)malloc((1<<20)*sizeof(int));
    int *max_sizes = (int *)malloc((1<<20)*sizeof(int));
    memset(max_sizes,0,sizeof(int)*(1<<20));
    int index = max_frames;
    for(int i = 0 ; i < (1<<20); i++){
        curr_opt_index[i] = 0;
    }

    for(int i = 0 ; i < (1<<20); i++){
        size[i] = -1;
    }

    /*for(int i = 0 ; i < (1<<20); i++){
    arr[i] = (int *)malloc(10*sizeof(int)); 
    }*/
    int idx = 0;
    int prev_size = 8;
    if(!strcmp("OPT",argv[3])){
    FILE *file = fopen(argv[1],"r");
    while(!feof(file))
        {
            fgets (read, 20, file );
            char *token;
            char *oper;
            token = strtok(read," \n");
            oper = strtok(NULL," \n");
            if(!oper){
                continue;
            }
            long add = strtol(token, NULL, 0);
            int vpn = (int)(add>>12);
            if(arr[vpn] == NULL){
                arr[vpn] = (int *)malloc(10*sizeof(int));
                max_sizes[vpn] = 10;
            }
            arr[vpn][++size[vpn]] = idx;
            idx++;
            if(size[vpn] == max_sizes[vpn]-1){
                arr[vpn] = (int *)realloc(arr[vpn],(max_sizes[vpn]*2)*sizeof(int));
                max_sizes[vpn] = 2*max_sizes[vpn];
            }
        }
        fclose(file);
    }
   //...................................................................
    int pointer = 0; //CLOCK
    /*
     output variables
    */
   int accesses = 0;
   int misses = 0;
   int writes = 0;
   int drops = 0;

    FILE *filepointer = fopen(argv[1],"r");
    int curr_frame_no = 0;
    if (filepointer == NULL)
    {
        printf("%s file failed to open",argv[1]);
    }
    else{
        while(!feof(filepointer))
        {
            fgets (read, 20, filepointer );
            char *token;
            char *oper;
            token = strtok(read," \n");
            oper = strtok(NULL," \n");
            if(!oper){
                continue;
            }
            accesses++;
            char comp[3];
            strcpy(comp,"W");
            long add = strtol(token, NULL, 0);
            int vpn = (int)(add>>12);
            int present_bit = vpn_table[vpn] & (1<<10);
            present_bit = present_bit>>10;
            curr_opt_index[vpn]++;

            if(present_bit){ 
                if(!strcmp("CLOCK",argv[3])){
                    vpn_table[vpn] |= (1<<12);
                }

                time[vpn_table[vpn] & 1023] = ctime;
                ctime++;
                if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
            }
            else if(!strcmp("FIFO",argv[3])){
                misses++;
                if(curr_frame_no < max_frames){
                    vpn_table[vpn] = curr_frame_no | (1<<10);
                    pfn_table[curr_frame_no] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                    curr_frame_no++;
                }
                else{
                    int dirty_bit = vpn_table[pfn_table[fifo_cnt%max_frames]] & (1<<11);
                    vpn_table[pfn_table[fifo_cnt%max_frames]] = 0;
                    dirty_bit = dirty_bit>>11;
                    if(dirty_bit){
                        writes++;
                    }
                    else{
                        drops++;
                    }

                    if(verbose_MOD) {

                        if(!dirty_bit) {
                            printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", 
                                vpn, pfn_table[fifo_cnt%max_frames]);
                        }

                        else{
                            printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", 
                                vpn, pfn_table[fifo_cnt%max_frames]);
                        }
        

                    }


                    vpn_table[vpn] = (fifo_cnt%max_frames) | (1<<10);
                    pfn_table[fifo_cnt%max_frames] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                    fifo_cnt++;
                }
            }
            else if(!strcmp("RANDOM",argv[3])){
                misses++;
                if(curr_frame_no < max_frames){
                    vpn_table[vpn] = curr_frame_no | (1<<10);
                    pfn_table[curr_frame_no] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                    curr_frame_no++;
                }
                else{
                    int out_page;
                    out_page = rand()%max_frames;
                    
                    int dirty_bit = vpn_table[pfn_table[out_page]] & (1<<11);
                    vpn_table[pfn_table[out_page]] = 0;
                    dirty_bit = dirty_bit>>11;
                    if(dirty_bit){
                        writes++;
                    }
                    else{
                        drops++;
                    }

                    if(verbose_MOD) {

                        if(!dirty_bit) {
                            printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", 
                                vpn, pfn_table[out_page]);
                        }

                        else{
                            printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", 
                                vpn, pfn_table[out_page]);
                        }
        

                    }

                    vpn_table[vpn] = (out_page) | (1<<10);
                    pfn_table[out_page] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                }
            }
            else if(!strcmp("LRU",argv[3])){
                misses++;
                if(curr_frame_no < max_frames){
                    vpn_table[vpn] = curr_frame_no | (1<<10);
                    pfn_table[curr_frame_no] = vpn;
                    time[curr_frame_no] = ctime;
                    ctime++;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                    curr_frame_no++;
                }
                else{
                    int lru = 0;
                    for(int i = 0; i<max_frames; i++){
                        if(time[i] < time[lru]){
                            lru = i;
                        }
                    }

                    int dirty_bit = vpn_table[pfn_table[lru]] & (1<<11);
                    vpn_table[pfn_table[lru]] = 0;
                    dirty_bit = dirty_bit>>11;
                    if(dirty_bit){
                        writes++;
                    }
                    else{
                        drops++;
                    }

                    if(verbose_MOD) {

                        if(!dirty_bit) {
                            printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", 
                                vpn, pfn_table[lru]);
                        }

                        else{
                            printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", 
                                vpn, pfn_table[lru]);
                        }
        

                    }

                    vpn_table[vpn] = (lru) | (1<<10);
                    pfn_table[lru] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                    time[lru] = ctime;
                    ctime++;
                }
            }
            else if(!strcmp("OPT",argv[3])){
                misses++;
                if(curr_frame_no < max_frames){
                    vpn_table[vpn] = curr_frame_no | (1<<10);
                    pfn_table[curr_frame_no] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                    curr_frame_no++;
                }
                else{
                    int out_page = 0;
                    for(int i = 0; i<max_frames ; i++){
                        int pn1 = pfn_table[out_page];
                        int pn2 = pfn_table[i];
                        if(curr_opt_index[pn2] == size[pn2]+1){
                            out_page = i;
                            break;
                        }
                        int c1 = arr[pn1][curr_opt_index[pn1]];
                        int c2 = arr[pn2][curr_opt_index[pn2]];
                        if(c2 > c1){
                            out_page = i;
                        }
                    }

                    int dirty_bit = vpn_table[pfn_table[out_page]] & (1<<11);
                    vpn_table[pfn_table[out_page]] = 0;
                    dirty_bit = dirty_bit>>11;
                    if(dirty_bit){
                        writes++;
                    }
                    else{
                        drops++;
                    }

                    if(verbose_MOD) {

                        if(!dirty_bit) {
                            printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", 
                                vpn, pfn_table[out_page]);
                        }

                        else{
                            printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", 
                                vpn, pfn_table[out_page]);
                        }
        

                    }


                    vpn_table[vpn] = (out_page) | (1<<10);
                    pfn_table[out_page] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                }
            }else if(!strcmp("CLOCK",argv[3])){
                misses++;
                if(curr_frame_no < max_frames){
                    vpn_table[vpn] = curr_frame_no | (1<<10);
                    pfn_table[curr_frame_no] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    } 
                    curr_frame_no++;
                }
                else{
                    int temp = pointer;
                    int out_page;
                    int check = 0;
                    for(;pointer<max_frames;pointer++){
                        int second_chance_bit = vpn_table[pfn_table[pointer]] & (1<<12);
                        second_chance_bit = second_chance_bit>>12;
                        if(second_chance_bit){
                            vpn_table[pfn_table[pointer]] &= 4091;
                        }
                        else{
                            out_page = pointer;
                            pointer++;
                            check = 1;
                            break;
                        }
                    }
                    if(pointer == max_frames && check == 0){
                        pointer = 0;
                        for (;pointer < temp; pointer++)
                        {
                        int second_chance_bit = vpn_table[pfn_table[pointer]] & (1<<12);
                        second_chance_bit = second_chance_bit>>12;
                        if(second_chance_bit){
                            vpn_table[pfn_table[pointer]] &= 4091;
                        }
                        else{
                            check = 1;
                            out_page = pointer;
                            pointer++;
                            break;
                        } 
                        }
                    }

                    if(check == 0) {
                        out_page = pointer;
                        pointer++;
                    }

                    pointer %= max_frames; 
                    
                    int dirty_bit = vpn_table[pfn_table[out_page]] & (1<<11);
                    vpn_table[pfn_table[out_page]] = 0;
                    dirty_bit = dirty_bit>>11;
                    if(dirty_bit){
                        writes++;
                    }
                    else{
                        drops++;
                    }
                    if(verbose_MOD) {

                        if(!dirty_bit) {
                            printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", 
                                vpn, pfn_table[out_page]);
                        }

                        else{
                            printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", 
                                vpn, pfn_table[out_page]);
                        }
        

                    }
                    vpn_table[vpn] = (out_page) | (1<<10) | (1<<12);
                    pfn_table[out_page] = vpn;
                    if(!strcmp(oper,comp)){
                        vpn_table[vpn] |= (1<<11); 
                    }
                }
            }
         }
    }
    fclose(filepointer);
    printf("Number of memory accesses: %d\n",accesses);
    printf("Number of misses: %d\n",misses);
    printf("Number of writes: %d\n",writes);
    printf("Number of drops: %d\n",drops);
}