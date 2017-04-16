#include "Util.h"
#include "DcmFileFormat.h"
#include "DcmFileProcess.h"
#include "ImageConvert.h"

#include "HBmpConvert.h"
#include "HLymphHOG.h"
#include "HCandidate.h"

#include "ximage.h"

#include "dcmtk/config/osconfig.h"  
#include "dcmtk/dcmdata/dctk.h"  
#include "dcmtk/dcmdata/dcpxitem.h"  
#include "dcmtk/dcmjpeg/djdecode.h"  
#include "dcmtk/dcmjpeg/djencode.h" 
#include "dcmtk/dcmjpeg/djcodece.h"  
#include "dcmtk/dcmjpeg/djrplol.h"  
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmdata/dcvrfd.h"
#include "dcmtk/dcmdata/dcistrmf.h"
#include "dcmtk/dcmdata/dcdicdir.h"

#include "../EasyCNN/EasyCNN.h"


using namespace THU_STD_NAMESPACE; // ����DCM��ʽת�������ռ�
using namespace std;

#define DirGenerationDcm false     // dcmȫ�����ݵ��ļ�Ŀ¼����
#define DirGenerationPixel false   // dcm��ʵ�������������ļ�Ŀ¼����
#define DcmToBmp false             // �Ƿ����dcm��bmp�ĸ�ʽת��������һ����ͼ���ʽת�����Ե���CxImage
#define PixelRecords false         // ��������ʵ����������б���
#define CandidateGenerateNeg false  // ���������������ļ��������������ļ�����
#define CandidateGeneratePos false  // ���������������ļ��������������ļ�����
#define WorldCoordinate false


// �ַ���split
void split(const string& src, const string& separator, vector<string>& dest)
{
	string str = src;
	string substring;
	string::size_type start = 0, index;

	do
	{
		index = str.find_first_of(separator, start);
		if (index != string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = str.find_first_not_of(separator, index);
			if (start == string::npos) return;
		}
	} while (index != string::npos);

	//the last token
	substring = str.substr(start);
	dest.push_back(substring);
}



