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
    int W = width;
    int H = height;
    printf("%d\n",width );
    printf("%d\n",height );
   
    char maskfile[50];
    char bgfile[50];
    unsigned char *src_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    unsigned char *src_u = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    unsigned char *src_v = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    unsigned char *dst_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    unsigned char *dst_u = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    unsigned char *dst_v = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));

    unsigned char *bg_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
    unsigned char *bg_u = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    unsigned char *bg_v = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH/4*sizeof(unsigned char ));
    // uv can use the same mask as luma
    unsigned char *mask_y = (unsigned char *) malloc(IMG_HEIGHT*IMG_WIDTH*sizeof(unsigned char ));
 
    int luma_size =  width * height;
    int chroma_size =  luma_size/4;
    FILE *Filein = fopen (argv[2], "rb");
    sprintf( bgfile,"/xuhua/bg%d.yuv", width );
    FILE *Filebg = fopen (bgfile, "rb");
    FILE *Fileout = fopen (argv[3], "wb");
    if (Fileout == NULL) printf ("Unable to open output sequence file!\n");
    if (Filein == NULL) printf ("Unable to open source sequence file!\n");
    if (Filebg == NULL) printf ("Unable to open background sequence file!\n");
    int thr = 100;
    printf("before loop");
// loop to process every frame
    int i_frame = 1;
    sprintf(maskfile,"/xuhua/mask960/%d.yuv", i_frame );
    FILE *Filemask = fopen ( maskfile, "rb");
#if 1
    if( fread( src_y, 1, luma_size, Filein ) != luma_size ) printf(" read src_y error");
    if( fread( src_u, 1, chroma_size, Filein ) != chroma_size ) printf(" read src_y error");
    if( fread( src_v, 1, chroma_size, Filein ) != chroma_size ) printf(" read src_y error");
    if( fread( mask_y, 1, luma_size, Filemask ) != luma_size ) printf(" read src_y error");
    if( fread( bg_y, 1, luma_size, Filebg ) != luma_size ) printf(" read src_y error");
    if( fread( bg_u, 1, chroma_size, Filebg ) != chroma_size ) printf(" read src_y error");
    if( fread( bg_v, 1, chroma_size, Filebg ) != chroma_size ) printf(" read src_y error");
    genBinDiff( mask_y , mask_y , width, width, height, thr );
    virtualBG( dst_y,src_y,bg_y, mask_y, W, W, H, 1 ) ;
    // uv component use the same mask as luma
    virtualBG( dst_u,src_u,bg_u, mask_y, W/2, W/2, H/2, 2 ) ;
    virtualBG( dst_v,src_v,bg_v, mask_y ,W/2, W/2, H/2, 2 ) ;
    fwrite( dst_y, 1, luma_size, Fileout );
    fwrite( dst_u, 1, chroma_size, Fileout );
    fwrite( dst_v, 1, chroma_size, Fileout );
    fclose( Filemask ) ;
#endif
//end loop
//release ;clean
    fclose(Filein);
    fclose(Filebg);
    fclose(Fileout);
    free( mask_y );
    free( src_y );
    free( src_u );
    free( src_v );
    free( dst_y );
    free( dst_u );
    free( dst_v );
    free( bg_y );
    free( bg_u );
    free( bg_v );
}
