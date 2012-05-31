/*!
 * @file Labeling.h
 * @author Yasutomo Kawanishi
 * @date Last Change:2011/May/30.
 * */

#ifndef __LABELING_H_
#define __LABELING_H_
#include <cv.h>
#include <vector>

#define LABELING_CONNECT_8 0
#define LABELING_CONNECT_4 1

/**
 * @class Labeling
 * @note 0が背景
 * ソートする場合は面積の大きい順に1,2,3…
 * ソートしない場合は見つけた順に1,2,3…
 * のラベルが振られる。
 * */
class Labeling{
public:
	Labeling();
	~Labeling();
	Labeling(const Labeling& other);
	Labeling& operator=(const Labeling& other);

	unsigned int operator()(const cv::Mat& src, int connect=LABELING_CONNECT_8); // 領域ラベリングを行う
	cv::Mat_<unsigned int> getLabel()const; // ラベル画像を取得（背景が0,それ以外はラベルを画素値として持つCV_32UC1画像）
	unsigned int getRegionSize(unsigned int i)const; // 指定した領域の大きさを返す

private:
	inline unsigned int _checkNeighbor();
	inline unsigned int _compaction();
	inline unsigned int _compaction(unsigned int i);
	inline void _compaction2();
	inline void _sort(unsigned int num);

	unsigned int neighbor[4];
	std::vector<unsigned int> labeltable;
	std::vector<unsigned int> regionsize;
	cv::Mat label;
	bool isLabeled;

	struct _PairSort{
	public:
		bool operator()(const std::pair<unsigned int,unsigned int>& x,const std::pair<unsigned int,unsigned int>& y)const
			{return x.second>y.second;}
	};
};

#endif
