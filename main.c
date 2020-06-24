/*****************************************************************************
 * main.c: imgproc API usage example
 *****************************************************************************/
#include <stdio.h>
#include "imgproc.h"
/*********************************
* this function can split one yuv file to separate frames, from 1 to total n*um 1.yuv ,2.yuv .....total.yuv
***********************************************/
void splitYUV(char* yuvfile,int width,int height)
{
    unsigned char *luma_y, *chroma_u, *chroma_v;
    unsigned char srcfile[20] ;//output file name
    int i_frame = 1;
    FILE *Filein = fopen (yuvfile, "rb");
    if (Filein == NULL) printf ("Unable to open source sequence file!\n");
    int luma_size =  width * height;
    int chroma_size =  luma_size/4;
    luma_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    chroma_u = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    chroma_v = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    for( ;; i_frame++ )
    {
        /* Read input frame */
        printf("%d\n",i_frame);
        if( fread( luma_y, 1, luma_size, Filein ) != luma_size )
            break;
        if( fread( chroma_u, 1, chroma_size, Filein ) != chroma_size )
            break;
        if( fread( chroma_v, 1, chroma_size, Filein ) != chroma_size )
            break;
        printf("%d\n",i_frame);
        sprintf(srcfile,"%d.yuv", i_frame );
        printf( "%s\n",srcfile );
        FILE *myFile = fopen(srcfile,"w");
        if( myFile == NULL ) printf("open output yuv file %s error \n",srcfile);
        fwrite( luma_y,1,luma_size,myFile);
        fwrite( chroma_u,1,chroma_size,myFile);
        fwrite( chroma_v,1,chroma_size,myFile);
        fclose( myFile );
    }
    free(luma_y);
    free(chroma_u);
    free(chroma_v);
}

void main( int argc, char **argv )
{
    int width, height;
    if( !(argc > 1)) printf("Example usage: main 352x288 input.yuv output.yuv\n" );
    if( 2 != sscanf( argv[1], "%dx%d", &width, &height )) printf("resolution not specified or incorrect\n" );

    printf("%d\n",width );
    printf("%d\n",height );

    unsigned char *src_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    unsigned char *src_u = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    unsigned char *src_v = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    unsigned char *dst_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));

    unsigned char *dst_u = src_u; 
    unsigned char *dst_v = src_v;
 
    int luma_size =  width * height;
    int chroma_size =  luma_size/4;
    FILE *Filein = fopen (argv[2], "rb");
    FILE *Fileout = fopen (argv[3], "wb");
    if (Fileout == NULL) printf ("Unable to open output sequence file!\n");
    if (Filein == NULL) printf ("Unable to open source sequence file!\n");
    int thr = 100;
// loop to process every frame

    if( fread( src_y, 1, luma_size, Filein ) != luma_size ) printf(" read src_y error");
    if( fread( src_u, 1, chroma_size, Filein ) != chroma_size ) printf(" read src_y error");
    if( fread( src_v, 1, chroma_size, Filein ) != chroma_size ) printf(" read src_y error");
    genBinDiff( dst_y , src_y , width, width, height, thr );
    fwrite( dst_y, 1, luma_size, Fileout );
    fwrite( dst_u, 1, chroma_size, Fileout );
    fwrite( dst_v, 1, chroma_size, Fileout );

//end loop
//release ;clean
    fclose(Filein);
    fclose(Fileout);
    free( src_y );
    free( src_u );
    free( src_v );
    free( dst_y );
}
