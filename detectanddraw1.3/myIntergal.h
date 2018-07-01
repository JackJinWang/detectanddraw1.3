#pragma once
#include"MyMat.h"
#include"basicStruct.h"
void myIntegral(MyMat *src, MyMat *sum, MyMat *sqsum, MyMat *tilted);
void GetGrayIntegralImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride); //求解积分图
void GetGraySqImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride); //求解平方积分图
void bubbleSort(int* pData, int *idx, int length);
/*
*图像缩放函数
*/
uchar get_scale_value(MyMat* input_img, float raw_i, float raw_j);
MyMat* scale(MyMat* input_img, MyMat* output_img, int width, int height);  //最邻近算法
MyMat* bin_linear_scale(MyMat* input_img, MyMat* output_img, int width, int height);  //双线性插值
/*
*并查集窗口合并，超过p，百分比合并
*/
bool isOverlap(const MyRect &rc1, const MyRect &rc2); //判断重合
bool judge(double p, MyRect r1, MyRect r2);
int myPartition(const vector<MyRect>& _vec, vector<int>& labels,double p);