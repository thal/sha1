#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t H[5] =
{
    0x67452301,
    0xEFCDAB89,
    0x98BADCFE,
    0x10325476,
    0xC3D2E1F0
};

uint32_t rotl( uint32_t x, int shift )
{
    return (x << shift) | (x >> (sizeof(x)*8 - shift));
}

uint32_t roundFunc( uint32_t b, uint32_t c, uint32_t d, int roundNum )
{
    if( roundNum <= 19 )
    {
        return (b & c) | ((~b) & d);
    }
    else if( roundNum <= 39 )
    {
        return ( b ^ c ^ d );
    }
    else if( roundNum <= 59 )
    {
        return (b & c) | (b & d) | (c & d);
    }
    else
    {
        return ( b ^ c ^ d );
    }
}

uint32_t kForRound( int roundNum )
{
    if( roundNum <= 19 )
    {
        return 0x5a827999;
    }
    else if( roundNum <= 39 )
    {
        return 0x6ed9eba1;
    }
    else if( roundNum <= 59 )
    {
        return 0x8f1bbcdc;
    }
    else
    {
        return 0xca62c1d6;
    }
}

int pad(uint8_t * block, uint8_t * extraBlock, int blockSize, int fileSize)
{
    int twoBlocks = 0;
    //l is block size in bits
    uint64_t l = (uint64_t)fileSize * 8;
    if(blockSize <= 55)
    {
        block[blockSize] = 0x80;
        int i;
        for( i = 0; i < 8; i++ )
        {
            block[56+i] = (l >> (56-(8*i)));
        }
    }
    else
    {
        twoBlocks = 1;
        if(blockSize < 63)
            block[blockSize] = 0x80;
        else
            extraBlock[0] = 0x80;

        int i;
        for( i = 0; i < 8; i++ )
        {
            extraBlock[56+i] = (l >> (56-(8*i)));
        }
    }
    return twoBlocks;
}

void doSha1(uint8_t * block)
{
    static uint32_t w[80] = {0x00000000};
    int i;
    for( i = 0; i < 16; i++ )
    {
        int offset = (i*4);
        w[i] =  block[offset]     << 24 |
                block[offset + 1] << 16 |
                block[offset + 2] << 8  |
                block[offset + 3];
    }

    for( i = 16; i < 80; i++ )
    {
        uint32_t tmp = (w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]);
        w[i] = rotl( tmp, 1 );
    }

    uint32_t a = H[0];
    uint32_t b = H[1];
    uint32_t c = H[2];
    uint32_t d = H[3];
    uint32_t e = H[4];

    for( i = 0; i < 80; i++ )
    {
        uint32_t tmp = rotl(a, 5) + roundFunc(b,c,d,i) + e + w[i] + kForRound(i);
        e = d;
        d = c;
        c = rotl(b, 30);
        b = a;
        a = tmp;
//        printf("%d: %x, %x, %x, %x, %x\n", i, a, b, c,d,e);
    }

    H[0] = H[0] + a;
    H[1] = H[1] + b;
    H[2] = H[2] + c;
    H[3] = H[3] + d;
    H[4] = H[4] + e;
}


int main( int argc, char **argv )
{
    if( argc == 2 )
    {
        char * fileName = argv[1];
        //Read the input file 512 bits at a time = 64 bytes = 16 32-bit words
        FILE * file = fopen( fileName, "rb" );
        int fileSize = 0;
        if( file != NULL )
        {
            uint8_t *block = malloc(64);
            size_t readSize = fread( block, 1, 64, file );
            fileSize += readSize;
            while( readSize == 64 )
            {
                doSha1(block);
                readSize = fread( block, 1, 64, file );
                fileSize += readSize;
            }
            //We need padding
            uint8_t *extraBlock = malloc(64);
            int i;
            for(i = readSize; i < 64; i++)
            {
                block[i] = 0x00;
                extraBlock[i] = 0x00;
            }
            int twoBlocks = pad(block, extraBlock, readSize, fileSize);
            doSha1(block);
            if(twoBlocks == 1)
            {
                doSha1(extraBlock);
            }

            for( i = 0; i < 5; i++)
            {
                printf("%08x",H[i]);
            }
            printf("\n");
        }
        else
        {
            printf("Input file must exist.\n");
        }
    }
    else
    {
        printf("Usage is sha1 [filename]\n");
    }
}


