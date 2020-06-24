/**********************************
*contain image processing function
***********************************
*Jun.2020
*
* Authors:nancy
*
**********************************/


#define ROI_X0 0
#define ROI_Y0 0
#define ROI_X1 1280
#define ROI_Y1 720

#define FACE_NUM 10
#define IMG_WIDTH 1920
#define IMG_HEIGHT 1088
// ROI box struct
typedef struct rect
{
                int x0;
                int y0;
                int x1;
                int y1;
}rect_t;

#define STACK_LENGTH 90000

typedef struct point
{
    int curY;
    int curX;
		
}pixel_t;

typedef struct feature
{
   pixel_t lefttop;
   rect_t rect;

}feature_t;

typedef struct stack
{
    pixel_t pixelStack[STACK_LENGTH];
    int top;
		
}SeqStack_t;

void genBinDiff( unsigned char* diffMask ,unsigned char* pIn,int out_stride, int out_w,int out_h,int thr );
void vrtualBG(unsigned char* dst,unsigned char* ori,unsigned char* bg,unsigned char* mask ,int stride, int width,int height,int step);
