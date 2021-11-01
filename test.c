#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
int ex1();
int ex2();

int main(int argc, char **argv)
{

    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */
    unsigned char *wb_blk;
    int i = 0;
    int cnt = 0;
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    if ((blk = readBlockFromDisk(200, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    return 1;


}

int ex1()
{
    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */
    unsigned char *wb_blk;
    int i = 0;
    int cnt = 0;
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    /* Get a new block in the buffer */
    wb_blk = getNewBlockInBuffer(&buf);


    for(int j = 17; j <= 48; ++j)
    {
    /* Read the block from the hard disk */
        if ((blk = readBlockFromDisk(j, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }

        /* Process the data in the block */
        int X = -1;
        int Y = -1;
        char str[5];
        char str2[5];
        printf("read block %d\n", j);
        for (i = 0; i < 7; i++) //一个blk存7个元组加一个地址
        {

            for (int k = 0; k < 4; k++)
            {
                str[k] = *(blk + i*8 + k);
            }
            X = atoi(str);
            if(X == 130)
            {
                for (int k = 0; k < 4; k++)
                {
                    *(wb_blk + 8 * cnt + k) = str[k];
                    *(wb_blk + 8 * cnt + k + 4) = str2[k] = *(blk + i*8 + 4 + k);
                }
                Y = atoi(str2);
                ++cnt;
                printf("(%d, %d) ", X, Y);
            }
        }
        freeBlockInBuffer(blk, &buf);
        printf("\n");

    }
    if (writeBlockToDisk(wb_blk, 8888, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }
    printf("IO's is %d\n\n", buf.numIO); /* Check the number of IO's */
    return 0;
}



void buf_swap(unsigned char *l, unsigned char *r)
{

    for(int i = 0; i < 8; ++i)
    {
        unsigned char tmp = *(l + i);
        *(l + i) = *(r + i);
        *(r + i) = tmp;
    }
}
//if l >= r return 1
//else return 0
int buf_compare(unsigned char *l, unsigned char *r, int off)
{
    char str1[5];
    char str2[5];
    for(int k = 0; k < 4; ++k)
    {
        str1[k] = *(l + k + off);
        str2[k] = *(r + k + off);
    }
    int x = atoi(str1);
    int y = atoi(str2);
    if(x > y) return 1;
    else if(x < y) return 0;
    else if(off != 4) return buf_compare(l, r, 4);
    else return 1;

}
int partition(unsigned char *blk, int l, int r)
{

    //don't sort the addr

    int i = l, j = l;
    while (j < r)
    {
        if((j + 8)%65 == 0)
        {
            j = j + 9;
            continue;
        }
        if(buf_compare(blk + r, blk + j, 0))//blk[j] <= blk[r]
        {

            buf_swap(blk+i, blk+j);
            i = i + 8;
            if((i + 8)%65 == 0)
            {
                i = i + 9;
            }
        }
        j += 8;
    }
    buf_swap(blk + i, blk + r);
    return i;

}

void quick_sort(unsigned char *blk, int l, int r)
{

    if(l%65 == 0) ++l;
    if((l + 8)%65 == 0) l += 9;
    if(r % 65 == 0) r -= 16;
    if((r + 8)%65 == 0) r -= 8;


    if(l >= r) return;
    printf("%d, %d\n", l, r);
    int pi = partition(blk, l, r);
    if((pi - 1)%65 == 0)
    {
        quick_sort(blk, l, pi - 1);
        quick_sort(blk, pi + 8, r);
    }
    else
    {
        quick_sort(blk, l, pi - 8);
        quick_sort(blk, pi + 8, r);
    }

}

int ex2()
{
    Buffer buf;
    unsigned char *blk;
    //four merge sort
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

/**********************************sort R****************************/


//--------------------------------quick sort 16 blocks,echo time for 8-------------//
    for(int i = 1; i <= 8; ++i)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
    }
    quick_sort(buf.data, 1, 504);
    for(int i = 0; i < 8; ++i)
    {
        blk = buf.data + 1 + i * 65;
        if (writeBlockToDisk(blk, i+1, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
    }
    for(int i = 9; i <= 16; ++i)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
    }
    quick_sort(buf.data, 1, 504);
    for(int i = 0; i < 8; ++i)
    {
        blk = buf.data + 1 + i * 65;
        if (writeBlockToDisk(blk, i+9, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
    }


//---------------------------------------------merge sort ,take 2 blocks from each every time-------//
    int j = 1;
    unsigned char *blk1;
    unsigned char *wb_blk1;
    unsigned char *wb_blk2;
    for(int i = 1; i <= 8; ++i)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL || (blk1 = readBlockFromDisk(i+8, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        wb_blk1 = getNewBlockInBuffer(&buf);
        wb_blk2 = getNewBlockInBuffer(&buf);
        int l = 0,r = 0;
        while(l < 7 && r < 7)
        {
            if(buf_compare(blk+l, blk1+r, 0))
            {
                if(l + r < 7)
                {
                    *(wb_blk1 + l + r) = *(blk1 + r);
                }
                else
                {
                    *(wb_blk2 + l + r - 6) = *(blk1 + r);
                }
                ++r;

            }
            else
            {
                if(l + r < 7)
                {
                    *(wb_blk1 + l + r) = *(blk + l);
                }
                else
                {
                    *(wb_blk2 + l + r - 6) = *(blk + l);
                }
                ++l;
            }
        }
        while(l < 7)
        {
            *(wb_blk2 + l + r - 6) = *(blk + l);
            ++l;
        }
        while(r < 7)
        {
            *(wb_blk2 + l + r - 6) = *(blk1 + r);
            ++r;
        }


    }

    /*
    for(int i = 0; i < 8; ++i)
    {
        blk = buf.data + 1 + i * 65;
        printf("block %d %d\n", i ,blk - buf.data);
        int X = -1;
        int Y = -1;
        char str[5];
        char str2[5];
        for (int j = 0; j < 7; j++) //一个blk存7个元组加一个地址
        {
            for (int k = 0; k < 4; k++)
            {
                str[k] = *(blk + j*8 + k);
            }
            X = atoi(str);

            for (int k = 0; k < 4; k++)
            {
                str2[k] = *(blk + j*8 + 4 + k);
            }
            Y = atoi(str2);
            printf("(%d, %d)\n", X, Y);
        }


    }*/
    return 0;
}
