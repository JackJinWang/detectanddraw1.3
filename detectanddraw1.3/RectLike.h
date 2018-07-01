#pragma once
#include"basic.h"
#include "basicStruct.h"
#include"myIntergal.h"
#include<vector>
#include<iostream>
using namespace std;
class  RectLike {

public:
	double p;//公共区域所占的百分比，超过该比例则合并  
	float start_x, start_y, end_x, end_y;//合并后公共区域的起始点  
	float w, h;//r1和r2公共区域的长，宽  

public:
	RectLike();
	RectLike(double p);
	void overlap(MyRect &r1, MyRect &r2);
	float overlap_area(MyRect r1, MyRect r2);//判断框出图像的重叠面积  
	bool operator()(MyRect r1, MyRect r2);
	MyRect merge(MyRect r1, MyRect r2);
	int Disjoint_set_merge(int min_neighber,vector<MyRect> MyRects, vector<MyRect>& merge_rects, vector<int>& labels, RectLike rLike);
	int Disjoint_set_merge2(int min_neighber, vector<MyRect> MyRects, vector<MyRect>& merge_rects, vector<int>& labels, RectLike rLike);
	//并查集方法，只现实了区域的合并  
};

RectLike::RectLike()
{
	start_x = 0;
	end_x = 0;
	start_y = 0;
	end_y = 0;
	p = 0.6;
};
RectLike::RectLike(double p)
{
	this->p = p;
}
void RectLike::overlap(MyRect &r1, MyRect &r2)
{
	this->end_x = MAX(r1.x + r1.width, r2.x + r2.width);
	this->start_x = MIN(r1.x, r2.x);
	this->end_y = MAX(r1.y + r1.height, r2.y + r2.height);
	this->start_y = MIN(r1.y, r2.y);
	this->w = r1.width + r2.width - (end_x - start_x);
	this->h = r1.height + r2.height - (end_y - start_y);
}
float RectLike::overlap_area(MyRect r1, MyRect r2)
{//判断框出图像的重叠面积  
	overlap(r1, r2);
	if (w <= 0 || h <= 0)
		return 0;
	else
	{
		float area = this->w*this->h;
		return area;
	}
}
bool RectLike::operator()(MyRect r1, MyRect r2)
{
	//方法1，opencv hog,Cascade使用的合并方法  
	double delta = p*(MIN(r1.width, r2.width) + MAX(r1.height, r2.height))*0.5;
	return std::abs(r1.x - r2.x) <= delta &&
		std::abs(r1.y - r2.y) <= delta &&
		std::abs(r1.x + r1.width - r2.x - r2.width) <= delta &&
		std::abs(r1.y + r1.height - r2.y - r2.height) <= delta;
	/*
	//方法2，求面积的合并方法  
	int area1 = r1.area();
	int area2 = r2.area();
	int area = overlap_area(r1, r2);//相交Rect面积  
	return area1<area2 ? area >= p*area1 : area >= p*area2;
	//方法3，投影法  
	int xlap = 40;int ylap = 40;
	int x1 = r1.x, x2 = r1.x + r1.width, x3 = r2.x, x4 = r2.x + r2.width;
	int y1 = r1.y, y2 = r1.y + r1.height, y3 = r2.y, y4 = r2.y + r2.height;
	if (x1 <= x3)
	{
		if ((x2 <= x3&&x3 - x2 <= xlap) || (x1 <= x3&&x3 <= x2&&x2 <= x4) || (x1 <= x3&&x3 <= x4&&x4 <= x2))
		{
			if ((y4 <= y1&&y1 - y4 <= ylap) || (y2 <= y3&&y3 - y2 <= ylap) || (y1 <= y3&&y3 <= min(y2, y4)) || (y3<y1&&y1 <= min(y2, y4)))
			{
				return true;
			}

		}
	}
	else
	{
		if ((x4 <= x1&&x1 - x4 <= xlap) || (x3 <= x1&&x1 <= x4&&x4 <= x2) || (x3 <= x1&&x1 <= x2&&x2 <= x4))
		{
			if ((y2 <= y3&&y3 - y2 <= ylap) || (y4 <= y1&&y1 - y4 <= ylap) || (y3 <= y1&&y1 <= min(y4, y2)) || (y1<y3&&y3 <= min(y4, y2)))
			{
				return true;
			}
		}
	}
	return false;
	*/
}
MyRect RectLike::merge(MyRect r1, MyRect r2)
{
	overlap(r1, r2);
	MyRect rect = { start_x, start_y, end_x - start_x, end_y - start_y };
	return rect;
}
int RectLike::Disjoint_set_merge(int min_neighber,vector<MyRect> rects, vector<MyRect>& merge_rects, vector<int>& labels, RectLike rLike)
{   //并查集方法，只现实了区域的合并  
	//rects为合并前的矩形框，merge_rects为合并后矩形框，labels为矩形框的标签，p为合并的置信度  
	vector<int>label_temp;
	int * number_labels = new int[labels.size()];
	for (size_t i = 0;i < labels.size();i++)
	{
		label_temp.push_back(1);
		number_labels[i] = 0;
	}
	int count = myPartition(rects, labels, this->p);
	for (size_t i = 0;i < labels.size();i++)
	{
		number_labels[labels[i]]++;
	}
	//如果没有合并，还是返回原始的rects,labels，如果合并了，返回以0开始的labels  
	if (count != labels.size())//两者不相等，说明进行了合并  
	{

		for (size_t i = 0;i<labels.size();i++)
		{
			if (label_temp[i] == 0) continue;
			MyRect rect_temp = rects[i];
			label_temp[i] = 0;
			if (number_labels[i] < min_neighber)
			{
				label_temp[i] = 0;
				continue;
			}
			else
			{
				for (size_t j = i + 1;j < labels.size();j++)
				{
					if (labels[i] == labels[j])
					{
						rect_temp = rLike.merge(rect_temp, rects[j]);
						label_temp[j] = 0;
					}


				}
				merge_rects.push_back(rect_temp);
			}
		}

	}
	return count;
	delete[]number_labels;
}

