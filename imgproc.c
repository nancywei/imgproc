

/******************************************************
 * imgproc.c: image processing
 *****************************************************************************
 * Copyright (C) 2005-2019 x264 project
 *  *
 *   * Authors: Loren Merritt <lorenm@u.washington.edu>
 *****************************************************************/
#include "imgproc.h"
#include <stdio.h>


//initialize stack
static void Stackinit(SeqStack_t *s)
{
    s->top = 0;
}

//pop element
static int Stackpop(SeqStack_t* s, pixel_t* e)
{
    if( s->top <= 0 )
    {
        return 0;
    }
    else
    {
        s->top--;
        *e = s->pixelStack[s->top];
        return 1;
    }
}

static int Stackprint(SeqStack_t* s)
{
    pixel_t e;
    if( s->top <= 0 )
    {
        return 0;
    }
    else
    {
       do
       {
           Stackpop(s,&e);
           yajie_print("stackprint x:%d,y:%d\n", e.curX,e.curY);


       }while(s->top);
    }

}
//push element
static int Stackpush(SeqStack_t* s, pixel_t* e)
{
    if( s->top >=STACK_LENGTH )
    {
        return 0;
    }
    else
    {
        s->pixelStack[s->top] = *e;
        s->top++;
        return 1;
    }
}

