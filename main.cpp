#include <cv.h>
#include <highgui.h>
#include <iostream>
#include "Labeling.h"

#ifdef __BENCH__ // Define __BENCH__ if you want to do benchmarking.
#include <time.h>
void benchmark(const cv::Mat& img, Labeling& labeler);
#endif

int main(int argc, char **argv){
	// Target image ( CV_8UC1, binary(W/B) image )
	cv::Mat img=cv::imread("test.bmp",CV_LOAD_IMAGE_GRAYSCALE);
	// Result image ( CV_16UC1 )
	cv::Mat label;
	// for Visualization
	cv::Mat resultimg;

	int num; // number of blob ids ( id of the background is 0 )

	Labeling labeler;

	// labeling procedure
	num=labeler(img,LABELING_CONNECT_8);

#ifdef __BENCH__
	benchmark(img, labeler);
#endif

	// get the labeling result
	label=labeler.getLabel();
	std::cout << "number of regions: " << num << std::endl;

	// number of the regions are start from 1
	for(int i=1;i<num;i++){
		std::cout << "size of region " << i;
		std::cout << ": " << labeler.getRegionSize(i) << std::endl;
	}

	// visualizing
	label=label*(255.0/num);
	label.convertTo(resultimg,CV_8UC1);

	// show
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