int RectLike::Disjoint_set_merge2(int min_neighber, vector<MyRect> rects, vector<MyRect>& merge_rects, vector<int>& labels, RectLike rLike)
{   //并查集方法，只现实了区域的合并  
	//rects为合并前的矩形框，merge_rects为合并后矩形框，labels为矩形框的标签，p为合并的置信度  
	MyAvgComp *comps;
	vector<int>label_temp;
	vector<MyAvgComp> faceTemp;
	int * number_labels = new int[labels.size()];
	for (size_t i = 0;i < labels.size();i++)
	{
		label_temp.push_back(1);
		number_labels[i] = 0;
	}
	int count = myPartition(rects, labels, p);
	comps = (MyAvgComp*)malloc((count + 1) * sizeof(comps[0])); // 
	memset(comps, 0, (count + 1) * sizeof(comps[0]));
	// count number of neighbors

	for (int i = 0; i < labels.size(); i++)
	{
		MyRect r1 = rects[i];
		int idx = labels[i];
		assert((unsigned)idx < (unsigned)count);
		comps[idx].neighbors++;
		comps[idx].rect.x += r1.x;
		comps[idx].rect.y += r1.y;
		comps[idx].rect.width += r1.width;
		comps[idx].rect.height += r1.height;
	}
	// calculate average bounding box
	for (int i = 0; i < count; i++)
	{
		int n = comps[i].neighbors;
		if (n >= min_neighber)
		{
			MyAvgComp comp;
			comp.rect.x = (comps[i].rect.x * 2 + n) / (2 * n);
			comp.rect.y = (comps[i].rect.y * 2 + n) / (2 * n);
			comp.rect.width = (comps[i].rect.width * 2 + n) / (2 * n);
			comp.rect.height = (comps[i].rect.height * 2 + n) / (2 * n);
			comp.neighbors = comps[i].neighbors;
			faceTemp.push_back(comp);
		}
	}
	int *flag_merge = new int[faceTemp.size()];
	for (int i = 0;i < faceTemp.size();i++)
	{
		flag_merge[i] = 1;
	}
	for (int i = 0; i < faceTemp.size(); i++)
	{
		MyAvgComp r1 = faceTemp[i];
		int  flag = 1;
		for (int j = 0; j < faceTemp.size(); j++)
		{
			if ((flag_merge[i] == 0) && (flag_merge[j] == 0))
			{
				continue;
			}
			MyAvgComp r2 = faceTemp[j];
			int distance_x = r2.rect.width * 0.5;
			int distance_y = r2.rect.height * 0.5;
			//	distance_x = r2.rect.width;
			//	distance_y = r2.rect.height;
			//int distance = 0;
			//r1 在 r2的范围内
			if ((i != j) &&
				(r1.rect.x >= r2.rect.x - distance_x) &&
				(r1.rect.y >= r2.rect.y - distance_y) &&
				(r1.rect.x + r1.rect.width <= r2.rect.x + r2.rect.width + distance_x) &&
				(r1.rect.y + r1.rect.height <= r2.rect.y + r2.rect.height + distance_y))
			{
				if (r2.neighbors >= MAX(2, r1.neighbors) || r1.neighbors <2)
					flag_merge[i] = 0;
				if (r1.neighbors >= MAX(2, r2.neighbors) || r2.neighbors < 2)
					flag_merge[j] = 0;
			}
		}
		/*
		if (flag)
		{
		merge_rects.push_back(r1.rect);
		}
		*/
	}
	for (int i = 0;i < faceTemp.size();i++)
	{
		if (flag_merge[i] == 1)
			merge_rects.push_back(faceTemp[i].rect);
	}
	/*
	// filter out small face rectangles inside large face rectangles
	for (int i = 0; i < faceTemp.size(); i++)
	{
	MyAvgComp r1 = faceTemp[i];
	int  flag = 1;
	for (int j = 0; j < faceTemp.size(); j++)
	{
	MyAvgComp r2 = faceTemp[j];
	int distance_x = r2.rect.width * 0.5;
	int distance_y = r2.rect.height * 0.5;
	//	distance_x = r2.rect.width;
	//	distance_y = r2.rect.height;
	//int distance = 0;
	if( (i != j) &&
	(r1.rect.x >= r2.rect.x - distance_x) &&
	(r1.rect.y >= r2.rect.y - distance_y)&&
	(r1.rect.x + r1.rect.width <= r2.rect.x + r2.rect.width + distance_x)&&
	(r1.rect.y + r1.rect.height <= r2.rect.y + r2.rect.height + distance_y)&&
	(r2.neighbors >= MAX(1, r1.neighbors) || r1.neighbors < 3))
	{
	flag = 0;
	break;
	}
	}
	if (flag)
	{
	merge_rects.push_back(r1.rect);
	}
	}
	*/
	delete[]flag_merge;
	free(comps);
	delete[] number_labels;
	return count;
}