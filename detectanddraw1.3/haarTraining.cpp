#include "haarTraining.h"
#include <vector>
#include <fstream>
#include"tinyxml2.h"
using namespace tinyxml2;
#include<string>
#include <ctype.h>
#include <math.h>
#include"myIntergal.h"
#include"delete.h"
#include <omp.h>
#include <time.h>
#include"RectLike.h"
using namespace std;



/*
*
* define xml name
*/
#define WEAK "weak"
#define HAARFEATUR "haarfeature"
#define ERRORER "error"
#define LEFT "left"
#define RIGHT "right"
#define THRESHOLD "threshold"
#define NUMBER "number"
#define DESC "desc"
#define RECT "rect"
#define RECT_0 "rect_0"
#define RECT_1 "rect_1"
#define RECT_2 "rect_2"
#define TITLED "titled"
#define BOUNDER_IGNORE 4       //忽略的边界像素
#define POS_FLAG 1
#define NEG_FLAG 2
/*
* get sum image offsets for <rect> corner points
* step - row step (measured in image pixels!) of sum image
*/
#define CV_SUM_OFFSETS( p0, p1, p2, p3, rect, step )                      \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x + w, y) */                                                      \
    (p1) = (rect).x + (rect).width + (step) * (rect).y;                   \
    /* (x , y+h) */                                                      \
    (p2) = (rect).x + (step) * ((rect).y + (rect).height);                \
    /* (x + w, y + h) */                                                  \
    (p3) = (rect).x + (rect).width + (step) * ((rect).y + (rect).height);

/*
* get tilted image offsets for <rect> corner points
* step - row step (measured in image pixels!) of tilted image
*/
#define CV_TILTED_OFFSETS( p0, p1, p2, p3, rect, step )                   \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x - h, y + h) */                                                  \
    (p1) = (rect).x - (rect).height + (step) * ((rect).y + (rect).height);\
    /* (x + w, y + w) */                                                  \
    (p2) = (rect).x + (rect).width + (step) * ((rect).y + (rect).width);  \
    /* (x + w - h, y + w + h) */                                          \
    (p3) = (rect).x + (rect).width - (rect).height                        \
           + (step) * ((rect).y + (rect).width + (rect).height);

