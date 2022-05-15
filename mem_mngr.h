#include <stdio.h>
/*
For the memory manager to be used:
    1. call the "initMemMngr()" to initialize the widths arrays..
    2. if we need to allocate memory for process call "allocateProcess(mem_size, id)" given it as shown the ,mem_size and id of the process
    3. same for deallocation memory just call "deallocateProcess(mem_size, id)" given it as shown the ,mem_size and id of the process
*/

//Memory max size is 1024 (Given)
#define MAX_SIZE (int)1024
//struct for holding the process' memory parameters:
struct memStruct
{
    int start;
    int end;
    int id;
};

struct memStruct n;

/*Lists for different sizes : 256,128,64,32,16,8 -> let minimum size is 2^3=8
these lists are for holding the available slots for each mem width at the front and the used will be at the end*/
struct memStruct arr256[4]; //as the max number of processes with width 256 is 4
struct memStruct arr128[8]; //as the max number of processes with width 128 is 8
struct memStruct arr64[16]; //as the max number of processes with width 64 is 16
struct memStruct arr32[32]; //as the max number of processes with width 32 is 32
struct memStruct arr16[64]; //as the max number of processes with width 16 is 64
struct memStruct arr8[128]; //as the max number of processes with width 8 is 128
struct memStruct *general[6] = {arr8, arr16, arr32, arr64, arr128, arr256};
int nSizes[6] = {128, 64, 32, 16, 8, 4};   //used for sorting;containing the max number of processes per width
int widths[6] = {8, 16, 32, 64, 128, 256}; //different widths for the memory

/*for initializing all the widht's arrays with MAX_SIZE (dummy) except for the 256's array:*/
void initMemMngr()
{
    n.id = -1;
    for (int i = 0; i < 128; i++)
    {
        if (i < 4)
        {
            for (int j = 0; j < 6; j++)
            {
                general[j][i].start = MAX_SIZE;
                general[j][i].end = MAX_SIZE;
            }
        }
        else if (i < 8)
        {
            for (int j = 0; j < 5; j++)
            {
                general[j][i].start = MAX_SIZE;
                general[j][i].end = MAX_SIZE;
            }
        }
        else if (i < 16)
        {
            for (int j = 0; j < 4; j++)
            {
                general[j][i].start = MAX_SIZE;
                general[j][i].end = MAX_SIZE;
            }
        }
        else if (i < 32)
        {
            for (int j = 0; j < 3; j++)
            {
                general[j][i].start = MAX_SIZE;
                general[j][i].end = MAX_SIZE;
            }
        }
        else if (i < 64)
        {
            for (int j = 0; j < 2; j++)
            {
                general[j][i].start = MAX_SIZE;
                general[j][i].end = MAX_SIZE;
            }
        }
        else
        {
            general[0][i].start = MAX_SIZE;
            general[0][i].end = MAX_SIZE;
        }
    }
    //initially we have only 4 portions in arr256:
    for (short i = 0; i < 4; i++)
    {
        arr256[i].start = 256 * i;
        arr256[i].end = arr256[i].start + 256;
    }
}
//getting the least buddy size:
int getBuddySize(int size)
{
    size = size - 1;
    int clog2;
    for (clog2 = 0; size > 0; clog2 = clog2 + 1) //getting the log2 celling
        size = size >> 1;
    int buddySize = 1;
    for (short i = 0; i < clog2; i++) //2 ^ clog2
    {
        buddySize *= 2;
    }
    return (buddySize < 8) ? 8 : buddySize; //as our least width size is 8=2^3
}
//given the buddy size ->return the index of this width in the general array
int getIndex(int size)
{
    for (int i = 0; i < 6; i++)
    {
        if (size == widths[i])
            return i;
    }
    return -1; //not a valid buddy width
}

