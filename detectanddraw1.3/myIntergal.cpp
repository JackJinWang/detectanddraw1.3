#pragma once
#include"myIntergal.h"
#include"basic.h"
#include"basicStruct.h"
#include<omp.h>
#include<assert.h>

/*
*求解积分图
*/
void GetGrayIntegralImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride)
{
	int *ColSum = (int *)calloc(Width, sizeof(int));        //    用的calloc函数哦，自动内存清0
	memset(Integral, 0, (Width + 1) * sizeof(int));
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		int *LinePL = Integral + Y * (Width + 1) + 1;
		int *LinePD = Integral + (Y + 1) * (Width + 1) + 1;
		LinePD[-1] = 0;
		for (int X = 0; X < Width; X++)
		{
			ColSum[X] += LinePS[X];
			LinePD[X] = LinePD[X - 1] + ColSum[X];
		}
	}
	free(ColSum);
}
//求解平方积分图
void GetGraySqImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride)
{
	uchar *copy = new uchar[Width*Height];
	for (int i = 0;i < Width;i++)
	{
		for (int j = 0;j < Height;j++)
		{
			copy[j + i * Width] = Src[j + i * Width] * Src[j + i * Width];
		}
	}
	GetGrayIntegralImage(copy,Integral,Width,Height,Stride);
	delete []copy;
}
/*
*冒泡排序
*idx 索引数组
*data_array 排序数组
*/
void bubbleSort(int* pData, int *idx, int length)
{
	int temp, idxt;
	for (int i = 0;i != length;++i)
	{
		for (int j = 0; j != length; ++j)
		{
			if (pData[i] < pData[j])
			{
				temp = pData[i];
				idxt = idx[i];
				pData[i] = pData[j];
				idx[i] = idx[j];
				pData[j] = temp;
				idx[j] = idxt;
			}
		}
	}
}
//最近邻内插算法
MyMat* scale(MyMat* input_img, MyMat* output_img, int width, int height)
{

	float h_scale_rate = (float)input_img->rows / height;  //高的比例
	float w_scale_rate = (float)input_img->cols / width;  //宽的比例
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int i_scale = h_scale_rate * i;   //依照高的比例计算原图相应坐标中的x，这里采用的是向下取整，当然四舍五入也可以
			int j_scale = w_scale_rate * j;  //依照宽的比例计算原图相应坐标中的y
											 //cout << "i_scale: " << i_scale <<" j_scale: "<< j_scale << endl;

			output_img->data.ptr[i * width + j] = input_img->data.ptr[i_scale*input_img->width + j_scale];
		}
	}

	return output_img;
}
//双线性插值
//f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)  
uchar get_scale_value(MyMat* input_img, float raw_i, float raw_j)
{
	int i = raw_i;
	int j = raw_j;
	float u = raw_i - i;
	float v = raw_j - j;
	int width = input_img->width;
	int height = input_img->height;
	//注意处理边界问题，容易越界
	if (i + 1 >= input_img->rows || j + 1 >= input_img->cols)
	{
		uchar p = input_img->data.ptr[i * input_img->width + j];
		return p;
	}

	uchar x1 = input_img->data.ptr[i*width + j];  //f(i,j)
	uchar x2 = input_img->data.ptr[i*width + j + 1];  //f(i,j+1)
	uchar x3 = input_img->data.ptr[(i + 1) * width + j];   //(i+1,j)
	uchar x4 = input_img->data.ptr[(i + 1) * width + (j + 1)];  //f(i+1,j+1) 
																// printf("%d %d\n", i, j);
	return ((1 - u)*(1 - v)*x1 + (1 - u)*v*x2 + u*(1 - v)*x3 + u*v*x4);
}
//双线性插值
MyMat* bin_linear_scale(MyMat* input_img, MyMat* output_img, int width, int height)
{

	float h_scale_rate = (float)input_img->rows / height;  //高的比例
	float w_scale_rate = (float)input_img->cols / width;  //宽的比例
#ifdef _OPENMP
	omp_set_num_threads(omp_get_max_threads());         //开启并行计算
#pragma omp parallel for shared(output_img)
#endif // _OPENMP
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float i_scale = h_scale_rate * i;   //依照高的比例计算原图相应坐标中的x，这里采用的是向下取整，当然四舍五入也可以
			float j_scale = w_scale_rate * j;  //依照宽的比例计算原图相应坐标中的y
											   //cout << "i_scale: " << i_scale <<" j_scale: "<< j_scale << endl;

			output_img->data.ptr[i * width + j] = get_scale_value(input_img, i_scale, j_scale);
		}
	}

	return output_img;
}
/*
*并查集窗口合并，超过p，百分比合并
*/
bool judge(double p, MyRect r1, MyRect r2)
{
	//方法1，opencv hog,Cascade使用的合并方法  
	
	double delta = p*(MIN(r1.width, r2.width) + MAX(r1.height, r2.height))*0.5;
	return std::abs(r1.x - r2.x) <= delta &&
		std::abs(r1.y - r2.y) <= delta &&
		std::abs(r1.x + r1.width - r2.x - r2.width) <= delta &&
		std::abs(r1.y + r1.height - r2.y - r2.height) <= delta;
	/*	
	int distance = (r1.width)*0.3;

	return r2.x <= r1.x + distance &&
		r2.x >= r1.x - distance &&
		r2.y <= r1.y + distance &&
		r2.y >= r1.y - distance &&
		r2.width <= (int)(r1.width * 1.5) &&
		(int)(r2.width * 1.5) >= r1.width;
		*/
}