int opencv_main(int argc, char* argv[])
{

	/*
	// ͼ���Ƭ����
	BmpConvert bmpsample = BmpConvert("F:\\lymph node detection dataset\\DOI\\ABD_LYMPH_001\\000367.bmp");
	bmpsample.CroppedEdge(128, 96, 224, 224);
	bmpsample.InfoShow();
	vector<Mat> result = bmpsample.SegmentationBottom( cvSize(9, 9) );
	bmpsample.MultiImageShow("Multiple Images", result, cvSize(9, 9), cvSize(400, 280));
	*/
	//vector<Mat> result = bmpsample.SegmentationTop(cvSize(8, 8));
	//bmpsample.MultiImageShow("Multiple Images", result, cvSize(8, 8), cvSize(400, 280));
	
	  
	// HoGͼ��ѵ��
	//LymphHOG lymphhog = LymphHOG();
	//lymphhog.LabelOutput(false);
	//lymphhog.SVMTraining(false);
	//lymphhog.classify();
	//system("pause");

	// �����ѡ������ļ�����
	if (CandidateGenerateNeg)
	{
		//���������  
		CStatDir statdir;

		char buf[256] = "F:\\lymph node detection dataset\\MED_ABD_LYMPH_CANDIDATES";
		char posfilename[] = "F:\\lymph node detection dataset\\candidateneg.lst";
		ofstream fout(posfilename);
		//����Ҫ������Ŀ¼  
		if (!statdir.SetInitDir(buf))
		{
			puts("Ŀ¼������");
			return 0;
		}

		//��ʼ����  
		vector<string> file_vec = statdir.BeginBrowseFilenames("*negCADe_physicalPoints.txt");
		for (vector<string>::const_iterator it = file_vec.begin(); it < file_vec.end(); ++it)
			fout << *it << endl;

		printf("�ļ�����: %d\n", file_vec.size());  // 176
	}

	// �����ѡ������ļ�����
	if (CandidateGeneratePos)
	{
		//���������  
		CStatDir statdir;

		char buf[256] = "F:\\lymph node detection dataset\\MED_ABD_LYMPH_CANDIDATES";
		char posfilename[] = "F:\\lymph node detection dataset\\candidatepos.lst";
		ofstream fout(posfilename);
		//����Ҫ������Ŀ¼  
		if (!statdir.SetInitDir(buf))
		{
			puts("Ŀ¼������");
			return 0;
		}

		//��ʼ����  
		vector<string> file_vec = statdir.BeginBrowseFilenames("*posCADe_physicalPoints.txt");
		for (vector<string>::const_iterator it = file_vec.begin(); it < file_vec.end(); ++it)
			fout << *it << endl;

		printf("�ļ�����: %d\n", file_vec.size());  // 176
	}

    // �������������ʼ�����ļ�
	if (WorldCoordinate)
	{
		//���������  
		CStatDir statdir;

		char buf[256] = "F:\\lymph node detection dataset\\DOI";
		char filename[] = "F:\\lymph node detection dataset\\DATA\\coordinate.txt";
		ofstream fout(filename);
		//����Ҫ������Ŀ¼  
		if (!statdir.SetInitDir(buf))
		{
			puts("Ŀ¼������");
			return 0;
		}

		//��ʼ����  
		vector<string> file_vec = statdir.BeginBrowseFilenames("*000000.dcm");
		for (vector<string>::const_iterator it = file_vec.begin(); it < file_vec.end(); ++it)
			fout << *it << endl;

		//printf("�ļ�����: %d\n", file_vec.size());  // 176

		string DcmFileName;
		ifstream finFile("F:\\lymph node detection dataset\\DATA\\coordinate.txt");

		while (getline(finFile, DcmFileName))
		{
			// ʵ����dcm������
			TDcmFileFormat dcm = TDcmFileFormat(DcmFileName.c_str());
			// ��ȡdcmͼ���ImagePositionPatient
			string result = dcm.getImagePositionPatient();
			vector<string> ImagePosition;
			::split(result, "\\", ImagePosition);
			double value0 = atof(ImagePosition[0].c_str());
			double value1 = atof(ImagePosition[1].c_str());
			double value2 = atof(ImagePosition[2].c_str());
			cout << value0 << " " << value1 << " " << value2 << endl;

			// ��ȡdcmͼ���PixelSpacing
			string result1 = dcm.getPixelSpacing();
			vector<string> PixelSpacing;
			::split(result1, "\\", PixelSpacing);
			double value3 = atof(PixelSpacing[0].c_str());	
			double value4 = atof(PixelSpacing[1].c_str());
			cout << value3 << " " << value4 << endl;

			int InstancePosition = dcm.getPositionNumber();
			cout << InstancePosition << " " << value2 - InstancePosition + 1 << endl;
		}

	}
	 
	//Candidate candidate;
	//candidate.WorldCoordinateConvert();
	//candidate.OutputCandidate();
	
	//DcmFileProcess::readAllDcm("F:\\lymph node detection dataset\\DOI\\ABD_LYMPH_001\\61.7.22285965616260355338860879829667630274\\61.7.167248355135476067044532759811631626828");
	//DcmFileProcess::deleteCache();
	/*
	cv::Mat temp = cv::imread("D:\\MFC\\Samples\\OpenCV\\Debug\\cache\\000001.bmp", cv::IMREAD_GRAYSCALE);
	std::cout << temp.channels();
	uchar* data = new uchar[temp.rows * temp.cols];
	int count = 0;
	for (int i = 0; i < temp.rows; i++)
	{
		for (int j = 0; j < temp.cols; j++)
		{
			if (i >= 0 && i < temp.cols && j >= 0 && j < temp.rows)
				//std::cout << (int)temp.at<uchar>(i, j) << " ";
			data[count] = temp.at<uchar>(i, j);
			count++;
		}
		std::cout << std::endl;
	}

	cv::Mat mat(temp.rows, temp.cols, CV_8UC1);
	for (int i = 0; i < mat.rows; ++i){       //վ��˵ָ��Ч����ߣ��Ҿ�����ָ��= =#
		uchar *p = mat.ptr<uchar>(i);
		for (int j = 0; j < mat.cols; ++j) //M.ptr<uchar>(i)���ص��ǵ� i �����ص���׵�ַ
			p[j] = data[i * mat.rows + j];
	}

	for (int i = 0; i < temp.rows; i++)
	{
		for (int j = 0; j < temp.cols; j++)
		{
			if (i >= 0 && i < temp.cols && j >= 0 && j < temp.rows)
				std::cout << (int)mat.at<uchar>(i, j) << " ";
		}
		std::cout << std::endl;
	}
	imwrite("test.bmp", mat);
	*/
	

	system("pause");

	return 0;
}

