/*****************************************************************************
 * main.c: imgproc API usage example
 *****************************************************************************/
#include <stdio.h>

void main( int argc, char **argv )
{
    printf("test make file");
    int width, height;
    unsigned char *luma_y, *chroma_u, *chroma_v;
    unsigned char srcfile[20] ;
    int i_frame = 1;
    if( !(argc > 1)) printf("Example usage: main 352x288 input.yuv output.yuv\n" );
    if( 2 != sscanf( argv[1], "%dx%d", &width, &height )) printf("resolution not specified or incorrect\n" );

    Filein = fopen (argv[2], "rb");
    if (Filein == NULL) printf ("Unable to open source sequence file!\n");
    int luma_size =  width * height;
    int chroma_size =  luma_size/4;
    for( ;; i_frame++ )
    {
        /* Read input frame */
        if( fread( luma_y, 1, luma_size, Filedin ) != luma_size )
            break;
        if( fread( chroma_u, 1, chroma_size, Filein ) != chroma_size )
            break;
        if( fread( chroma_v, 1, chroma_size, Filein ) != chroma_size )
            break;
        sprintf(srcfile,"%d.yuv", i_frame );
        FILE *myFile = fopen(srcfile,"w");
        fwrite( luma_y,1,luma_size,myFile);
        fwrite( chroma_u,1,chroma_size,myFile);
        fwrite( chroma_v,1,chroma_size,myFile);
        fclose( myFile );
    }
}