// sort our lists using insertion sort
void sort(struct memStruct arr[], int n, int size)
{
    int i, j, start, end, id;
    //arrange with start addr
    for (i = 1; i < n; i++)
    {
        start = arr[i].start;
        end = arr[i].end;
        id = arr[i].id;
        j = i - 1;

        while (j >= 0 && arr[j].start > start)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1].start = start;
        arr[j + 1].end = end;
        arr[j + 1].id = id;
    }
    //arrange with id
    for (i = 1; i < n; i++)
    {
        start = arr[i].start;
        end = arr[i].end;
        id = arr[i].id;
        j = i - 1;

        while (j >= 0 && arr[j].id > id)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1].start = start;
        arr[j + 1].end = end;
        arr[j + 1].id = id;
    }
}
//check merging on deallocation
void checkMerging(struct memStruct arr[], int n, int index)
{
    for (int i = 0; i < n - 1; i++)
    {
        /*
            The conditions of merging:
            1.availble empty slots (i.e. not MAX_SIZE)
            2.must be neighbors
            3.the first slot must be in an even address position according to the target widht, i.e. 0x32,2x64,4,128,..
         */
        if (arr[i].start != MAX_SIZE && arr[i].id == 0 && arr[i + 1].id == 0 && arr[i].end == arr[i + 1].start && (arr[i].start / widths[index]) % 2 == 0) //i is even and neigbors->merge
        {
            //put it in the bigger width list
            int indexTemp = index + 1;
            for (int j = 0; j < nSizes[indexTemp]; j++)
            {
                if (general[indexTemp][j].start == MAX_SIZE)
                {
                    general[indexTemp][j].start = arr[i].start;
                    general[indexTemp][j].end = arr[i + 1].end;
                    sort(general[indexTemp], nSizes[indexTemp], widths[indexTemp]);
                    if (indexTemp < 5) //exiting condition no merging after 256's list
                        checkMerging(general[indexTemp], nSizes[indexTemp], indexTemp);
                    break;
                }
            }

            //free it in the current width's list
            arr[i].start = MAX_SIZE;
            arr[i].end = MAX_SIZE;
            arr[i + 1].start = MAX_SIZE;
            arr[i + 1].end = MAX_SIZE;
            sort(general[index], nSizes[index], widths[index]);
            break;
        }
    }
}
//dealocating the memory of that process; by resetting the id of that mem slot and check for merging..
struct memStruct deallocateMemory(int index, int id)
{
    for (int i = 0; i < nSizes[index]; i++)
    {
        if (general[index][i].id == id)
        {
            struct memStruct t;
            t.id = general[index][i].id;
            t.start = general[index][i].start;
            t.end = general[index][i].end;
            general[index][i].id = 0;
            sort(general[index], nSizes[index], widths[index]);
            checkMerging(general[index], nSizes[index], index);
            return t;
        }
    }
}
//allocating the memory in the best suaitable location according to its size; if possible..
struct memStruct allocateMemory(int size, int index, int id)
{
    if (general[index][0].start == MAX_SIZE) // empty; i.e. no avalible slots = All occupied
    {
        if (index == 5)
        {
            printf("No memory can be allocated for this process :( - All Occupied\n");
            return n;
        }
        index++;
        allocateMemory(size, index, id); //recursive calling after increasing the index to search in the bigger width list..
        index--;
        if (general[index][0].start == MAX_SIZE || general[index][0].id != 0) //still empty
            return n;
    }
    if (size == widths[index])
    {
        if (general[index][0].id != 0) //occupied
        {
            printf("No memory can be allocated for this process :( - All Occupied\n");
            return n;
        }
        //allocation this slot to this process
        general[index][0].id = id;
        printf("Process with id= %d start at address %d and ends at %d\n", general[index][0].id, general[index][0].start, general[index][0].end);
        struct memStruct t = general[index][0];
        sort(general[index], nSizes[index], widths[index]);
        return t;
    }
    else
    {
        if (general[index][0].id != 0) //occupied
        {
            printf("No memory can be allocated for this process :( -All Occupied\n");
            return n;
        }
        //putting 2 new slots in the smaller width list
        int indexTemp = index - 1;
        general[indexTemp][0].start = general[index][0].start;
        general[indexTemp][0].end = general[index][0].start + widths[indexTemp];
        general[indexTemp][1].start = general[index][0].start + widths[indexTemp];
        general[indexTemp][1].end = general[index][0].end;
        sort(general[indexTemp], nSizes[indexTemp], widths[indexTemp]);
        //emptying the current width list ->it doesn't have free space then..
        general[index][0].start = MAX_SIZE;
        general[index][0].end = MAX_SIZE;
        sort(general[index], nSizes[index], widths[index]);
    }
}

//the high level functions that process generator call:
//for allocating..
struct memStruct allocateProcess(int mem_size, int id)
{
    int buddySize, index;
    buddySize = getBuddySize(mem_size);
    index = getIndex(buddySize);
    if (index != -1)
        return allocateMemory(buddySize, index, id);
    else
        printf("Invalid process Mem Size i.e size > 256 \n");

    return n;
}
//for deallocating..
struct memStruct deallocateProcess(int mem_size, int id)
{
    int buddySize, index;
    buddySize = getBuddySize(mem_size);
    index = getIndex(buddySize);
    if (index != -1)
        return deallocateMemory(index, id);
    else
        printf("Invalid process Mem Size i.e size > 256 \n");

    return n;
}