// This function splits the input sequence or set into one or more equivalence classes and
// returns the vector of labels - 0-based class indexes for each element.
// predicate(a,b) returns true if the two sequence elements certainly belong to the same class.
//
// The algorithm is described in "Introduction to Algorithms"
// by Cormen, Leiserson and Rivest, the chapter "Data structures for disjoint sets"
int myPartition(const vector<MyRect>& _vec, vector<int>& labels,double p)
{
	int i, j, N = (int)_vec.size();
	const MyRect* vec = &_vec[0];

	const int PARENT = 0;
	const int RANK = 1;

	vector<int> _nodes(N * 2);
	int(*nodes)[2] = (int(*)[2])&_nodes[0];

	// The first O(N) pass: create N single-vertex trees
	for (i = 0; i < N; i++)
	{
		nodes[i][PARENT] = -1;
		nodes[i][RANK] = 0;
	}

	// The main O(N^2) pass: merge connected components
	for (i = 0; i < N; i++)
	{
		int root = i;

		// find root
		while (nodes[root][PARENT] >= 0)
			root = nodes[root][PARENT];

		for (j = 0; j < N; j++)
		{
			if (i == j || !judge(p, vec[i], vec[j]))
				continue;
			int root2 = j;

			while (nodes[root2][PARENT] >= 0)
				root2 = nodes[root2][PARENT];

			if (root2 != root)
			{
				// unite both trees
				int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
				if (rank > rank2)
					nodes[root2][PARENT] = root;
				else
				{
					nodes[root][PARENT] = root2;
					nodes[root2][RANK] += rank == rank2;
					root = root2;
				}
				assert(nodes[root][PARENT] < 0);

				int k = j, parent;

				// compress the path from node2 to root
				while ((parent = nodes[k][PARENT]) >= 0)
				{
					nodes[k][PARENT] = root;
					k = parent;
				}

				// compress the path from node to root
				k = i;
				while ((parent = nodes[k][PARENT]) >= 0)
				{
					nodes[k][PARENT] = root;
					k = parent;
				}
			}
		}
	}

	// Final O(N) pass: enumerate classes
	labels.resize(N);
	int nclasses = 0;

	for (i = 0; i < N; i++)
	{
		int root = i;
		while (nodes[root][PARENT] >= 0)
			root = nodes[root][PARENT];
		// re-use the rank as the class label
		if (nodes[root][RANK] >= 0)
			nodes[root][RANK] = ~nclasses++;
		labels[i] = ~nodes[root][RANK];
	}

	return nclasses;
}
bool isOverlap(const MyRect &rc1, const MyRect &rc2)
{
	if (rc1.x + rc1.width  > rc2.x &&
		rc2.x + rc2.width  > rc1.x &&
		rc1.y + rc1.height > rc2.y &&
		rc2.y + rc2.height > rc1.y
		)
		return true;
	else
		return false;
}