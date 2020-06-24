/*****************************************************************************
 * main.c: imgproc API usage example
 *****************************************************************************/
#include <stdio.h>
#include "imgproc.h"
#define IMG_HEIGHT 1080
#define IMG_WIDTH  1920
void main( int argc, char **argv )
{
    int width, height;
    unsigned char *luma_y, *chroma_u, *chroma_v;
    unsigned char srcfile[20] ;
    int i_frame = 1;
    if( !(argc > 1)) printf("Example usage: main 352x288 input.yuv output.yuv\n" );
    if( 2 != sscanf( argv[1], "%dx%d", &width, &height )) printf("resolution not specified or incorrect\n" );

    printf("%d\n",width );
    printf("%d\n",height );
    FILE *Filein = fopen (argv[2], "rb");
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
