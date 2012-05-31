#include <cv.h>
#include <highgui.h>
#include <iostream>
#include "Labeling.h"

#ifdef __BENCH__ // ベンチマークをする場合は __BENCH__を一番上でdefineしてください
#include <time.h>
void benchmark(const cv::Mat& img, Labeling& labeler);
#endif

int main(int argc, char **argv){
	// 対象となる画像（CV_8UC1の二値画像である必要があります）
	cv::Mat img=cv::imread("test.bmp",CV_LOAD_IMAGE_GRAYSCALE);
	// 処理結果（cv::Mat_<int>で返します）
	cv::Mat label;
	// 結果表示用
	cv::Mat resultimg;

	int num; // 領域数（背景も１つの領域（0番目）として含む）

	Labeling labeler;

	// ラベリング処理
	num=labeler(img,LABELING_CONNECT_8);

#ifdef __BENCH__ // ベンチマークをする場合は __BENCH__をdefineしてください
	benchmark(img, labeler);
#endif

	// ラベリング結果を取得
	label=labeler.getLabel();
	std::cout << "number of regions: " << num << std::endl;

	// 1からが白の領域
	for(int i=1;i<num;i++){
		std::cout << "size of region " << i;
		std::cout << ": " << labeler.getRegionSize(i) << std::endl;
	}

	// 適当に可視化して
	label*=(double)256/num;
	label.convertTo(resultimg,CV_8UC1);

	// 表示
	cv::imshow("test", img);
	cv::imshow("label", resultimg);
	cv::waitKey(0);
	return 0;
}

#ifdef __BENCH__
void benchmark(const cv::Mat& img, Labeling& labeler){
	const int repeat=1000;
	int num;
	clock_t start,end;
	start = clock();

	for(int i=0;i<repeat;i++) num=labeler(img,LABELING_CONNECT_8);
	end = clock();
	std::cout << "average:" << (double)(end-start)/CLOCKS_PER_SEC / repeat << std::endl;
}
#endif
