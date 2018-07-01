/*
*
* 未脱离opencv的函数，最终必须删除
*
*/
#pragma once
#include"basic.h"
#include "MyMat.h"
#include <omp.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
using namespace cv;
inline MyMat* transMat(MyMat * result,char *picName)
{
	Mat img = imread(picName, CV_LOAD_IMAGE_GRAYSCALE);
	int height = img.size().height;
	int width = img.size().width;
	if(img.empty())
	{
		return nullptr;
	}

	uchar *temp;
	for (int i = 0; i < height; i++)
	{
		// get the pointer to the ith row
		temp = img.ptr<uchar>(i);
		// operates on each pixel
		for (int j = 0; j < width; j++)
		{
			// assigns new value
			result->data.ptr[i*width+j] = temp[j];
		}
	}
	return result;
}
inline MyMat* transMatAndSmooth(MyMat * result, char *picName)
{
	Mat img = imread(picName, CV_LOAD_IMAGE_GRAYSCALE);
	int height = img.size().height;
	int width = img.size().width;
	if (img.empty())
	{
		return nullptr;
	}
	Mat out;
	blur(img, out, Size(3, 3));
	//equalizeHist(out, out);
	uchar *temp;
#ifdef _OPENMP
	omp_set_num_threads(THREAD_NUMBER);         //开启并行计算
#pragma omp parallel for
#endif // _OPENMP
	for (int i = 0; i < height; i++)
	{
		// get the pointer to the ith row
		temp = out.ptr<uchar>(i);
		// operates on each pixel
		for (int j = 0; j < width; j++)
		{
			// assigns new value
			result->data.ptr[i*width + j] = temp[j];
		}
	}
	return result;
}
inline MyMat* transMatAndSmooth(MyMat * result, Mat img)
{
	if (img.empty())
	{
		return nullptr;
	}
	int height = img.size().height;
	int width = img.size().width;

	Mat out;
	blur(img, out, Size(3, 3));
	//equalizeHist(out, out);
	uchar *temp;
#ifdef _OPENMP
	omp_set_num_threads(THREAD_NUMBER);         //开启并行计算
#pragma omp parallel for
#endif // _OPENMP
	for (int i = 0; i < height; i++)
	{
		// get the pointer to the ith row
		temp = out.ptr<uchar>(i);
		// operates on each pixel
		for (int j = 0; j < width; j++)
		{
			// assigns new value
			result->data.ptr[i*width + j] = temp[j];
		}
	}
	return result;
}
inline Mat transCvMat(MyMat * result)
{
	Mat imgMat(result->height, result->width, CV_8UC1);
	int height = result->height;
	int width = result->width;


	uchar *temp;
	for (int i = 0; i < height; i++)
	{
		// operates on each pixel
		for (int j = 0; j < width; j++)
		{
			imgMat.at<uchar>(i, j) = result->data.ptr[i*width + j];
		}
	}
	return imgMat;
}

