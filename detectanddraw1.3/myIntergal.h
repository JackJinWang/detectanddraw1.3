#pragma once
#include"MyMat.h"
#include"basicStruct.h"
void myIntegral(MyMat *src, MyMat *sum, MyMat *sqsum, MyMat *tilted);
void GetGrayIntegralImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride); //������ͼ
void GetGraySqImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride); //���ƽ������ͼ
void bubbleSort(int* pData, int *idx, int length);
/*
*ͼ�����ź���
*/
uchar get_scale_value(MyMat* input_img, float raw_i, float raw_j);
MyMat* scale(MyMat* input_img, MyMat* output_img, int width, int height);  //���ڽ��㷨
MyMat* bin_linear_scale(MyMat* input_img, MyMat* output_img, int width, int height);  //˫���Բ�ֵ
/*
*���鼯���ںϲ�������p���ٷֱȺϲ�
*/
bool isOverlap(const MyRect &rc1, const MyRect &rc2); //�ж��غ�
bool judge(double p, MyRect r1, MyRect r2);
int myPartition(const vector<MyRect>& _vec, vector<int>& labels,double p);