static int StackNotEmpty(SeqStack_t* s)
{
    if( s->top <= 0 ) 
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*
static int Stacktop(SeqStack_t* s, pixel_t* e)
{
    if( s->top <= 0 )
    {
        return 0;
    }
    else
    {
        *e = s->pixelStack[s->top-1];
        return 1;
    }
}
*/

/*regionPreprocess
performs dilation and erosion*/


/*     After dilation ,connected area is almost face area.
	// connected component analysis(4-component)
	// 
	// 1. begin with a forgeground pixel and push its forground neighbors into a stack;
	// 2. pop the pop pixel on the stack and label it with the same label until the stack is empty
	// 
	//  forground pixel: _binImg(x,y)=1
	//  background pixel: _binImg(x,y) = 0

*/

static void shapeBox(unsigned char* binImg, int width, int height,rect_t* tRoi)
{
 
    int label = 0; //start by 0
 
    int h = height;
    int w = width;
    rect_t* roi = tRoi;
    int smax = 0,dxmin = 0;	
    unsigned char *maskinit = (unsigned char*)malloc(sizeof(unsigned char)*(IMG_HEIGHT*IMG_WIDTH>>4));
    memset(maskinit,0,IMG_HEIGHT*IMG_WIDTH>>4);
    SeqStack_t faceStack;
    SeqStack_t *s = &faceStack;
    for(int y = 0; y < h; y ++)
    {
	    unsigned char *data = binImg + y * w;
	    unsigned char *mask = maskinit + y * w;
	    for(int x = 0; x < w; x ++)
	    {
	        if(*( data + x ) != 0 && *( mask + x ) == 0)
	        {
	            mask[x]=1;
		        Stackinit(s);
		        pixel_t pPixel,cPixel;
		        pixel_t *p = &pPixel;
                pixel_t *curPixel = &cPixel;
		        p->curX = x;
		        p->curY = y;
		        Stackpush(s,p); // pixel position: <i,j>
		        int x0 = x,y0 = y,x1 = x,y1 = y; //init roi area
	    	    while(StackNotEmpty(s))
		        {
		          //get the top pixel on the stack and label it with the same label
		          //Stacktop(s,curPixel);
					
		          //pop the top pixel
		            Stackpop(s,curPixel);
	                int curY = curPixel->curY;
		            int curX = curPixel->curX;
                    unsigned char* cdata = binImg + curY * w + curX;  
                    unsigned char* cmask = maskinit + curY * w + curX;  
		            //finding boundary of roi for getting face location
                    if (curX < x0) x0 = curX;
                    if (curY < y0) y0 = curY;
                    if (curX > x1) x1 = curX;
                    if (curY > y1) y1 = curY;
		            //push the 4-neighbors(foreground pixels)
 
		            if(curX-1 >= 0)
		            {
			            if(*(cdata - 1) != 0 && *(cmask - 1) != 1) //leftpixel
			            {
			                p->curX = curX-1;
			                p->curY = curY;
				
			                Stackpush(s,p);
			                *(cmask - 1) = 1;
			            }
		            }
		            if(curX+1 <= w-1)
		            { 
			            if(*(cdata + 1) != 0 && *(cmask + 1) != 1) //right pixel
			            {
			                p->curX = curX+1;
			                p->curY = curY;
			                Stackpush(s,p);
			                *(cmask + 1) = 1;
			             }
		            }
		            if(curY-1 >= 0)
		            {
			            if(*(cdata - w ) != 0 && *(cmask - w ) != 1) //up pixel
			            {
			                p->curX = curX;
			                p->curY = curY - 1;
			                Stackpush(s,p);
			                *(cmask - w) = 1;
			            }  
		            }
		            if(curY+1 <= h-1)
		            {
			            if(*(cdata + w ) != 0 && *(cmask + w ) != 1) //down pixel
			            {
			                p->curX = curX;
			                p->curY = curY + 1;
			                Stackpush(s,p);
			                *(cmask + w) = 1;
			            } 
		            }
		        }
                if( x0 != x1 && y0 != y1){

                    int square = (y1 - y0) * (x1 - x0);
                    if(square > smax)
                    {
                        smax = square; 
	                    roi[label].x0 = x0<<3; //mapping uv dialation image to Y componet
                        roi[label].y0 = y0<<3;
                        roi[label].x1 = x1<<3;
                        roi[label].y1 = y1<<3;
                        dxmin = abs((x0 + x1)/2 - w/2); //distance near center
                    }
                    else if(square > 200)
                    {
                        //keep the center face
                        int dx  = abs((x0 + x1)/2 - w/2);
                        if (dx < dxmin)
                        {
                            
	                        roi[label].x0 = x0<<3; //mapping uv dialation image to Y componet
                            roi[label].y0 = y0<<3;
                            roi[label].x1 = x1<<3;
                            roi[label].y1 = y1<<3;
                        }
                    }
    /*                yajie_print(2,&(x0),1,1,1,"x0");
                    yajie_print(2,&(y0),1,1,1,"y0");
                    yajie_print(2,&(x1),1,1,1,"x1");
                    yajie_print(2,&(y1),1,1,1,"y1");
                    yajie_print(2,&(square),1,1,1,"label");
                    yajie_print(2,&(label),1,1,1,"label");
                    yajie_print(2,&(roi[0].x0),1,1,1,"x0");
                    yajie_print(2,&(roi[0].y0),1,1,1,"y0");
                    yajie_print(2,&(roi[0].x1),1,1,1,"x1");
                    yajie_print(2,&(roi[0].y1),1,1,1,"y1");*/
                }
	        }
	    }
    }
    free(maskinit);
}

void accumulate(unsigned char* pBitmap,unsigned char* pOutBD , int stride,int width,int height)
{
    int i,j;
    int m,n;
    int nSum,flag;
    int h = height/4;
    int w = width/4;
		
    //unsigned char aOutBD[IMG_HEIGHT/8][IMG_WIDTH/8];
    //unsigned char *pOutBD = &aOutBD[0][0];
/* step 1:accumulate 4x4 rectangle to 1 point for further dilation */
    for(j=0;j<h;j++)
    {
	    for(i= 0;i<w;i++)
	    {
	        nSum=0;
                flag = 0;
	        for (m=0;m<4;m++)
	        {
	            for(n=0;n<4;n++)
		        {
		            if((*(pBitmap+( 4*j + m ) * stride + (4*i+n))) != 0)
		            {
		                nSum ++;
                                flag = 255;//generate bitmap;
		            }
		        }
	        }
		
	        //rectangle4x4
	        pOutBD[j* w + i] = flag;
	    }
    }

}

void drawline(unsigned char* pInput,int stride, int x0, int y0, int x1, int y1)
{
    int ymax = 0,ymin = 0;
    if ( y1 > y0)
    {
         ymax = y1;
         ymin = y0; 
    }
    else
    {
         ymin = y1;
         ymax = y0; 

    }
    if( y0 == y1 ) //draw horizontal line 
    {
        int j = y0 ;
        unsigned char* pCt = pInput + j*stride + x0; //start from (1,1)
	for(int i = x0 ;i < x1; i ++)
        {
           *pCt = 255;
            pCt ++;
        }
    }
    else if (x0 == x1)
    {
        unsigned char* pCt;// = pInput + ymin*stride + x0; //start from (1,1)
	for(int i = ymin ;i < ymax; i ++)
        {
           pCt = pInput + i*stride + x0; //start from (1,1)
           *pCt = 255;
        }

    }
    else if (x0 != x1)
    {
        int ymax = 0,ymin = 0;
        unsigned char* line = NULL;
        if ( y1 > y0)
        {
         ymax = y1;
         ymin = y0; 
        }
        else
        {
         ymin = y1;
         ymax = y0; 

        }
        float ctg0 = (float)(x1 - x0)/(float)(ymax - ymin);
        int x = 0;
        /*yajie_print(2,&x0,1,1,1,"start x0");
        yajie_print(2,&x1,1,1,1,"start x1");
        yajie_print(2,&y1,1,1,1,"end y1");
        yajie_print(2,&y0,1,1,1,"end y0");
        printf("ctg0:%f\n",ctg0);*/
        for(int y = ymax + 1 ; y > ymin - 1 ; y--)
        {
            if ( y1 < y0)
            x = x1 - ((float) (y - ymin)* ctg0) ;
            else
            {
                x = x0 + ((float) (y - ymin)* ctg0) ;

     //           yajie_print(2,&x,1,1,1,"drawline x");
       //         yajie_print(2,&y,1,1,1,"drawline y");
            }
            if( x > 0 && x < stride )
            {
               line = pInput + (y + 1) * stride + x - 1; //start from (1,1)
               *line = 255;
               *(line + 1) = 255;
               *(line - 1) = 255;
               *(line + stride) = 255;
               *(line + stride + 1) = 255;
               *(line + stride - 1) = 255;
               *(line - stride + 1) = 255;
               *(line - stride - 1) = 255;
               *(line - stride ) = 255;
            }
       
        }
    }      
}

void dilation(unsigned char* pInput,unsigned char *pDl, int stride,int w,int h)
{

    for(int j = 1;j < h - 1;j ++)
    {
        unsigned char* pCt = pInput + j*stride + 1; //start from (1,1)
        unsigned char* pDCenter = pDl + j*stride + 1; //start from (1,1)
	    for(int i = 1;i < w - 1;i ++)
	    {
           *pDCenter = *(pCt - stride) & *(pCt + stride) & *(pCt - 1 ) & *(pCt + 1);	
           pCt ++;
           pDCenter ++;
        }
    }	
    return;
}

void erosion(unsigned char* pInput,unsigned char *pEr, int stride,int w,int h)
{

    //yajie_print(2,&stride,1,1,1,"stride");
    //yajie_print(2,&w,1,1,1,"w");
    //yajie_print(2,&h,1,1,1,"h");
    //yajie_print(8,pInput,240,136,1,"pInput");
    for(int j = 1;j < h - 1;j ++)
    {
        unsigned char* pCt = pInput + j*stride + 1; //start from (1,1)
        unsigned char* pECenter = pEr + j*stride + 1; //start from (1,1)
	    for(int i = 1;i < w - 1;i ++)
	    {
            if (* pCt != 0){
                //yajie_print(8,pCt,1,1,1,"pCt");
                *(pECenter) = 255;
                *(pECenter -1) = 255;
                *(pECenter +1) = 255;
                *(pECenter -stride) = 255;
                *(pECenter +stride) = 255;
            }
                pCt ++;
                pECenter ++;
        }
    }	
    //yajie_print(8,pEr,240,136,1,"pEr");
    return;
}
/* pBitmap: binary image containing face location
   pOutBD: accumulate 4x4 rectangle from pBitmap.     
   pDl :   execute dilation to pOutBD which can exclude isolate point */
static void regionPreprocess(int width,int height,unsigned char* pBitmap,unsigned char* pOutBD,unsigned char* pDl)
{
    int i,j;
    int m,n;
    int nSum,flag;
    int h = height>>2;
    int w = width>>2;
		
/* step 1:accumulate 4x4 rectangle to 1 point for further dilation */
    for(j=0;j<h;j++)
    {
	    for(i= 0;i<w;i++)
	    {
	        nSum=0;
            flag = 0;
	        for (m=0;m<4;m++)
	        {
	            for(n=0;n<4;n++)
		        {
		            if((*(pBitmap+(4*j +m)*width+(4*i+n))) != 0)
		            {
		                nSum ++;
                        flag = 255;//generate bitmap;
		            }
		        }
	        }
		
	        //rectangle4x4
	        pOutBD[j* w+i] = flag;
	    }
    }

    /*step2:performs dilation*/
    for(j = 1;j < h - 1;j ++)
    {
        unsigned char* pCt = pOutBD + j*w + 1; //start from (1,1)
        unsigned char* pDCenter = pDl + j*w + 1; //start from (1,1)
	    for(i = 1;i < w - 1;i ++)
	    {
           *pDCenter = *(pCt - w) & *(pCt + w) & *(pCt - 1 ) & *(pCt + 1);	
           pCt ++;
           pDCenter ++;
        }
    }	
    return;
}

#if 0
void dilation(unsigned char* pDl,unsigned char* pInput, int stride, int w , int h )
{
    /*step:performs dilation*/
    for(int j = 1;j < h - 1;j ++)
    {
            unsigned char* pCt = pInput + j*stride + 1; //start from (1,1)
            unsigned char* pDCenter = pDl + j*stride + 1; //start from (1,1)
	    for(int i = 1;i < w - 1;i ++)
	    {
              *pDCenter = *(pCt - stride) & *(pCt + stride) & *(pCt - 1 ) & *( pCt + 1 );	
              pCt ++;
              pDCenter ++;
            }
    }
}
#endif
/*
classify pixels of the input image into skin-color and non-skin-color
X264 UV components is stored in UV12 format
*/
static void classify(unsigned char* pPixelUV,int width,int height,int stride,unsigned char* pBitMap,rect_t *tRoi)
{
    int i,j;
    unsigned char nValCb=0;
    unsigned char nValCr=0;
    int x0 = tRoi[0].x0/2;
    int y0 = tRoi[0].y0/2;
    int x1 = tRoi[0].x1/2;
    int y1 = tRoi[0].y1/2;
/*    yajie_print(2,&width,1,1,1,"classify width");
    yajie_print(2,&height,1,1,1,"classify height");
    yajie_print(2,&x0,1,1,1,"classify x0");
    yajie_print(2,&y0,1,1,1,"classify y0");
    yajie_print(2,&x1,1,1,1,"classify x1");
    yajie_print(2,&y1,1,1,1,"classify y1");*/
    unsigned char* pTempU=pPixelUV + 2*x0;	
    unsigned char* pTempV=pPixelUV + 2*x0 + 1;
    unsigned char* pMap = pBitMap + x0;	
    for(j= y0;j< y1;j++)
    {
    		
		for(i= x0;i < x1 ;i++)
		{
			nValCb = *pTempU;
			nValCr = *pTempV;
			//if((nValCb<=123)&&(nValCb>=110)&&(nValCr<=157)&&(nValCr>=140))
			if((nValCb<=120)&&(nValCb>=97)&&(nValCr<=157)&&(nValCr>=143))
			{
				*pMap = 255;
			}
            pMap ++;
			pTempU += 2;
			pTempV += 2;
		}
                pMap = pBitMap + x0 + j * width;
		pTempU = pPixelUV + 2*x0 + j * stride;
		pTempV = pPixelUV + 2*x0 + j * stride + 1;
    }

    return;
}
//kxk is the accumulate widthxheight of pixel
static void accPreprocess(int width,int height,unsigned char* pBitmap,unsigned char* pOutBD, int out_stride,int out_w,int out_h)
{
    int i,j;
    int m,n;
    int nSum;
    int h = out_h;
    int w = out_w;
    int k = 2;		
    unsigned char flag = 255;//generate bitmap;
/* step 1:accumulate 4x4 rectangle to 1 point for further dilation */
if( width / out_w == 2)
{
// for pEr draw face
    for(j=0;j<h;j++)
    {
	    for(i= 0;i<w;i++)
	    {
	        nSum=0;
                flag = 0;
	        for (m=0 ; m < k; m++)
	        {
	            for( n=0 ; n < k; n++)
		        {
		            if((*(pBitmap+(k*j +m)*width+(k*i+n))) != 0)
		            {
		                nSum ++;
	                        pOutBD[j* w+i] = flag;
		            }
		        }
	        }
		
	        //rectangle4x4
	        //pOutBD[j* w+i] = flag;
	    }
     }
}
else if ( out_w/width == 2)
{
// for plane[0] draw face
    for(j = 0;j < height ;j++)
    {
	    for(i = 0;i < width ;i++)
	    {

		if(((*(pBitmap + j*width + i ))) != 0)

	        {
                    
                    pOutBD[ 2*j * out_stride + 2*i] = flag;
                    pOutBD[(2*j + 1)* out_stride + 2*i] = flag;
                    pOutBD[ 2*j * out_stride + 2*i + 1] = flag;
                    pOutBD[(2*j + 1)* out_stride + 2*i + 1] = flag;

                }
            }
    }

}

}
/*
void drawface(unsigned char* pPixelUV,int width,int height,int stride,unsigned char* pOutBD,rect_t *tRoi,int out_stride, int out_w,int out_h)
{
    unsigned char pBitMap[IMG_WIDTH*IMG_HEIGHT/4];
    memset(pBitMap,0,(IMG_WIDTH*IMG_HEIGHT/4)*sizeof(unsigned char));
    classify(pPixelUV,width,height,stride,pBitMap,tRoi);
    fillHoleInRoi(pBitMap,stride,width,height,tRoi);
    FILE *myFile = fopen("./aBitMap640x360.yuv","wb");
    fwrite(pBitMap,1,640*360,myFile);
    fclose(myFile);

    accPreprocess(width,height,pBitMap,pOutBD,out_stride,out_w,out_h);

    return;
}*/

/*
function:FaceDetection
get the box of face in yuv.
*/
	
void x264_FaceDetection(unsigned char* pPixelUV,int width,int height,int stride,rect_t* roi)
{

    if (width > IMG_WIDTH/2 || height > IMG_HEIGHT/2) // detect image smaller than or equal to 1080p.
	    return;
    unsigned char aBitMap[IMG_WIDTH*IMG_HEIGHT/4];
    unsigned char aOutBD[IMG_HEIGHT/8][IMG_WIDTH/8];
    unsigned char aDilation[IMG_HEIGHT/8][IMG_WIDTH/8];
		
 		
    int iDW= width>>2;
    int iDH= height>>2;
		
    
    memset(aBitMap,0,IMG_WIDTH*IMG_HEIGHT/4);
    //yajie_print(2,&(width),1,1,1,"width");	
    //yajie_print(2,&(height),1,1,1,"height");	
    // classify the skin from U/V.
    classify(pPixelUV,width,height,stride,aBitMap,&roi[0]);
    /*FILE *myFile = fopen("./aBitMap640x360.yuv","wb");
    fwrite(aBitMap,1,640*360,myFile);
    fclose(myFile);*/
    //preprocess the region
    regionPreprocess(width,height,aBitMap,&aOutBD[0][0],&aDilation[0][0]);
    /* 
    FILE *myFile1 = fopen("../dilation120x68.yuv","wb");
    fwrite(&aDilation[0][0],1,120*68,myFile1);
    fclose(myFile1);*/
    //box re-shape to rectangle,and mapping to Y component .
    shapeBox(&aDilation[0][0],iDW,iDH,&roi[0]);
/*
    FILE *myFile2 = fopen("../dilation120x68.yuv","wb");
    fwrite(&aDilation[0][0],1,120*68,myFile2);
    fclose(myFile2);
*/
    return;
		
}

// Er is the edge image after erosion ;
// c is the pixel in face center
//               |r_v       face_height/x
//               |
//           ____|____r_h   face_width/2
//
void connectedge(unsigned char* Er, int width, int height, int x0, int y0, int step, int r_min, int r_max,int ymin)
{
    //int cx = c->x;
    //int cy = c->y;
    pixel_t Edge[100];
    pixel_t right[100];
    int FW = 255;
    int m = 0,n = 0,k=0;
    int w_x = r_min; //horizontal search width
    int w_y = r_max/2; // vertical search width
    int r_x = r_min;
    int y = y0;
    /*yajie_print(2,&(x0),1,1,1,"x0");
    yajie_print(2,&(y0),1,1,1,"y0");
    yajie_print(2,&(r_min),1,1,1,"r_min");
    yajie_print(2,&(r_max),1,1,1,"r_max");*/
    for (int m = 0 ;m < 3*r_max/step && y > ymin; m++ )
    {
        int startx = x0 - r_x - w_x;
        if (startx < 0) startx = 0;
        int endx = x0  ;
        unsigned char* startEr =  Er + y * width + startx ;
        y = y0 - m * step;
        /*yajie_print(2,&(startx),1,1,1,"start x");
        yajie_print(2,&(endx),1,1,1,"end x");
        yajie_print(2,&(y),1,1,1,"search y");*/
       // yajie_print(2,&(m),1,1,1,"m");
        unsigned char flag = 0;
        for (int x = startx; x < endx; x ++ )
        {
            if( *( Er + y * width + x ) != 0 )
            {
              Edge[n].curX = x;
              Edge[n].curY = y;
              flag = 1;
             /* yajie_print(2,&(x),1,1,1,"curX");
              yajie_print(2,&(y),1,1,1,"curY");
              yajie_print(2,&(n),1,1,1,"n");*/
              //yajie_print(2,&(m),1,1,1,"m");
              n++;
              break;
            }
     
        }
        //r_x = r_min * (2*r_max/3 - m * step)/r_max;
        if (r_x < 0) r_x =0;

        int startx1 = x0 + r_x + w_x;
        if(startx1 > width) startx1 = width;
        int endx1 = x0  ;
        //int k = 0;
        /*yajie_print(2,&(startx1),1,1,1,"startx1");
        yajie_print(2,&(endx1),1,1,1,"endx1");
        yajie_print(2,&(y),1,1,1,"search y");*/
        for (int x = startx1; x > endx1; x -- )
        {
            if( *( Er + y * width + x ) != 0 )
            {
              right[k].curX = x;
              right[k].curY = y;
              /*yajie_print(2,&(x),1,1,1,"right x");
              yajie_print(2,&(y),1,1,1,"right y");
              yajie_print(2,&(k),1,1,1,"right k");*/
              k++;
              break;
            }
        }
    }
    m = n;
    //yajie_print(2,&(k),1,1,1,"totalk");
    //yajie_print(2,&(n),1,1,1,"totaln");
    for ( int j = k - 1;j > 0; j--, m++) 
    {
            Edge[m] = right[j];

    }
    //yajie_print(2,&(m),1,1,1,"totalm");
    for (int i = 0; i < (m - 1) ; i++)
    {
     //   yajie_print(2,&( Edge[i].curX ),1,1,1,"edge curX");
     //   yajie_print(2,&( Edge[i].curY ),1,1,1,"edge curY");
//        yajie_print(2,&( Edge[i + 1].curX ),1,1,1,"Edge curX");
 //       yajie_print(2,&( Edge[i + 1].curY ),1,1,1,"Edge curY");*/
        //for (int j = i + 1; i < n; i++)
          //  drawline(Er, width, Edge[i].curX, Edge[i].curY, Edge[j].curX, Edge[j].curY );
        drawline(Er, width, Edge[i].curX, Edge[i].curY, Edge[i+1].curX, Edge[i+1].curY );
        if (Edge[i].curX > Edge[i+1].curX )
        {
           int r = i - 10;
           if (r < 0) r = 0;
           if ( i > n && (r - 10)<n ) r = n;
           drawline(Er, width, Edge[r].curX, Edge[r].curY, Edge[i+1].curX, Edge[i+1].curY );
        }
    }
}

/*
  refine dst: the edge have original resolution
  ladst : laplacian gradient
  ptg:    laplacian theta
  pga:    gassian edge output 
*/
void refineEdge(unsigned char* dst, unsigned char* pEr,int Er_stride, int16_t* pla,int16_t* ptg, unsigned char* pga,int stride,rect_t *roi)
{
   yajie_print(2,&Er_stride,1,1,1,"enter refineEdge");
   int x0 = roi->x0;
   int y0 = roi->y0;
   int x1 = roi->x1;
   int y1 = roi->y1;
   yajie_print(2,&x1,1,1,1,"roi x1 refineEdge");
   unsigned char* dsx, *Er,*la,*tg,ga;

   for ( int y = y0;y < y1; y++){
      Er = pEr + Er_stride*(y/4);
      dsx = dst + stride * y;
      la = pla + stride * y;
      tg = ptg + stride * y;
      ga = pga + stride * y;
      for ( int x = x0;x < x1; x ++){
          if ( *(Er + x/4) != 0)
          {
              yajie_print(8,(ga + x),20,1,1,"gaussian");   
              yajie_print(16,(la + x),20,1,1,"laplacian gradient");   
              yajie_print(16,(tg + x),20,1,1,"laplacian tg");   
          }      
      }
   }
}
void fillhole(unsigned char* binImg, int stride, int width, int height)
{
 
    int label = 0; //start by 0
    int BG = 255;
    //int num = 0; //points number in connected area 
    int h = height;
    int w = stride;
    int wd = width;
    int smax = 0,dxmin = 0;
    feature_t tfeature;	
    unsigned char *maskinit = (unsigned char*)malloc(sizeof(unsigned char)*(IMG_HEIGHT*IMG_WIDTH));
    memset(maskinit,0,IMG_HEIGHT*IMG_WIDTH);
    SeqStack_t faceStack;
    SeqStack_t connectArea; //all points in one connected area
    SeqStack_t *s = &faceStack;
    SeqStack_t *c = &connectArea;
    int Maxhole = w * h /8; 
    //yajie_print(2,&(stride),1,1,1,"fillhole stride");     
 
    for(int y = 0; y < h; y ++)
    {
	    unsigned char *data = binImg + y * w;
	    unsigned char *mask = maskinit + y * w;
	    for(int x = 0; x < wd; x ++)
	    {
	        if(*( data + x ) != BG && *( mask + x ) == 0)
	        {
	                mask[x]=1;
		        Stackinit(s);
		        Stackinit(c);
		        pixel_t pPixel,cPixel;
		        pixel_t *p = &pPixel;
                        pixel_t *curPixel = &cPixel;
		        p->curX = x;
		        p->curY = y;
		        Stackpush(s,p); // pixel position: <i,j>
		        int x0 = x,y0 = y,x1 = x,y1 = y; //init roi area
                        int lefttopx = x,lefttopy = y;//record start left point
	    	    while(StackNotEmpty(s))
		        {
                    
		          //get the top pixel on the stack and label it with the same label
		          //Stacktop(s,curPixel);
					
		          //pop the top pixel
		            Stackpop(s,curPixel);
			    Stackpush(c,curPixel);
	                int curY = curPixel->curY;
		            int curX = curPixel->curX;
                    unsigned char* cdata = binImg + curY * w + curX;  
                    unsigned char* cmask = maskinit + curY * w + curX;  
		            //finding boundary of roi for getting face location
                    if (curX < x0) x0 = curX;
                    if (curY < y0) y0 = curY;
                    if (curX > x1) x1 = curX;
                    if (curY > y1) y1 = curY;
		            //push the 4-neighbors(foreground pixels)
 
		            if(curX-1 >= 0)
		            {
			            if(*(cdata - 1) != BG && *(cmask - 1) != 1) //leftpixel
			            {
			                p->curX = curX-1;
			                p->curY = curY;
				
			                Stackpush(s,p);
			                *(cmask - 1) = 1;
			            }
		            }
		            if(curX+1 <= w-1)
		            { 
			            if(*(cdata + 1) != BG && *(cmask + 1) != 1) //right pixel
			            {
			                p->curX = curX+1;
			                p->curY = curY;
			                Stackpush(s,p);
			                *(cmask + 1) = 1;
			             }
		            }
		            if(curY-1 >= 0)
		            {
			            if(*(cdata - w ) != BG && *(cmask - w ) != 1) //up pixel
			            {
			                p->curX = curX;
			                p->curY = curY - 1;
			                Stackpush(s,p);
			                *(cmask - w) = 1;
			            }  
		            }
		            if(curY+1 <= h-1)
		            {
			            if(*(cdata + w ) != BG && *(cmask + w ) != 1) //down pixel
			            {
			                p->curX = curX;
			                p->curY = curY + 1;
			                Stackpush(s,p);
			                *(cmask + w) = 1;
			            } 
		            }
		        }
                /*if( x0 == x1 && y0 == y1 && type == 0){ //del absolute point
                    *(data + lefttopx) = 0;
                }*/

                //if( abs(x0 - x1)>10 && (y0 - y1)>10 ){
                if( c->top < 20000){ //fill hole in interest area
                    //int square = (y1 - y0) * (x1 - x0);
                    //yajie_print(2,&(c->top),1,1,1,"c->top");
                    pixel_t pixel_hole;
                    pixel_t *p = &pixel_hole;
                    int i = c->top - 1;
                    do{
                        *p = c->pixelStack[i];

                        *(binImg + ( p->curY ) * stride + ( p->curX ) ) = BG;
                    } while( i-- );                    
                            /*tfeature.rect.x0 = x0;
                            tfeature.rect.y0 = y0;
                            tfeature.rect.x1 = x1;
                            tfeature.rect.y1 = y1;
                            
                            tfeature.lefttop.curX = lefttopx;
                            tfeature.lefttop.curY = lefttopy;
                            tfeature.num = num;*/
    /*                yajie_print(2,&(x0),1,1,1,"x0");
                    yajie_print(2,&(y0),1,1,1,"y0");
                    yajie_print(2,&(x1),1,1,1,"x1");
                    yajie_print(2,&(y1),1,1,1,"y1");
                    yajie_print(2,&(square),1,1,1,"label");
                    yajie_print(2,&(label),1,1,1,"label");
                    yajie_print(2,&(roi[0].x0),1,1,1,"x0");
                    yajie_print(2,&(roi[0].y0),1,1,1,"y0");
                    yajie_print(2,&(roi[0].x1),1,1,1,"x1");
                    yajie_print(2,&(roi[0].y1),1,1,1,"y1");*/
                }
	        }
	    }
    }
    free(maskinit);
    return;

}
//input one pixel (x,y) find all connect pixel have the same value as (x,y); output connect area c
//when find hole BG is 0; w is stride of binImg;
static void findOneConnectArea(unsigned char* binImg,int stride,int w,int h, int x,int y, SeqStack_t* c, unsigned char FW,rect_t *roi)
{
                      SeqStack_t roiArea;
                      SeqStack_t *s = &roiArea;
                      unsigned char *maskinit = (unsigned char*)malloc(sizeof(unsigned char)*(IMG_HEIGHT*IMG_WIDTH));
                      memset(maskinit,0,IMG_HEIGHT*IMG_WIDTH * sizeof(unsigned char ));
                        unsigned char* cdata = NULL;  
                        unsigned char* cmask = maskinit + y * stride + x; 
                        unsigned char BG = 255; 
                        *cmask = 1;
                        //*cmask = 1;
		        Stackinit(s);
		        Stackinit(c);
		        pixel_t pPixel,cPixel;
		        pixel_t *p = &pPixel;
		        p->curX = x;
		        p->curY = y;
                        pixel_t *curPixel = &cPixel;
		        Stackpush(s,p); // pixel position: <i,j>
#if 0
                     yajie_print(2,&w,1,1,1,"width");
                     yajie_print(2,&h,1,1,1,"height");
                     yajie_print(2,&x,1,1,1,"start find connect curX");
                     yajie_print(2,&y,1,1,1,"start find connect curY");
#endif
	    	    while(StackNotEmpty(s))
		        {
                    
		          //get the top pixel on the stack and label it with the same label
		          //Stacktop(s,curPixel);
					
		          //pop the top pixel
		            Stackpop(s,curPixel);
			    Stackpush(c,curPixel);
	                    int curY = curPixel->curY;
		            int curX = curPixel->curX;
                            cdata = binImg + curY * stride + curX;  
                            cmask = maskinit + curY * stride + curX;  
		            //neglect area contain boundary of roi ;
                            if (curX <= roi[0].x0 ||curY <= roi[0].y0 ||curX >= roi[0].x1  - 1||curY >= roi[0].y1  - 1 ) 
                            {
		               Stackinit(s);
		               Stackinit(c);
                               break;

                            }
	  	            //push the 4-neighbors(foreground pixels)
                            //yajie_print(2,&curX,1,1,1,"find connect curX");
                            //yajie_print(2,&curY,1,1,1,"find connect curY");
		            if(curX-1 >= 0)
		            {
                             //       yajie_print(8,cmask - 1,1,1,1,"cmask-1");
                              //      yajie_print(8,cdata - 1,1,1,1,"cdata-1");
			            if(*(cdata - 1) == FW && *(cmask - 1) == 0) //leftpixel
			            {
			                p->curX = curX-1;
			                p->curY = curY;
				
			                Stackpush(s,p);
			                *(cmask - 1) = BG;
                                        //yajie_print(2,&(p->curX),1,1,1,"find left x");
                                        //yajie_print(2,&(p->curY),1,1,1,"find left Y");
                                        //*(cdata - 1) = BG;
			            }
		            }
		            if(curX+1 <= w-1)
		            { 
                                  //  yajie_print(8,cmask + 1,1,1,1,"cmask+1");
                                   // yajie_print(8,cdata + 1,1,1,1,"cdata+1");
			            if(*(cdata + 1) == FW && *(cmask + 1) == 0) //right pixel
			            {
			                p->curX = curX+1;
			                p->curY = curY;
			                Stackpush(s,p);
			                *(cmask + 1) = BG;
                                      //  yajie_print(2,&(p->curX),1,1,1,"find right x");
                                       // yajie_print(2,&(p->curY),1,1,1,"find right Y");
                                        //*(cdata + 1) = BG;
			             }
		            }
		            if(curY-1 >= 0)
		            {
			            if(*(cdata - stride ) == FW && *(cmask - stride ) == 0) //up pixel
			            {
			                p->curX = curX;
			                p->curY = curY - 1;
			                Stackpush(s,p);
			                *(cmask - stride) = BG;
                                    //    yajie_print(2,&(p->curX),1,1,1,"find top x");
                                     //   yajie_print(2,&(p->curY),1,1,1,"find top Y");
                                        //*(cdata - stride) = BG;
			            }  
		            }
		            if(curY+1 <= h-1)
		            {
			            if(*(cdata + stride ) == FW && *(cmask + stride ) == 0) //down pixel
			            {
			                p->curX = curX;
			                p->curY = curY + 1;
			                Stackpush(s,p);
			                *(cmask + stride) = BG;
                                        //yajie_print(2,&(p->curX),1,1,1,"find bottom x");
                                        //yajie_print(2,&(p->curY),1,1,1,"find bottom Y");
                                        //*(cdata + stride) = BG;
			            } 
		            }
		        }
#if 0
                     yajie_print(2,&(c->top),1,1,1,"total find connect");
                      
    FILE *myFile06 = fopen("./mask.yuv","w");
    fwrite( maskinit ,1,1344*720,myFile06);
    fclose(myFile06);
    free(maskinit);
#endif
}

static drawhole(SeqStack_t* connectArea,unsigned char* OutBD, int out_stride,unsigned char* V)
{
               if( connectArea->top > 0 )
               {
                    SeqStack_t *c = connectArea;
                    pixel_t pixel_hole;
                    pixel_t *p = &pixel_hole;
                    int i = c->top - 1;
                    do{
                        *p = c->pixelStack[i];

                        *( OutBD + ( p->curY ) * out_stride + ( p->curX ) ) = V;
                    } while( i-- );                    
               } 
}
void  fillHoleInRoi(unsigned char* faceMask , unsigned char* OutBD,int out_stride,int out_w,int out_h, unsigned char* pBitMap,int stride,int w,int h,rect_t *tRoi)
{
    unsigned char* map = NULL;
    unsigned char* out = NULL;
    int x = 0,y = 0; 
    unsigned char* BG = 255;
    unsigned char* FW = 0;
    SeqStack_t connectArea;
    pixel_t e;
    int x0 = tRoi[0].x0;
    int y0 = tRoi[0].y0;
    int x1 = tRoi[0].x1;
    int y1 = tRoi[0].y1;
    /*findOneConnectArea( OutBD, out_stride, out_w, out_h, x, y,&connectArea, FW ,tRoi);
    drawhole(&connectArea,OutBD, out_stride, BG);

    yajie_print(8,OutBD + y * out_stride + x,1,1,1," OutBD value in ConnectArea");
    yajie_print(2,&(connectArea.top),1,1,1," pixel num in ConnectArea");
    do
    { 
                    Stackpop( &connectArea,&e );
                    yajie_print(2,&(e.curX),1,1,1,"X pixel in ConnectArea");
                    yajie_print(2,&(e.curY),1,1,1,"Y pixel in ConnectArea");

     }while(connectArea.top > 0 );*/
int k = out_w/w;
if( k == 4)
{ 
    yajie_print(2,&(x0),1,1,1,"start x0"); 
    yajie_print(2,&(y0),1,1,1,"start y0"); 
    yajie_print(2,&(x1),1,1,1,"start x1"); 

    yajie_print(2,&(y1),1,1,1,"start y1"); 
}
    //int i = 655;
    //int ymin = y0,ymax = y0;
for (int i = x0; i < x1 ; i++)
{
    int flag = 0;
    int ymin = y0,ymax = y0;
    for (int j = y0; j < y1 ; j ++)
    {
        map = pBitMap + stride * j/k;
    //    out = OutBD + out_stride * j;
        if( *( map + i/k ) != 0)
        {
          if(flag == 0 ) ymin = j;
          else if(j > ymax ) ymax = j;
          flag = 1;
          
        }
    }
    int fh = 0; 
    for (int j = ymin ; j < ymax - 1  ; j ++)
    {

        out = OutBD + out_stride * j;
        if( *( out + i ) == 0)
        {
           fh = 1;
           x = i;
           y = j;
           //yajie_print(2,&(x),1,1,1,"find hole x"); 
           //yajie_print(2,&(y),1,1,1,"find hole y"); 
           findOneConnectArea( OutBD, out_stride, out_w, out_h, x, y,&connectArea, FW ,tRoi);
           //if (connectArea.top < 5000 ) 
           drawhole(&connectArea,OutBD, out_stride, BG);
           drawhole(&connectArea,faceMask, out_stride, BG);
        }
    }
    //if (fh == 1)
    //{
     //if( k == 4) yajie_print(2,&(ymin),1,1,1,"ymin"); 
     //if( k == 4) yajie_print(2,&(ymax),1,1,1,"ymax"); 
       /*do
       { 
         Stackpop( &connectArea,&e );
         yajie_print(2,&(e.curX),1,1,1,"X pixel in ConnectArea");
         yajie_print(2,&(e.curY),1,1,1,"Y pixel in ConnectArea");

       }while(connectArea.top > 0 );
      */
                    
    //}
}
#if 0
               //start search plane0
               if( *(out + i - 1) == 0)
               {
                 x = i - 1;
                 y = j;
                 //yajie_print(2,&x,1,1,1,"fill hole start x"); 
                 //yajie_print(2,&y,1,1,1,"fill hole  start y"); 
                 findOneConnectArea( OutBD, out_stride, out_w, out_h, x, y,&connectArea, BG ,tRoi);
                 /*yajie_print(2,&(connectArea.top),1,1,1,"out of findOneConnectArea");
                 do
                 { 
                    Stackpop( &connectArea,&e );
                    yajie_print(2,&(e.curX),1,1,1,"X pixel in ConnectArea");
                    yajie_print(2,&(e.curY),1,1,1,"Y pixel in ConnectArea");

                 }while(connectArea.top > 0 );*/
               
               }
               else if( *(out + i - out_stride) == 0)
               {
                 x = i ;
                 y = j - 1;
                 findOneConnectArea( OutBD, out_stride, out_w, out_h, x, y,&connectArea, BG ,tRoi);

               }
               else if( *(out + i - out_stride - 1) == 0)
               {
                 x = i - 1 ;
                 y = j - 1;
                 findOneConnectArea( OutBD, out_stride, out_w, out_h, x, y,&connectArea, BG ,tRoi);

               }
               else if( *(out + i + out_stride - 1) == 0)
               {
                 x = i - 1 ;
                 y = j + 1;
                 findOneConnectArea( OutBD, out_stride, out_w, out_h, x, y,&connectArea, BG ,tRoi);

               }
               if( connectArea.top > 0 )
               {
                    SeqStack_t *c = &connectArea;
                    pixel_t pixel_hole;
                    pixel_t *p = &pixel_hole;
                    int i = c->top - 1;
                    do{
                        *p = c->pixelStack[i];

                        *( OutBD + ( p->curY ) * out_stride + ( p->curX ) ) = 255;
                    } while( i-- );                    
               } 
 #endif     
}
// This function replace background according to the mask . 
// mask value 255 means select pixel from original image; 
// mask value  0 for select pixel from background image.
// step is 2 when mask double size of ori
// step is 1 when mask same size of ori
void virtualBG(unsigned char* dst,unsigned char* ori,unsigned char* bg,unsigned char* mask ,int stride, int width,int height,int step) 
{
    int w = width;
    
    int mask_stride = stride;
    if (step == 2)
        mask_stride = stride<<1;
    int h = height;
    unsigned char *dsx = dst;
    unsigned char *srx = ori;
    unsigned char *bgx = bg;
    unsigned char *mx = mask;
  //  yajie_print(2,&stride,1,1,1,"stride");
   // yajie_print(2,&w,1,1,1,"w");
   // yajie_print(2,&h,1,1,1,"h");
    for (int j = 0; j < h -1 ; j++)
    {
    //    yajie_print(2,&j,1,1,1,"virtual j\n");
        dsx = dst + j * stride;
        srx = ori + j * stride;
        bgx = bg + j * width;
        mx = mask + j* step * mask_stride;
        for (int i = 0; i < w-1 ; i++)
        {
      //      yajie_print(2,&i,1,1,1,"virtual i\n");
            if ( mx[i * step] != 0)
            {

 //               yajie_print(2,&i,1,1,1,"copy srx\n");
                memcpy (dsx ,srx,1) ;
                //memcpy (dsx + stride ,srx + stride,4) ;
                //memcpy (dsx + 2*stride ,srx + 2*stride,4) ;
                //memcpy (dsx + 3*stride ,srx + 3*stride,4) ;
	   }
           else
           {

//                yajie_print(2,&i,1,1,1,"copy bgx\n");
                memcpy (dsx ,bgx,1) ;
                //memcpy (dsx + stride ,bgx + width,4) ;
                //memcpy (dsx + 2*stride ,bgx + 2*width,4) ;
                //memcpy (dsx + 3*stride ,bgx + 3*width,4) ;

           }
           srx = srx + 1;
           dsx = dsx + 1;     
           bgx = bgx + 1;     
        }
    }
}
// This function replace background according to the mask . 
// mask value 255 means select pixel from original image; 
// mask value  0 for select pixel from background image.
void virtualBG_UV(unsigned char* dst,unsigned char* ori ,unsigned char* bg_u,unsigned char* bg_v,int stride, int width,int height,unsigned char* mask ,int mask_stride,int mask_width,int mask_height ) 
{
/*
    int w = width/2;
    int w_uv = width;
    int h = height/2;
*/
    int w = width;
    int w_uv = width;
    int h = height;
    int m;
    unsigned char *dsx = dst;
    unsigned char *srx = ori;
    unsigned char *bgu = bg_u;
    unsigned char *bgv = bg_v;
    unsigned char *mx = mask;
/*    yajie_print(2,&stride,1,1,1,"stride");
    yajie_print(2,&width,1,1,1,"width");
    yajie_print(2,&height,1,1,1,"height");
    yajie_print(2,&w,1,1,1,"w");
    yajie_print(2,&w_uv,1,1,1,"w_uv");
    yajie_print(2,&h,1,1,1,"h");*/
    for (int j = 0; j < h -1 ; j++)
    {
    //    yajie_print(2,&j,1,1,1,"virtual j\n");
        dsx = dst + j * stride;
        srx = ori + j * stride;
        bgu = bg_u + j * width;
        bgv = bg_v + j * width;
        mx = mask + 2 * j * mask_stride;
        for (int i = 0; i < w-1 ; i++)
        {

            if ( mx[i] == 1)
            {
                   *(dsx)= *bgu ;
                   *(dsx + 1 ) = *bgv ;
    /*             *(dsx + 2 ) = *(bgu + 1);
                   *(dsx + 3 ) = *(bgv + 1);
                   *(dsx + stride )= *(bgu + width);
                   *(dsx + stride + 1 ) = *(bgv + width) ;
                   *(dsx + stride + 2 ) = *(bgu + width + 1);
                   *(dsx + stride + 3 ) = *(bgv + width + 1);*/

            }
            else
            {
                memcpy (dsx ,srx,2) ;
                //memcpy (dsx + stride ,srx + stride,4) ;
            }
               dsx = dsx + 2;
               srx = srx + 2;
               bgu = bgu + 1;
               bgv = bgv + 1;

        }

    }
}


void drawface(unsigned char* pPixelUV,int width,int height,int stride,unsigned char* pOutBD,rect_t *tRoi,int out_stride, int out_w,int out_h)
{
    unsigned char pBitMap[IMG_WIDTH*IMG_HEIGHT/4];
    memset(pBitMap,0,(IMG_WIDTH*IMG_HEIGHT/4)*sizeof(unsigned char));
    classify(pPixelUV,width,height,stride,pBitMap,tRoi);
    accPreprocess(width,height,pBitMap,pOutBD,out_stride,out_w,out_h);

    return;
}

void drawfacefill(unsigned char* faceMask ,unsigned char* pPixelUV,int width,int height,int stride,unsigned char* pOutBD,int out_stride, int out_w,int out_h, rect_t *tRoi)
{
    unsigned char pBitMap[IMG_WIDTH*IMG_HEIGHT/4];
    memset(pBitMap,0,(IMG_WIDTH*IMG_HEIGHT/4)*sizeof(unsigned char));
    classify(pPixelUV,width,height,stride,pBitMap,tRoi);
 /*   FILE *myFile = fopen("./aBitMap640x360.yuv","wb");
    fwrite(pBitMap,1,640*360,myFile);
    fclose(myFile);*/

    accPreprocess(width,height,pBitMap,pOutBD,out_stride,out_w,out_h);
    accPreprocess(width,height,pBitMap,faceMask,out_stride,out_w,out_h);

 //   yajie_print(2,&(out_w),1,1,1,"in draw face");
   /* 
    FILE *myFile05 = fopen("./bfill.yuv","w");
    fwrite( pOutBD ,1,1344*720,myFile05);
    fclose(myFile05);
*/
    //fillHoleInRoi(faceMask,pOutBD,out_stride, out_w ,out_h ,pBitMap,width,width,height,tRoi);
/*
    FILE *myFile04 = fopen("./afill.yuv","w");
    fwrite( pOutBD ,1,1344*720,myFile04);
    fclose(myFile04);*/
    return;
}

// generate diff Mask as plane0^
void genDiff( unsigned char* diffMask ,unsigned char* pIn,int out_stride, int out_w,int out_h )
{
    unsigned char* in = NULL, *diff = NULL;
 
    for (int j = 0; j < out_h -1 ; j++)
    {
        in = pIn + j * out_stride ;
        diff = diffMask + j * out_stride ;
        for (int i = 0; i < out_w -1 ; i++)
        {
           if( *( in + i) == 0)
               *(diff + i) = 255;
           else
               *(diff + i) = 0;
        }

    }
}

// retain pixel y component value > 120  
static void genEdge( unsigned char* EdgeMask ,unsigned char* Input,int out_stride, int out_w,int out_h,int value )
{
    unsigned char* in = NULL, *edge = NULL;
 
    for (int j = 0; j < out_h -1 ; j++)
    {
        in = Input + j * out_stride ;
        edge = EdgeMask + j * out_stride ;
        for (int i = 0; i < out_w -1 ; i++)
        {
           if( *(in) >= 120 && *edge != 0)
               *(edge) = 255;
           else
               *(edge) = 0;
           in ++;
           edge ++; 
        }

    }
}
// diffMask = Input2 & Input1^
void genORDiff( unsigned char* diffMask ,unsigned char* Input1,unsigned char* Input2,int out_stride, int out_w,int out_h )
{
    unsigned char* in1 = NULL, *in2 = NULL, *diff = NULL;
 
    for (int j = 0; j < out_h -1 ; j++)
    {
        in1 = Input1 + j * out_stride ;
        in2 = Input2 + j * out_stride ;
        diff = diffMask + j * out_stride ;
        for (int i = 0; i < out_w -1 ; i++)
        {
           if( *( in1 ) != 0 && *(in2) == 0 )
               *(diff) = 255;
           else
               *(diff) = 0;
           in1 ++;
           in2 ++;
           diff ++; 
        }

    }
}

// pInput is the edge image; diffMask is the erosion conditon image;
// pOutput is the backgroud bin image
void conditionErosion( unsigned char* pOutput ,unsigned char* pInput,int stride, int w,int h )
{
    drawline( pOutput,stride,0,1,w-1,1 );
    for(int j = 1;j < h - 2;j ++)
    {
        unsigned char* pCt = pInput + j*stride + 1; //start from (1,1)
        unsigned char* pECenter = pOutput + j*stride + 1; //start from (1,1)
	    for(int i = 1;i < w - 1;i ++)
	    {
            //yajie_print(8,pECenter,1,1,1,"output" ); 
            //yajie_print(8,pCt,1,1,1,"input" ); 
               if (* pECenter != 0 )
               {
                
                if (* pCt == 0 )
                //yajie_print(8,pCt,1,1,1,"pCt");
                    *(pECenter) = 255;
                if (* (pCt - 1) == 0 )
                    *(pECenter -1) = 255;
                if (* (pCt + 1) == 0 )
                    *(pECenter +1) = 255;
                if (* (pCt - stride) == 0 )
                    *(pECenter - stride) = 255;
                if (*( pCt - stride - 1) == 0 )
                    *(pECenter - stride - 1) = 255;
                if (* (pCt - stride + 1) == 0 )
                    *(pECenter - stride + 1) = 255;
                if (* (pCt + stride) == 0 )
                    *(pECenter + stride) = 255;
                if (* (pCt + stride - 1) == 0 )
                    *(pECenter + stride - 1 ) = 255;
                if (* (pCt + stride + 1) == 0 )
                    *(pECenter + stride + 1 ) = 255;
            
              }
                pCt ++;
                pECenter ++;
           }
    }

    unsigned char* pCt = NULL,*pECenter = NULL;

    for(int j = h - 2;j > 0;j--)
    {
        unsigned char* pCtline = pInput + j*stride ; //start from (1,1)
        unsigned char* pECenterline = pOutput + j*stride ; //start from (1,1)
	    for(int i = w - 2;i > 0;i -- )
	    {
                pCt = pCtline + i;
                pECenter = pECenterline + i;
            //yajie_print(8,pECenter,1,1,1,"output" ); 
            //yajie_print(8,pCt,1,1,1,"input" ); 
               if (* pECenter != 0 )
               {
                
                if (* pCt == 0 )
                //yajie_print(8,pCt,1,1,1,"pCt");
                    *(pECenter) = 255;
                if (* (pCt - 1) == 0 )
                    *(pECenter -1) = 255;
                if (* (pCt + 1) == 0 )
                    *(pECenter +1) = 255;
                if (* (pCt - stride) == 0 )
                    *(pECenter - stride) = 255;
                if (*( pCt - stride - 1) == 0 )
                    *(pECenter - stride - 1) = 255;
                if (* (pCt - stride + 1) == 0 )
                    *(pECenter - stride + 1) = 255;
                if (* (pCt + stride) == 0 )
                    *(pECenter + stride) = 255;
                if (* (pCt + stride - 1) == 0 )
                    *(pECenter + stride - 1 ) = 255;
                if (* (pCt + stride + 1) == 0 )
                    *(pECenter + stride + 1 ) = 255;
            
              }
           }
     }
}
/*
static inline double interpolate(int v00, int v10, int v01, int v11, double rh, double rv)
{
    //bilinear
        return v00*(1.0-rh)*(1.0-rv) + v10*rh*(1.0-rv) + v01*(1.0-rh)*rv + v11*rh*rv;
}
  */  
static inline int interpolate(int v00, int v10, int v01, int v11, int rh, int rv)
{
          //bilinear
        return (v00*(256-rh)*(256-rv) + v10*rh*(256-rv) + v01*(256-rh)*rv + v11*rh*rv) >> 16;
    //
}
// slim face or elarge eye
// x0,y0 is the center points of the circle
//
void warp( unsigned char* dst,unsigned char* src, int stride,int w,int h,int r ,int x0, int y0 , int x1, int y1)
{

// x,y is the aim picutre; ori_x, ori_y is the location on original pic
    yajie_print(2,&stride,1,1,1,"stride");
    yajie_print(2,&r,1,1,1,"radius");
    yajie_print(2,&x0,1,1,1,"x0");
    yajie_print(2,&y0,1,1,1,"y0");
    yajie_print(2,&x1,1,1,1,"x1");
    yajie_print(2,&y1,1,1,1,"y1");
    int src_linesize[3];
    src_linesize[0] = stride; 
    float s = 1.0;
    int r2 = r * r;
    int vx = ( x0 - x1 ) ;
    int vy = ( y0 - y1 ) ;
    int dv = vx * vx + vy * vy;
    yajie_print(2,&r2,1,1,1,"radias");
    //yajie_print(2,&mv2,1,1,1,"mv2");
    int d = 0,cx2 = 0,cy2 = 0,xx = 0,yy = 0; //distance to center x0,y0
    float yf = 0.0,xf = 0.0,ratio = 0.0;
    unsigned char* pic = NULL;
    unsigned char* src_data[3];
    src_data[0] = src;
    float k = 3.0;
    int enhance = 1.0; 
    for ( int j = y0 - r ;(j < y0 + r) && (j< h); j ++)
    {
        int cy2 = (j - y0) * (j - y0);
        pic = dst + j * stride;
        for ( int i = x0 - r ; i < x0 + r ; i ++)
        {
            cx2 = ( i - x0 ) * ( i - x0 );
            d = cx2 + cy2;
            //yajie_print(2,&d,1,1,1,"distance");
            //isSkin = *(pic + j * stride + i);
            int y = 0,x = 0;
            if ( d < r2 )
            { // for pixel in the circle
               /*yajie_print(2,&d,1,1,1,"d");
               yajie_print(2,&dv,1,1,1,"dv");
               yajie_print(2,&r2,1,1,1,"r2");*/
               enhance = sqrt((double)d)/10.0;
               if ( enhance < 1.0 ) enhance = 1.0; 
               ratio = (float)(r2 - d)/ (float)(r2 - d +  enhance* dv);
               if(ratio < k) k = ratio; 
               /*printf("d:%d\n",d);
               printf("r2:%d\n",r2);
               printf("dv:%d\n",dv);
               printf("ratio:%f\n",ratio);*/
              /* ratio = ratio / (ratio + s * (float) mv2);
               yajie_print(2,&ratio,1,1,1,"ratio");
               yajie_print(2,&vx,1,1,1,"vx");
               yajie_print(2,&vy,1,1,1,"vy"); */
               xf = (float)i + ratio * ratio * (float)vx;
               yf = (float)j + ratio * ratio * (float)vy;

               xx = (int) xf ;
               yy = (int) yf ;
               float rh = xf - xx;
               float rv = yf - yy;
               *(pic + i)  = (int)interpolate(*(src_data[0] + yy * src_linesize[0] + xx),
                                   *(src_data[0] + yy * src_linesize[0] + xx + 1),
                                   *(src_data[0] + (yy + 1) * src_linesize[0] + xx),
                                   *(src_data[0] + (yy + 1) * src_linesize[0] + xx + 1),
                                   rh, rv);
               /*printf("i:%d",i);
               printf("j:%d",j);
               printf("xx:%d",xx);
               printf("yy:%d \n",yy);*/
               //yajie_print(2,&xx,1,1,1,"location x");
               //yajie_print(2,&yy,1,1,1,"location y");
               
            }
            else
            {
               *(pic + i) = *(src_data[0] + j * src_linesize[0] + i);
            }
        }

    }

    //yajie_print(2,&k,1,1,1,"min ratio k");
    //printf("%f\n",k);
}
#define Clip1(a)            ((a)>255?255:((a)<0?0:(a)))
#define Clip1_10bit(a)      ((a)>1023? 1023 : ((a)<0?0:(a)))
#define pel_t unsigned char
void subsample( pel_t *pDstBuffer, int iDstWidth, int iDstHeight, pel_t *pSrcBuffer,int iSrcWidth, int iSrcHeight, int iBitLength)
{
    int i, j, iLineWidth, iSrcLineWidth;
    int jSrc;
    // imgY_org_buffer = img->imgY_org_buffer;
    for (i = 1; i< iDstHeight-1; i++) 
    {
        iLineWidth = i* iDstWidth;
        iSrcLineWidth = i*2*iSrcWidth;
        for(j=1;j<iDstWidth-1; j++) 
        {
            jSrc = 2*j;
            pDstBuffer[iLineWidth+j] = (4*pSrcBuffer[iSrcLineWidth+ jSrc] + pSrcBuffer[iSrcLineWidth+ jSrc-1] +
                pSrcBuffer[iSrcLineWidth+ jSrc+1] + pSrcBuffer[iSrcLineWidth+ jSrc - iSrcWidth]
                + pSrcBuffer[iSrcLineWidth+ jSrc + iSrcWidth] + 4)/8;
            pDstBuffer[iLineWidth+j] = iBitLength < 10? Clip1(pDstBuffer[iLineWidth+j]):
            Clip1_10bit(pDstBuffer[iLineWidth+j]);
        }
    }
    for (i = 0; i< iDstHeight; i++) 
    {
        pDstBuffer[i*iDstWidth] = pSrcBuffer[i*2*iSrcWidth];
        pDstBuffer[i*iDstWidth + iDstWidth-1] = pSrcBuffer[i*2*iSrcWidth + iSrcWidth-1];
    }
    for (i = 1; i< iDstWidth-1; i++) 
    {
        pDstBuffer[i] = pSrcBuffer[i*2];
        pDstBuffer[i + iDstWidth * (iDstHeight-1)] = pSrcBuffer[i*2 + iSrcWidth * (iSrcHeight-1)];
    }
}


/*
*******************************************************************************
* \feature
*    Upsampling Base layer MB for Enhancement layer prediction
*******************************************************************************
*/
void Up_Picture_Sample( pel_t **imgY_UpsampledTmp, int iDstWidth, int iDstHeight, pel_t **imgY_tmp,int iSrcWidth, int iSrcHeight, int iBitLength)
{
    int i, j, iLineWidth, iSrcLineWidth;
    int jSrc;
    
    for (i=0;i<iSrcHeight; i++) 
    { // same location, no interpolate
        for(j=0; j< iSrcWidth; j++) 
        {
            imgY_UpsampledTmp[i*2][j*2] =  imgY_tmp[i][j];
        }
    }
    for (i=0;i<iSrcHeight; i++) 
    { // half pel, x direction
        for(j=1; j<iSrcWidth-2; j++) 
        {
            imgY_UpsampledTmp[i*2][j*2+1] =  iBitLength< 10? Clip1((-imgY_tmp[i][j-1]+ 9*imgY_tmp[i][j] +
                9*imgY_tmp[i][j+1] - imgY_tmp[i][j+2] + 8)/16): 
            Clip1_10bit((-imgY_tmp[i][j-1]+ 9*imgY_tmp[i][j] +
                9*imgY_tmp[i][j+1] - imgY_tmp[i][j+2] + 8)/16);
        }
        imgY_UpsampledTmp[i*2][0*2+1] = (imgY_tmp[i][0] + imgY_tmp[i][1] + 1) /2;
        imgY_UpsampledTmp[i*2][(iSrcWidth-2)*2+1] = (imgY_tmp[i][iSrcWidth-2] + imgY_tmp[i][iSrcWidth-1] + 1) /2;
        imgY_UpsampledTmp[i*2][(iSrcWidth-1)*2+1] = imgY_tmp[i][iSrcWidth-1];
    }
    
    for(j=0; j<iSrcWidth; j++) 
    { // half pel, y direction
        for (i=1;i<iSrcHeight-2; i++) 
        { 
            imgY_UpsampledTmp[i*2+1][j*2] =  iBitLength< 10? Clip1((-imgY_tmp[i-1][j]+ 9*imgY_tmp[i][j] +
                9*imgY_tmp[i+1][j] - imgY_tmp[i+2][j] + 8)/16):
            Clip1_10bit((-imgY_tmp[i-1][j]+ 9*imgY_tmp[i][j] +
                9*imgY_tmp[i+1][j] - imgY_tmp[i+2][j] + 8)/16);
        }
        imgY_UpsampledTmp[0*2+1][2*j] = Clip1((imgY_tmp[0][j] + imgY_tmp[1][j] + 1) /2);
        imgY_UpsampledTmp[(iSrcHeight-2)*2+1][j*2] = (imgY_tmp[iSrcHeight-2][i] + imgY_tmp[iSrcHeight-1][j] + 1) /2;
        imgY_UpsampledTmp[(iSrcHeight-1)*2+1][j*2] = imgY_tmp[iSrcHeight-1][i];
    }
    
    for (i=1;i<iDstHeight; i+=2) 
    { // half pel, xand y  direction
        for(j=3; j<iDstWidth-3; j+=2) 
        {
            imgY_UpsampledTmp[i][j] =  iBitLength< 10?Clip1((-imgY_UpsampledTmp[i][j-3]+ 9*imgY_UpsampledTmp[i][j-1] +
                9*imgY_UpsampledTmp[i][j+1] - imgY_UpsampledTmp[i][j+3] + 8)/16): 
            Clip1_10bit((-imgY_UpsampledTmp[i][j-3]+ 9*imgY_UpsampledTmp[i][j-1] +
                9*imgY_UpsampledTmp[i][j+1] - imgY_UpsampledTmp[i][j+3] + 8)/16);
        }
        imgY_UpsampledTmp[i][1] = (imgY_UpsampledTmp[i][0] + imgY_UpsampledTmp[i][2] + 1) /2;
        imgY_UpsampledTmp[i][iDstWidth-3] = (imgY_UpsampledTmp[i][iDstWidth-4] + imgY_UpsampledTmp[i][iDstWidth-2] + 1) /2;
        imgY_UpsampledTmp[i][iDstWidth-1] = imgY_UpsampledTmp[i][iDstWidth-2];
    }
    
}