/*
* icvCreateIntHaarFeatures
*
* Create internal representation of haar features
*
* mode:
*  0 - BASIC = Viola
*  1 - CORE  = All upright
*  2 - ALL   = All features
*/
static
CvIntHaarFeatures* icvCreateIntHaarFeatures(MySize winsize,
	int mode,
	int symmetric)
{
	CvIntHaarFeatures* features = NULL;
	CvTHaarFeature haarFeature;

	//CvMemStorage* storage = NULL;
	//CvSeq* seq = NULL;
	//CvSeqWriter writer;

	vector<CvTHaarFeature> seq;

	int s0 = 36; /* minimum total area size of basic haar feature     */
	int s1 = 12; /* minimum total area size of tilted haar features 2 */
	int s2 = 18; /* minimum total area size of tilted haar features 3 */
	int s3 = 24; /* minimum total area size of tilted haar features 4 */

	int x = 0;
	int y = 0;
	int dx = 0;
	int dy = 0;

#if 0
	float factor = 1.0F;

	factor = ((float)winsize.width) * winsize.height / (24 * 24);

	s0 = (int)(s0 * factor);
	s1 = (int)(s1 * factor);
	s2 = (int)(s2 * factor);
	s3 = (int)(s3 * factor);
#else
	s0 = 1;
	s1 = 1;
	s2 = 1;
	s3 = 1;
#endif

	/* CV_VECTOR_CREATE( vec, CvIntHaarFeature, size, maxsize ) */
	//	storage = cvCreateMemStorage();
	//	cvStartWriteSeq(0, sizeof(CvSeq), sizeof(haarFeature), storage, &writer);

	for (x = 0; x < winsize.width; x++)
	{
		for (y = 0; y < winsize.height; y++)
		{
			for (dx = 1; dx <= winsize.width; dx++)
			{
				for (dy = 1; dy <= winsize.height; dy++)
				{
					// haar_x2 //右－左
					if ((x + dx * 2 <= winsize.width) && (y + dy <= winsize.height)) {
						if (dx * 2 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x2",
								x, y, dx * 2, dy, -1,
								x + dx, y, dx, dy, +2);
							/* CV_VECTOR_PUSH( vec, CvIntHaarFeature, haarFeature, size, maxsize, step ) */
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_y2 下减上
					if ((x + dx <= winsize.width) && (y + dy * 2 <= winsize.height)) {
						if (dx * 2 * dy < s0) continue;
						if (!symmetric || (x + x + dx <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_y2",
								x, y, dx, dy * 2, -1,
								x, y + dy, dx, dy, +2);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_x3  中间-两侧 横
					if ((x + dx * 3 <= winsize.width) && (y + dy <= winsize.height)) {
						if (dx * 3 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x3",
								x, y, dx * 3, dy, -1,
								x + dx, y, dx, dy, +3);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_y3 中间-两侧 竖
					if ((x + dx <= winsize.width) && (y + dy * 3 <= winsize.height)) {
						if (dx * 3 * dy < s0) continue;
						if (!symmetric || (x + x + dx <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_y3",
								x, y, dx, dy * 3, -1,
								x, y + dy, dx, dy, +3);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					if (mode != 0 /*BASIC*/) {
						// haar_x4
						if ((x + dx * 4 <= winsize.width) && (y + dy <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 4 <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_x4",
									x, y, dx * 4, dy, -1,
									x + dx, y, dx * 2, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// haar_y4
						if ((x + dx <= winsize.width) && (y + dy * 4 <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_y4",
									x, y, dx, dy * 4, -1,
									x, y + dy, dx, dy * 2, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}
					}

					// x2_y2
					if ((x + dx * 2 <= winsize.width) && (y + dy * 2 <= winsize.height)) {
						if (dx * 4 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x2_y2",
								x, y, dx * 2, dy * 2, -1,
								x, y, dx, dy, +2,
								x + dx, y + dy, dx, dy, +2);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					if (mode != 0 /*BASIC*/) {
						// point
						if ((x + dx * 3 <= winsize.width) && (y + dy * 3 <= winsize.height)) {
							if (dx * 9 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_point",
									x, y, dx * 3, dy * 3, -1,
									x + dx, y + dy, dx, dy, +9);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}
					}

					if (mode == 2 /*ALL*/) {
						// tilted haar_x2                                      (x, y, w, h, b, weight)
						if ((x + 2 * dx <= winsize.width) && (y + 2 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 2 * dy < s1) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x2",
									x, y, dx * 2, dy, -1,
									x, y, dx, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y2                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 2 * dy <= winsize.height) && (x - 2 * dy >= 0)) {
							if (dx * 2 * dy < s1) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y2",
									x, y, dx, 2 * dy, -1,
									x, y, dx, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_x3                                   (x, y, w, h, b, weight)
						if ((x + 3 * dx <= winsize.width) && (y + 3 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 3 * dy < s2) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x3",
									x, y, dx * 3, dy, -1,
									x + dx, y + dx, dx, dy, +3);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y3                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 3 * dy <= winsize.height) && (x - 3 * dy >= 0)) {
							if (dx * 3 * dy < s2) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y3",
									x, y, dx, 3 * dy, -1,
									x - dy, y + dy, dx, dy, +3);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}


						// tilted haar_x4                                   (x, y, w, h, b, weight)
						if ((x + 4 * dx <= winsize.width) && (y + 4 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 4 * dy < s3) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x4",


									x, y, dx * 4, dy, -1,
									x + dx, y + dx, dx * 2, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y4                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 4 * dy <= winsize.height) && (x - 4 * dy >= 0)) {
							if (dx * 4 * dy < s3) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y4",
									x, y, dx, 4 * dy, -1,
									x - dy, y + dy, dx, 2 * dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}


						/*

						// tilted point
						if ( (x+dx*3 <= winsize.width - 1) && (y+dy*3 <= winsize.height - 1) && (x-3*dy>= 0)) {
						if (dx*9*dy < 36) continue;
						if (!symmetric || (x <= (winsize.width / 2) ))  {
						haarFeature = cvHaarFeature( "tilted_haar_point",
						x, y,    dx*3, dy*3, -1,
						x, y+dy, dx  , dy,   +9 );
						CV_WRITE_SEQ_ELEM( haarFeature, writer );
						}
						}
						*/
					}
				}
			}
		}
	}

	//seq = cvEndWriteSeq(&writer);
	features = (CvIntHaarFeatures*)malloc(sizeof(CvIntHaarFeatures) +
		(sizeof(CvTHaarFeature) + sizeof(CvFastHaarFeature)) * seq.size());
	features->feature = (CvTHaarFeature*)(features + 1);
	features->fastfeature = (CvFastHaarFeature*)(features->feature + seq.size());
	features->count = seq.size();
	features->winsize = winsize;
	//	cvCvtSeqToArray(seq, (CvArr*)features->feature);
	for (int i = 0;i < seq.size();i++)
	{
		features->feature[i] = seq[i];
	}
	//	cvReleaseMemStorage(&storage);

	icvConvertToFastHaarFeature(features->feature, features->fastfeature,
		features->count, (winsize.width + 1));

	return features;
}
/*
*加速特征计算
*/
void icvConvertToFastHaarFeature(CvTHaarFeature* haarFeature,
	CvFastHaarFeature* fastHaarFeature,
	int size, int step)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < size; i++)
	{
		fastHaarFeature[i].tilted = haarFeature[i].tilted;
		if (!fastHaarFeature[i].tilted)
		{
			for (j = 0; j < CV_HAAR_FEATURE_MAX; j++)
			{
				fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
				if (fastHaarFeature[i].rect[j].weight == 0.0F)
				{
					break;
				}
				CV_SUM_OFFSETS(fastHaarFeature[i].rect[j].p0,
					fastHaarFeature[i].rect[j].p1,
					fastHaarFeature[i].rect[j].p2,
					fastHaarFeature[i].rect[j].p3,
					haarFeature[i].rect[j].r, step)
			}

		}
		else
		{
			for (j = 0; j < CV_HAAR_FEATURE_MAX; j++)
			{
				fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
				if (fastHaarFeature[i].rect[j].weight == 0.0F)
				{
					break;
				}
				CV_TILTED_OFFSETS(fastHaarFeature[i].rect[j].p0,
					fastHaarFeature[i].rect[j].p1,
					fastHaarFeature[i].rect[j].p2,
					fastHaarFeature[i].rect[j].p3,
					haarFeature[i].rect[j].r, step);
			}
		}
	}
}
/*
* icvCreateHaarTrainingData
*
* Create haar training data used in stage training
*/
static
CvHaarTrainigData* icvCreateHaarTrainingData(MySize winsize, int maxnumsamples)
{
	CvHaarTrainigData* data;
	data = NULL;
	uchar* ptr = NULL;
	size_t datasize = 0;

	datasize = sizeof(CvHaarTrainigData) +
		/* sum and tilted */
		(2 * (winsize.width + 1) * (winsize.height + 1) * sizeof(sum_type) +
			sizeof(float) +      /* normfactor */
			sizeof(float) +      /* cls */
			sizeof(float)        /* weight */
			) * maxnumsamples;
	data = (CvHaarTrainigData*)malloc(datasize);
	memset((void*)data, 0, datasize);

	data->maxnum = maxnumsamples;
	data->winsize = winsize;
	ptr = (uchar*)(data + 1);

	data->sum = myMat(maxnumsamples, (winsize.width + 1) * (winsize.height + 1), ONE_CHANNEL, INT_TYPE, (void*)ptr);
	ptr += sizeof(sum_type) * maxnumsamples * (winsize.width + 1) * (winsize.height + 1);

	data->tilted = myMat(maxnumsamples, (winsize.width + 1) * (winsize.height + 1), ONE_CHANNEL, INT_TYPE, (void*)ptr);
	ptr += sizeof(sum_type) * maxnumsamples * (winsize.width + 1) * (winsize.height + 1);

	data->normfactor = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);
	ptr += sizeof(float) * maxnumsamples;

	data->cls = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);
	ptr += sizeof(float) * maxnumsamples;

	data->weights = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);

	data->valcache = NULL;
	data->idxcache = NULL;

	return data;
}
typedef struct CvBackgroundData
{
	int    count;
	char** filename;
	int    last;
	int    round;
	MySize winsize;
} CvBackgroundData;
/*负图片*/
CvBackgroundData* cvbgdata = NULL;          //记住要释放
/*正图片*/
CvBackgroundData* cvposdata = NULL;      
/*样本获取计数*/
int trainingdata_number = 0;

static
CvBackgroundData* icvCreateBackgroundData(const char* filename,MySize winsize)
{
	CvBackgroundData* data = NULL;

	const char* dir = NULL;
	char full[PATH_MAX];
	char* imgfilename = NULL;
	size_t datasize = 0;
	int    count = 0;
	FILE*  input = NULL;
	char*  tmp = NULL;
	int    len = 0;

	assert(filename != NULL);

	dir = strrchr(filename, '\\');
	if (dir == NULL)
	{
		dir = strrchr(filename, '/');
	}
	if (dir == NULL)
	{
		imgfilename = &(full[0]);
	}
	else
	{
		strncpy(&(full[0]), filename, (dir - filename + 1));
		imgfilename = &(full[(dir - filename + 1)]);
	}

	input = fopen(filename, "r");
	if (input != NULL)
	{
		count = 0;
		datasize = 0;

		/* count */
		while (!feof(input))
		{
			*imgfilename = '\0';
			if (!fgets(imgfilename, PATH_MAX - (int)(imgfilename - full) - 1, input))
				break;
			len = (int)strlen(imgfilename);
			for (; len > 0 && isspace(imgfilename[len - 1]); len--)
				imgfilename[len - 1] = '\0';
			if (len > 0)
			{
				if ((*imgfilename) == '#') continue; /* comment */
				count++;
				datasize += sizeof(char) * (strlen(&(full[0])) + 1);
			}
		}
		if (count > 0)
		{
			//rewind( input );
			fseek(input, 0, SEEK_SET);
			datasize += sizeof(*data) + sizeof(char*) * count;
			data = (CvBackgroundData*)malloc(datasize);
			memset((void*)data, 0, datasize);
			data->count = count;
			data->filename = (char**)(data + 1);
			data->last = 0;
			data->round = 0;
			data->winsize = winsize;
			tmp = (char*)(data->filename + data->count);
			count = 0;
			while (!feof(input))
			{
				*imgfilename = '\0';
				if (!fgets(imgfilename, PATH_MAX - (int)(imgfilename - full) - 1, input))
					break;
 				len = (int)strlen(imgfilename);
				if (len > 0 && imgfilename[len - 1] == '\n')
					imgfilename[len - 1] = 0, len--;
				if (len > 0)
				{
					if ((*imgfilename) == '#') continue; /* comment */
					data->filename[count++] = tmp;
					strcpy(tmp, &(full[0]));
					tmp += strlen(&(full[0])) + 1;
				}
			}
		}
		fclose(input);
	}

	return data;
}
/*
* icvInitBackgroundReaders
*
* Initialize background reading process.
* <cvbgreader> and <cvbgdata> are initialized.
* Must be called before any usage of background
*
* filename - name of background description file
* winsize  - size of images will be obtained from background
*
* return 1 on success, 0 otherwise.
*/
static
int icvInitBackgroundReaders(const char* filename, MySize winsize)
{
	if (cvbgdata == NULL && filename != NULL)
	{
		cvbgdata = icvCreateBackgroundData(filename, winsize);
	}
	return (cvbgdata != NULL);
}
static
int icvInitPostiveReaders(const char* filename, MySize winsize)
{
	if (cvposdata == NULL && filename != NULL)
	{
		cvposdata = icvCreateBackgroundData(filename, winsize);
	}
	return (cvposdata != NULL);
}

static void getPicture(CvHaarTrainingData* training_data, int *number_all, int number, int flag, MySize mysize)
{
	ofstream filePic;
	switch (flag)
	{
	case NEG_FLAG:
	{
		MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
		MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	//  MyMat *tempTitle = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	 //   MyMat *tempSqsum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, DOUBLE_TYPE);//注意最后要释放
		for (int i = 0;i < number;i++)
		{

			int temp = number_all[i];
			tempMat = transMat(tempMat, cvbgdata->filename[temp]);
			filePic.open("E://pic.txt", ios::app);
			filePic <<trainingdata_number<<","<< cvbgdata->filename[temp] << endl;
			filePic.close();
			if (tempMat != nullptr)
			{
				//计算积分图
		//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
				int *address = training_data->sum.data.i;
				GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
				//GetGraySqImage(tempMat->data.ptr, tempSqsum->data.i, mysize.width, mysize.height, tempMat->step);
				//积分图复制到training_data中
				address = trainingdata_number * (mysize.width + 1)*(mysize.height + 1) + address;
				memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
				training_data->cls.data.fl[trainingdata_number] = 0.0;
				trainingdata_number++;
			}

		}
		releaseMyMat(tempMat);
		releaseMyMat(tempSum);
		//	releaseMyMat(tempTitle);
		//releaseMyMat(tempSqsum);
		break;
	}
	case POS_FLAG:
	{
		MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //注意最后要释放	
		MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	//	MyMat *tempTitle = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放
	//	MyMat *tempSqsum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, DOUBLE_TYPE);//注意最后要释放
		for (int i = 0;i < number;i++)
		{
			
			int temp = number_all[i];
			tempMat = transMat(tempMat, cvposdata->filename[temp]);
			filePic.open("E://pic.txt", ios::app);
			filePic << trainingdata_number << "," << cvposdata->filename[temp] << endl;
			filePic.close();
			if (tempMat != nullptr)
			{
				//计算积分图
				//myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
				int *address = training_data->sum.data.i;
				GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
				//积分图复制到training_data中
				address = trainingdata_number * (mysize.width + 1)*(mysize.height + 1) + address;
				memcpy(address, tempSum->data.i,sizeof(int)*(mysize.width+1)*(mysize.height +1));
				training_data->cls.data.fl[trainingdata_number] = 1.0;
				trainingdata_number++;
			}

		}
		releaseMyMat(tempMat);
		releaseMyMat(tempSum);
		filePic.close();
	//	releaseMyMat(tempTitle);
	//	releaseMyMat(tempSqsum);
		break;
	}
	default:
		break;
	}
}
/*
*计算特征值
*/
float cvEvalFastHaarFeature(const CvFastHaarFeature* feature,
	const sum_type* sum, const sum_type* tilted)
{
	const sum_type* img = feature->tilted ? tilted : sum;
	float ret = feature->rect[0].weight*
		(img[feature->rect[0].p0] - img[feature->rect[0].p1] -
			img[feature->rect[0].p2] + img[feature->rect[0].p3]) +
		feature->rect[1].weight*
		(img[feature->rect[1].p0] - img[feature->rect[1].p1] -
			img[feature->rect[1].p2] + img[feature->rect[1].p3]);

	if (feature->rect[2].weight != 0.0f)
		ret += feature->rect[2].weight *
		(img[feature->rect[2].p0] - img[feature->rect[2].p1] -
			img[feature->rect[2].p2] + img[feature->rect[2].p3]);
	return ret;
}
/*
*计算特征值
*/
float cvEvalFastHaarFeature2(const CvFastHaarFeature feature,
	const sum_type* sum, const sum_type* tilted)
{
	const sum_type* img = feature.tilted ? tilted : sum;
	float ret = feature.rect[0].weight*
		(img[feature.rect[0].p0] - img[feature.rect[0].p1] -
			img[feature.rect[0].p2] + img[feature.rect[0].p3]) +
		feature.rect[1].weight*
		(img[feature.rect[1].p0] - img[feature.rect[1].p1] -
			img[feature.rect[1].p2] + img[feature.rect[1].p3]);

	if (feature.rect[2].weight != 0.0f)
		ret += feature.rect[2].weight *
		(img[feature.rect[2].p0] - img[feature.rect[2].p1] -
			img[feature.rect[2].p2] + img[feature.rect[2].p3]);
	return ret;
}
/*
*计算特征值
*/
float cvEvalFastHaarFeature3(const CvFastHaarFeature* feature,
	const sum_type* sum)
{
	const sum_type* img =  sum;
	float ret = feature->rect[0].weight*
		(img[feature->rect[0].p0] - img[feature->rect[0].p1] -
			img[feature->rect[0].p2] + img[feature->rect[0].p3]) +
		feature->rect[1].weight*
		(img[feature->rect[1].p0] - img[feature->rect[1].p1] -
			img[feature->rect[1].p2] + img[feature->rect[1].p3]);

	if (feature->rect[2].weight != 0.0f)
		ret += feature->rect[2].weight *
		(img[feature->rect[2].p0] - img[feature->rect[2].p1] -
			img[feature->rect[2].p2] + img[feature->rect[2].p3]);
	return ret;
}
/*
*预先生成文本
*/
static
void createTxt(const char* featdirname,CvIntHaarFeatures* haarFeatures)
{
	int number = haarFeatures->count;
	char fileName[100];
//	ofstream *file;
//	file = new ofstream[number];
	ofstream file;
	for (int i = 1;i<=number;++i)
	{
		sprintf(fileName, "%s//%d.txt", featdirname,i);//记住更改路径
//		file[i - 1].open(fileName,ios::out);
//		file[i - 1].close();
		file.open(fileName, ios::out);
		file.close();
	}
//	delete []file;

}
/*
*获得特征值，并保存处理
*numprecalculated 内存限制 后续用
*fileOrMem 特征值存放到文件还是内存 0 内存，1文件
*/
static
void icvPrecalculate(int num_samples,CvHaarTrainingData* data, CvIntHaarFeatures* haarFeatures,
	int numprecalculated,int fileOrMem,const char* filedirname)
{
	switch (fileOrMem)
	{
	case SAVE_FEATURE_FILE:
	{
		//生成批量文件
		createTxt(filedirname, haarFeatures);
		//计算特征值
		char fileName[100];
		float val = 0.0;
		//ofstream *file;
		//file = new ofstream[haarFeatures->count];
		ofstream file;
		for (int i = 0; i < num_samples; i++)
		{
			for (int j = 0; j < haarFeatures->count; j++)
			{
				val = cvEvalFastHaarFeature(haarFeatures->fastfeature+j,data->sum.data.i + i * data->sum.width, data->sum.data.i);
				//sprintf(fileName, "F:\\workplace\\visualstudio\\facesource\\testpic\\feat\\%d.txt", j+1); 		
				
				sprintf(fileName, "%s//%d.txt", filedirname, j + 1);
			//	file[j].open(fileName, ios::app);
			//	file[j] << val << endl;
			//	file[j].close();
				file.open(fileName, ios::app);
				file << val << endl;
				file.close();

			}
		}

	//	delete[]file;
		break;
	} 
	case SAVE_FEATURE_MEM:
	{
		
		break;
	}
	default:
		break;
	}
}
/*
*更新权重
*/
static void icvSetWeightsAndClasses(CvHaarTrainingData* training_data,
	int num1, float weight1, float cls1,
	int num2, float weight2, float cls2)
{
	int j;

	assert(num1 + num2 <= training_data->maxnum);

	for (j = 0; j < num1; j++)
	{
		training_data->weights.data.fl[j] = weight1;
		training_data->cls.data.fl[j] = cls1;
	}
	for (j = num1; j < num1 + num2; j++)
	{
		training_data->weights.data.fl[j] = weight2;
		training_data->cls.data.fl[j] = cls2;
	}
}
static 
void saveXML(int stage,vector<MyStumpClassifier> strongClassifier,const char* dirname)
{
	XMLDocument doc;
	MyStumpClassifier weakClassifier;
	// 创建根元素<China>  
	XMLElement* root = doc.NewElement("root");
	doc.InsertEndChild(root);
	char son[100];
	for (int i = 0;i < 200;i++)
	{
		weakClassifier = strongClassifier[i];
		sprintf(son, "%d", i);
		XMLElement* cityElement = doc.NewElement(son);
		cityElement->SetAttribute("compidx", weakClassifier.compidx); // 设置元素属性  
		cityElement->SetAttribute("error", weakClassifier.error); // 设置元素属性  
		cityElement->SetAttribute("left", weakClassifier.left); // 设置元素属性  
		cityElement->SetAttribute("right", weakClassifier.right); // 设置元素属性  
		cityElement->SetAttribute("threshold", weakClassifier.threshold); // 设置元素属性  
		root->InsertEndChild(cityElement);
	}
	char docName[100];
	sprintf(docName,"stage%d.xml",stage);
	doc.SaveFile(docName);
}

/*
*对图片进行预测
*/
static
int* predict(int* preResult, int pictureNum, CvIntHaarFeatures* haarFeatures, CvHaarTrainigData* haarTrainingData, MyCARTClassifier strongClassifier)
{



		/*
		*计算加和 0.5 * (a1 + a2 + a3 +...)
		*/
		int length = strongClassifier.classifier.size();
		float *at = new float[length];
		float sum = 0.0f;
		for (int i = 0;i < length;i++)
		{
			float error = strongClassifier.classifier[i].error;
			float b = error / (1 - error);
			at[i] = log(1 / b);
			sum = sum + at[i];
		}
	
	/*
	*对图片进行预测分类
	*/
	sum = sum * strongClassifier.threshold;
	for (int i = 0;i < pictureNum;i++)
	{
		float val = 0.0f;
		float predictSum = 0.0;
		for (int j = 0;j < length;j++)
		{
			int featureNum = strongClassifier.classifier[j].compidx;
			val = cvEvalFastHaarFeature(haarFeatures->fastfeature + featureNum, haarTrainingData->sum.data.i + i * haarTrainingData->sum.width, haarTrainingData->sum.data.i);
		//	if ((strongClassifier[j].left == 1)&&(val < strongClassifier[j].threshold) && (haarTrainingData->cls.data.fl[i] == strongClassifier[j].left))
			if ((strongClassifier.classifier[j].left == 1) && (val < strongClassifier.classifier[j].threshold))
			{
				predictSum = predictSum + at[j];
				/*
				if (strongClassifier[j].left == 1)
					cout << "i:" << i << ",j:" << j << ",face" << endl;
				else
					cout << "i:" << i << ",j:" << j << ",non-face" << endl;
					*/

			}
			//else if ((strongClassifier[j].right == 1) && (val > strongClassifier[j].threshold) && (haarTrainingData->cls.data.fl[i] == strongClassifier[j].right))
			else if ((strongClassifier.classifier[j].right == 1) && (val > strongClassifier.classifier[j].threshold))
			{
				predictSum = predictSum + at[j];
				/*
				if (strongClassifier[j].left == 1)
					cout << "i:" << i << ",j:" << j << ",face" << endl;
				else
					cout << "i:" << i << ",j:" << j << ",non-face" << endl;
					*/
			}
		}
		if (predictSum >= sum)
		{
			preResult[i] = 1;
		//	cout << "最终face" << endl;
	    }
		else
		{
			preResult[i] = 0;
		//	cout << "最终non-face" << endl;
		}
	}
	delete[]at;
	return preResult;
}
/*
*对图片进行预测
*/
static
int predictSignal(MyMat* pic, MySize size, MyCascadeClassifier classifier)
{
	float scal = MAX(classifier.size.width / size.width * 1.0, classifier.size.height / size.height * 1.0, );
	float h_scale_rate = size.height * 1.0 / classifier.size.height;
	float w_scale_rate = size.width * 1.0 / classifier.size.width;
	int strong_number = classifier.StrongClassifier.size();

	for (int i = 0;i < strong_number;i++)
	{		
		float val = 0.0f;
		float predictSum = 0.0;
		for (int j = 0;j < classifier.StrongClassifier[i].classifier.size();j++)
		{			
			val = cvEvalFastHaarFeature2(classifier.StrongClassifier[i].classifier[j].fasthaarDesc, pic->data.i, pic->data.i);
			if ((classifier.StrongClassifier[i].classifier[j].left == 1) && (val < classifier.StrongClassifier[i].classifier[j].threshold))
			{
				predictSum = predictSum + classifier.StrongClassifier[i].classifier[j].error;
			}
			else if ((classifier.StrongClassifier[i].classifier[j].right == 1) && (val > classifier.StrongClassifier[i].classifier[j].threshold))
			{
				predictSum = predictSum + classifier.StrongClassifier[i].classifier[j].error;
			}
		}
		if (predictSum >= classifier.StrongClassifier[i].threshold)
		{
		
			continue;
		}
		else
		{
			return 0;
		}


	}
	return 1;

}
/*
*对图片进行预测
*/
static
int predictSignalForShrink(CvIntHaarFeatures* haarFeatures, MyMat* pic, MySize size, MyCascadeClassifier classifier)
{
	float scal = MAX(classifier.size.width / size.width * 1.0, classifier.size.height / size.height * 1.0, );
	float h_scale_rate = size.height * 1.0 / classifier.size.height;
	float w_scale_rate = size.width * 1.0 / classifier.size.width;
	int strong_number = classifier.StrongClassifier.size();
	float val = 0.0f;
	float predictSum = 0.0;
	for (int i = 0;i < strong_number;i++)
	{
		/*
		*计算加和 0.5 * (a1 + a2 + a3 +...)
		*/
		int length = classifier.StrongClassifier[i].classifier.size();
		vector<float> at;
		float sum = 0.0f;
		for (int j = 0;j < length;j++)
		{
			float error = classifier.StrongClassifier[i].classifier[j].error;
			float b = error / (1 - error);
			float temp = log(1 / b);
			at.push_back(temp);
			sum = sum + temp;
		}
		sum = sum * classifier.StrongClassifier[i].threshold;
		for (int j = 0;j < classifier.StrongClassifier[i].classifier.size();j++)
		{
			int featureNum = classifier.StrongClassifier[i].classifier[j].compidx;
			

			val = cvEvalFastHaarFeature3(haarFeatures->fastfeature + featureNum, pic->data.i);
			if ((classifier.StrongClassifier[i].classifier[j].left == 1) && (val < classifier.StrongClassifier[i].classifier[j].threshold))
			{
				predictSum = predictSum + at[j];
			}
			else if ((classifier.StrongClassifier[i].classifier[j].right == 1) && (val > classifier.StrongClassifier[i].classifier[j].threshold))
			{
				predictSum = predictSum + at[j];
			}
		}
		if (predictSum >= sum)
		{
			at.clear();
			continue;
		}
		else
		{
			return 0;
		}


	}
	return 1;

}
/*
* 释放空间
*/
static
void icvReleaseHaarTrainingDataCache(CvHaarTrainigData** haarTrainingData)
{
	if (haarTrainingData != NULL && (*haarTrainingData) != NULL)
	{
		if ((*haarTrainingData)->valcache != NULL)
		{
			releaseMyMat((*haarTrainingData)->valcache);
			(*haarTrainingData)->valcache = NULL;
		}
		if ((*haarTrainingData)->idxcache != NULL)
		{
			releaseMyMat((*haarTrainingData)->idxcache);
			(*haarTrainingData)->idxcache = NULL;
		}
	}
}
static
void icvReleaseIntHaarFeatures(CvIntHaarFeatures** intHaarFeatures)
{
	if (intHaarFeatures != NULL && (*intHaarFeatures) != NULL)
	{
		free((*intHaarFeatures));
		(*intHaarFeatures) = NULL;
	}
}

static
void icvReleaseHaarTrainingData(CvHaarTrainigData** haarTrainingData)
{
	if (haarTrainingData != NULL && (*haarTrainingData) != NULL)
	{
		
		icvReleaseHaarTrainingDataCache(haarTrainingData);
		free((*haarTrainingData));
	}
}
static
void icvReleaseBackgroundData(CvBackgroundData** data)
{
	assert(data != NULL && (*data) != NULL);

	free((*data));
}

/*
*读入XML
*/
MyCascadeClassifier readXML(const char* xmlPath, MyCascadeClassifier &classifier)
{
	int stage_number = 0;
	XMLDocument doc;

	/*读文件*/
	if (doc.LoadFile(xmlPath))
	{
		doc.PrintError();
		exit(1);
	}
	// 根元素  
	XMLElement* root = doc.RootElement();
	string size_str;
	XMLElement* size_root = root->FirstChildElement("size");
	size_str = size_root->GetText();
	stringstream input(size_str);
	//读入size
	int number = 0;
	int count = 0;
	while (input >> number)
	{
		if (count)
			classifier.size.width = number;
		else
			classifier.size.height = number;
		count++;
	}
	//遍历所有强分类器
	XMLElement* stage_number_root = root->FirstChildElement("stage_number");
	stage_number = atoi(stage_number_root->GetText());
	char stageName[100];
	for (int i = 0;i < stage_number;i++)
	{
		MyCARTClassifier tempStrong;
		sprintf(stageName, "stage_%d", i);
		//读入第一个强分类器
		XMLElement* strongClasssifer_root = root->FirstChildElement(stageName);
		float th = atof(strongClasssifer_root->Attribute("stage_thresold"));
		tempStrong.threshold = th;
		// 遍历<surface>元素  
		XMLElement* surface = strongClasssifer_root->FirstChildElement("weak");
		while (surface)
		{
			MyStumpClassifier tempWeak;			
			// 遍历子元素  
			XMLElement* surfaceChild = surface->FirstChildElement();
			while (surfaceChild)
			{
				//	遍历fasthaar
				if (strcmp(surfaceChild->Name(), HAARFEATUR) == 0)
				{
				//	tempWeak.compidx = atoi(surfaceChild->GetText());
					XMLElement* surfaceSun = surfaceChild->FirstChildElement();
					while (surfaceSun)
					{
						if (strcmp(surfaceSun->Name(), TITLED) == 0)
						{
							tempWeak.fasthaarDesc.tilted = atoi(surfaceSun->GetText());
					
						}
						else if (strcmp(surfaceSun->Name(), RECT) == 0)
						{
							//遍历xml三个矩形
							XMLElement* surfaceSunSun = surfaceSun->FirstChildElement();
							while (surfaceSunSun)
							{
								if (strcmp(surfaceSunSun->Name(), RECT_0) == 0)
								{
									float point_data;
									int count = 0;
									string point_str;
									point_str = surfaceSunSun->GetText();
									stringstream input_0(point_str);
									while (input_0 >> point_data)
									{
										switch (count)
										{
										case 0:
											tempWeak.fasthaarDesc.rect[0].p0 = point_data;
											break;
										case 1:
											tempWeak.fasthaarDesc.rect[0].p1 = point_data;
											break;
										case 2:
											tempWeak.fasthaarDesc.rect[0].p2 = point_data;
											break;
										case 3:
											tempWeak.fasthaarDesc.rect[0].p3 = point_data;
										case 4:
											tempWeak.fasthaarDesc.rect[0].weight = point_data;
											break;
											break;
										}
										count++;
									}
									
								}
								else if (strcmp(surfaceSunSun->Name(), RECT_1) == 0)
								{
									float point_data;
									int count = 0;
									string point_str;
									point_str = surfaceSunSun->GetText();
									stringstream input_0(point_str);
									while (input_0 >> point_data)
									{
										switch (count)
										{
										case 0:
											tempWeak.fasthaarDesc.rect[1].p0 = point_data;
											break;
										case 1:
											tempWeak.fasthaarDesc.rect[1].p1 = point_data;
											break;
										case 2:
											tempWeak.fasthaarDesc.rect[1].p2 = point_data;
											break;
										case 3:
											tempWeak.fasthaarDesc.rect[1].p3 = point_data;
											break;
										case 4:
											tempWeak.fasthaarDesc.rect[1].weight = point_data;
											break;
										}
										count++;
									}
								}
								else if (strcmp(surfaceSunSun->Name(), RECT_2) == 0)
								{
									float point_data;
									int count = 0;
									string point_str;
									point_str = surfaceSunSun->GetText();
									stringstream input_0(point_str);
									while (input_0 >> point_data)
									{
										switch (count)
										{
										case 0:
											tempWeak.fasthaarDesc.rect[2].p0 = point_data;
											break;
										case 1:
											tempWeak.fasthaarDesc.rect[2].p1 = point_data;
											break;
										case 2:
											tempWeak.fasthaarDesc.rect[2].p2 = point_data;
											break;
										case 3:
											tempWeak.fasthaarDesc.rect[2].p3 = point_data;
											break;
										case 4:
											tempWeak.fasthaarDesc.rect[2].weight = point_data;
											break;
										}
										count++;
									}
								}
								surfaceSunSun = surfaceSunSun->NextSiblingElement();
							}
						
						}
						surfaceSun = surfaceSun->NextSiblingElement();
					}
				}
				else if (strcmp(surfaceChild->Name(), ERRORER) == 0)
				{
					tempWeak.error = atof(surfaceChild->GetText());
				}
				else if (strcmp(surfaceChild->Name(), LEFT) == 0)
				{
					tempWeak.left = atof(surfaceChild->GetText());
				}
				else if (strcmp(surfaceChild->Name(), RIGHT) == 0)
				{
					tempWeak.right = atof(surfaceChild->GetText());
				}
				else if (strcmp(surfaceChild->Name(), THRESHOLD) == 0)
				{
					tempWeak.threshold = atof(surfaceChild->GetText());
				}
				surfaceChild = surfaceChild->NextSiblingElement();
			}
			tempStrong.classifier.push_back(tempWeak);
			surface = surface->NextSiblingElement("weak");
		}
		classifier.StrongClassifier.push_back(tempStrong);
	}
	return classifier;
}
/*
*人脸检测函数,矩形框扩大
*/
FaceSeq* myHaarDetectObjects(MyMat *pic, MyCascadeClassifier classifer, float scale, int neighbor, int type, MySize minSize, MySize maxSize)
{
	FaceSeq *reFaces = NULL;
	vector<MyRect> faces;
	CvIntHaarFeatures* haar_features = NULL;
	MySize pic_size;
	pic_size.width = pic->width;
	pic_size.height = pic->height;

	MySize classifer_size;
	classifer_size.width = classifer.size.width;
	classifer_size.height = classifer.size.height;  //xml检测器最小检测范围

	MySize current_size;
//	MyMat *tempSum = createMyMat(pic_size.height + 1, pic_size.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放,积分图
	 
	haar_features = icvCreateIntHaarFeatures(classifer_size, 0, 1); // 计算haar特征个数
	//对原图像进行复制
	//memcpy(tempPic->data.ptr, pic->data.ptr, sizeof(uchar)*(pic_size.width)*(pic_size.height));
	float scal_width = maxSize.width / minSize.width;
	float scal_height = maxSize.height / minSize.height;
	float max_scal = MIN(scal_width,scal_height);  //保证最大检测框不大于规定
	float current_scal = 1.0;
	if ((minSize.width < classifer_size.width) || (minSize.height < classifer_size.height))  //保证最小检测框大于训练xml的size
		current_size = classifer_size;
	else
		current_size = minSize;

	current_scal = MAX(current_size.width / (1.0 * classifer_size.width), current_size.height / (1.0 * classifer_size.height));
//	MyMat *subWindow = createMyMat(pic_size.height, pic_size.width, ONE_CHANNEL, INT_TYPE);//注意释放
	
	int count = 0;
	//求解积分图
//	GetGrayIntegralImage(pic->data.ptr, tempSum->data.i, pic_size.width, pic_size.height, pic->step);

	//特征框放大
	for (;;current_scal *= scale)
	{
		MyRect tempRect;
		/*
		*计算当前窗口大小
		*/
		tempRect.x = 0;
		tempRect.y = 0;
		tempRect.width = (int)(current_scal * classifer_size.width);
		tempRect.height = (int)(current_scal * classifer_size.height);
		if (tempRect.width > pic_size.width)
		{
			break;
		}
		if (tempRect.height > pic_size.height)
		{
			break;
		}
	//	printf("current scale:%f,%d,%d\n", current_scal, tempRect.width, tempRect.height);
		MySize currentSize;
		currentSize.width = tempRect.width;
		currentSize.height = tempRect.height;
		for (int i = BOUNDER_IGNORE;i < pic_size.height - BOUNDER_IGNORE - tempRect.height;i++)
		{
			for (int j = BOUNDER_IGNORE;j < pic_size.width - BOUNDER_IGNORE - tempRect.width;j++)
			{
				MyMat *subWindow = createMyMat(currentSize.height, currentSize.width, ONE_CHANNEL, UCHAR_TYPE);//注意释放
				MyMat *subWindowSum = createMyMat(currentSize.height + 1, currentSize.width + 1, ONE_CHANNEL, INT_TYPE);//注意释放
				tempRect.x = i;
				tempRect.y = j;  //子窗口图像位置
				for (int kk = 0;kk < tempRect.height;kk++)
				{
					
					memcpy(&subWindow->data.ptr[kk * subWindow->width], &pic->data.ptr[(i+kk)*pic->width+j], sizeof(uchar)*(currentSize.width));
				//	memcpy(&subWindow->data.i[kk * subWindow->width], &tempSum->data.i[(i + kk)*tempSum->width + j], sizeof(int)*(tempRect.width));
					
				}
				//计算积分图
				GetGrayIntegralImage(subWindow->data.ptr, subWindowSum->data.i, subWindow->width, subWindow->height, subWindow->step);
		//		char name[100];
				
		//		sprintf(name,"e:\\res2\\%d.png",count++);
		//		if(count%20 == 0)
		//		imwrite(name,transCvMat(subWindow));
				
				//开始检测
				if (predictSignal(subWindowSum, currentSize, classifer))
					faces.push_back(tempRect);
				releaseMyMat(subWindow);
				releaseMyMat(subWindowSum);
					
			}
		}
		
	
	}
	//转化
	reFaces = (FaceSeq*)malloc(sizeof(int) + sizeof(MyRect) * faces.size());
	reFaces->rect = (MyRect*)(reFaces + 1);
	reFaces->count = faces.size();
	for (int i = 0;i < faces.size();i++)
		reFaces->rect[i] = faces[i];
	 //计算积分图
	icvReleaseIntHaarFeatures(&haar_features);
//	releaseMyMat(tempSum);

//	releaseMyMat(subWindow);
	return reFaces;
}
/*
*多尺度检测
*/

FaceSeq* myHaarDetectObjectsShrink(MyMat *pic, MyCascadeClassifier classifer, float scale, 
	int neighbor, int type, MySize minSize, MySize maxSize)
{

	FaceSeq *reFaces = NULL;
	vector<MyRect> faces;
	MySize pic_size;
	pic_size.width = pic->width;
	pic_size.height = pic->height;
	vector<int> labels;
	MySize classifer_size;
	classifer_size.width = classifer.size.width;
	classifer_size.height = classifer.size.height;  //xml检测器最小检测范围	
	MyMat *tempSum = createMyMat(pic_size.height + 1, pic_size.width + 1, ONE_CHANNEL, INT_TYPE);//注意最后要释放,积分图																
	float scal_width = maxSize.width / classifer_size.width;
	float scal_height = maxSize.height / classifer_size.height;
	float max_scal = MIN(scal_width, scal_height);  //保证最大检测框不大于规定
	float current_scal = max_scal;	
	int count = 0;
	int count2 = 0;
	//图像缩小
	int y_step = 1;
	int x_step = 1;

	for (current_scal = 1.0;;current_scal *= scale)
	{
//		cout << current_scal << endl;
		if (((classifer_size.width * current_scal) > (maxSize.width )) || ((classifer_size.height * current_scal) > (maxSize.height )))
		{

				break;

		} //控制不大于最大
		if (((classifer_size.width * current_scal) < minSize.width) || ((classifer_size.height * current_scal) < minSize.height))
		{
			continue;
		} //控制不小于最小
		int width = pic_size.width / current_scal;   //计算当前图像长宽
		int height = pic_size.height / current_scal;

		MyMat *outPic = createMyMat(height,width,ONE_CHANNEL,UCHAR_TYPE);
		bin_linear_scale(pic, outPic, width, height);  //缩小图像

		
		/*
		char name[100];
		sprintf(name, "e:\\res4\\%d.png", count2++);
		imwrite(name, transCvMat(outPic));
		*/
	//	x_step = (current_scal > 2 ? 1 : 2);
		y_step = (current_scal > 2 ? 1 : 3);



#ifdef _OPENMP
		omp_set_num_threads(omp_get_max_threads());         //开启并行计算
#pragma omp parallel for shared(faces,labels)
#endif // _OPENMP

		for (int i = 0;i < outPic->height - classifer_size.height - 5;i = i + x_step)
		{
			for (int j = 0;j < outPic->width - classifer_size.width - 5;j = j + y_step)
			{
				MyMat *subWindow = createMyMat(classifer_size.height, classifer_size.width, ONE_CHANNEL, UCHAR_TYPE);//注意释放
				MyMat *subWindowSum = createMyMat(classifer_size.height + 1, classifer_size.width + 1, ONE_CHANNEL, INT_TYPE);//注意释放
				//截取子窗口
				//#pragma omp parallel for shared(subWindow,outPic)
				for (int kk = 0;kk < classifer_size.height;kk++)
				{				
					memcpy(&subWindow->data.ptr[kk * subWindow->width], &outPic->data.ptr[(i + kk)*outPic->width + j], sizeof(uchar)*(subWindow->width));
				}	
				GetGrayIntegralImage(subWindow->data.ptr, subWindowSum->data.i, subWindow->width, subWindow->height, subWindow->step);				
				//截取加和子窗口
	
				//开始检测	
				int result = predictSignal(subWindowSum, classifer_size, classifer);		
				#pragma omp critical 
				if (result == 1)
				{
					MyRect tempRect;
					tempRect.x = i * current_scal ;
					tempRect.y = j * current_scal;
					tempRect.width = classifer_size.width * current_scal;
					tempRect.height = classifer_size.height * current_scal;
					faces.push_back(tempRect);

					labels.push_back(count);
					count++;
				}				
				releaseMyMat(subWindow);
				releaseMyMat(subWindowSum);
			}
		}

		releaseMyMat(outPic);
	}
	
	RectLike rLike(0.2);
	vector <MyRect>mergefaces;   //合并后的人脸
	int mecount = rLike.Disjoint_set_merge2(neighbor,faces, mergefaces, labels, rLike);
	//转化
	reFaces = (FaceSeq*)malloc(sizeof(int) + sizeof(MyRect) * mergefaces.size());
	reFaces->rect = (MyRect*)(reFaces + 1);
	reFaces->count = mergefaces.size();
	for (int i = 0;i < mergefaces.size();i++)
		reFaces->rect[i] = mergefaces[i];
   /*
	
	reFaces = (FaceSeq*)malloc(sizeof(int) + sizeof(MyRect) * faces.size());
	reFaces->rect = (MyRect*)(reFaces + 1);
	reFaces->count = faces.size();
	for (int i = 0;i < faces.size();i++)
		reFaces->rect[i] = faces[i];
	*/
	//计算积分图
	//icvReleaseIntHaarFeatures(&haar_features);


	return reFaces;
}