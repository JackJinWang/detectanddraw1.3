#include<iostream>
#include<fstream>
#include<time.h>
#include"haarTraining.h"
#include"myIntergal.h"
#include"delete.h"

using namespace std;


static MyCascadeClassifier cascade;

//const char* cascade_name = "F:\\workplace\\visualstudio\\facesource\\testpic\\jilianre\\cascade.xml";

//const char* cascade_name = "F:\\workplace\\visualstudio\\facesource\\testpic\\1.7dt\\cascade.xml";
//const char* cascade_name = "F:\\workplace\\test\\result\\2.1dt2\\cascade.xml";  //可用
const char* cascade_name = "E:\\1.7dt\\cascade.xml";
//const char* cascade_name = "F:\\workplace\\visualstudio\\facesource\\testpic\\20180424_10001000\\casde.xml";
//人脸检测要用到的分类器  
void detect_and_draw(MyMat *img, Mat showpic,  float scale);
int main(int argc, char* argv[])
{
	cascade = readXML(cascade_name, cascade);
	cout << cascade.StrongClassifier.size() << endl;
	//图片检测
	
	Mat picMat = imread("e:\\61.jpg");
	MyMat *outpic = createMyMat(picMat.rows, picMat.cols, ONE_CHANNEL, UCHAR_TYPE);
	//bin_linear_scale(img, outpic, 450, 300);
	outpic = transMatAndSmooth(outpic, "e:\\61.jpg");
	float scale = 1.0;
	if (outpic == nullptr)
	{
		cout << "图片不存在" << endl;
		return 0;
	}
	detect_and_draw(outpic, picMat,scale);
	waitKey();
	
	/*
	Mat frame;
	Mat grayImage, cannyImage;
	bool stop = true;

	VideoCapture capture(0);//打开摄像头  
	if (!capture.isOpened())
	{
		cout << "读摄像头有误" << endl;
		return -1;
	}
	while (true)
	{
		capture >> frame;//读取当前帧到frame矩阵中  
		cvtColor(frame, grayImage, CV_BGR2GRAY);//转为灰度图
		float scale = 2;
		Mat small_pic;
		resize(grayImage, small_pic, Size(grayImage.cols / scale, grayImage.rows / scale), 0, 0, INTER_LINEAR);
		MyMat *outpic = createMyMat(small_pic.rows, small_pic.cols, ONE_CHANNEL, UCHAR_TYPE);
		//bin_linear_scale(img, outpic, 450, 300);
		outpic = transMatAndSmooth(outpic, small_pic);
		if (outpic == nullptr)
		{
			cout << "图片不存在" << endl;
			return 0;
		}
		detect_and_draw(outpic, frame, scale);
		//imshow("result", frame);
		if (waitKey(100) >= 0)
		{
			cout << "触发" << endl;
			break;
		}
			
	}
	*/
	
	
	return 0;
}

void detect_and_draw(MyMat *img,Mat showpic,float scale)
{
	double start, end;
	ofstream filePic;
	FaceSeq *faces = NULL;	
	if (img == nullptr)
	{
		cout << "图片不存在" << endl;
		return;
	}
	MySize minSize;
	minSize.width = 19;
	minSize.height = 19;
	MySize maxSize;
//	maxSize.width = img->width;
//	maxSize.height = img->height;
	maxSize.width = 800 > img->width ? img->width:800;
	maxSize.height = 800 > img->height ? img->height:800;
	start = clock();
	faces = myHaarDetectObjectsShrink(img, cascade, 1.2,6, 0, minSize, maxSize);
	end = clock();
	cout << "耗时：" << (end - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
	for (int i = 0; i <faces->count; i++)
	{
		Rect r;
		r.x = faces->rect[i].y * scale;
		r.y = faces->rect[i].x* scale;
		r.width = faces->rect[i].width* scale;
		r.height = faces->rect[i].height* scale;
		rectangle(showpic, r, Scalar(0, 0, 255),8);
	}
	imshow("result", showpic);
//	waitKey();
	free(faces);
	releaseMyMat(img);

}
/*
void detect_and_draw(IplImage* img)
{
static CvScalar colors[] =
{
{ { 0,0,255 } },
{ { 0,128,255 } },
{ { 0,255,255 } },
{ { 0,255,0 } },
{ { 255,128,0 } },
{ { 255,255,0 } },
{ { 255,0,0 } },
{ { 255,0,255 } }
};
double scale = 1.3;
IplImage* gray = cvCreateImage(cvSize(img->width, img->height), 8, 1);

IplImage* small_img = cvCreateImage(cvSize(
cvRound(img->width / scale), cvRound(img->height / scale)), 8, 1);

cvCvtColor(img, gray, CV_BGR2GRAY);
cvResize(gray, small_img, CV_INTER_LINEAR);
cvEqualizeHist(small_img, small_img);
cvClearMemStorage(storage);

if (cascade)
{

CvSeq* faces = cvHaarDetectObjects(
small_img, cascade, storage, 1.1, 2, 0, cvSize(
20, 20));

for (int i = 0; i < (faces ? faces->total : 0); i++)
{
CvRect* r = (CvRect*)cvGetSeqElem(faces, i);
CvPoint center;
int radius;
center.x = cvRound((r->x + r->width*0.5)*scale);
center.y = cvRound((r->y + r->height*0.5)*scale);
radius = cvRound((r->width + r->height)*0.25*scale);
cvCircle(img, center, radius, colors[i % 8], 3, 8, 0);
}
}
cvShowImage("result", img);
cvReleaseImage(&gray);
cvReleaseImage(&small_img);
}
*/