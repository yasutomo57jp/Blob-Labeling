/*!
 * @file Labeling.h
 * @author Yasutomo Kawanishi
 * @date Last Change:01-Jun-2012.
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

	unsigned short operator()(const cv::Mat& src, int connect=LABELING_CONNECT_8); // 領域ラベリングを行う
	cv::Mat getLabel()const; // ラベル画像を取得（背景が0,それ以外はラベルを画素値として持つCV_32UC1画像）
	unsigned short getRegionSize(unsigned short i)const; // 指定した領域の大きさを返す

private:
	inline unsigned short _checkNeighbor();
	inline unsigned short _compaction();
	inline unsigned short _compaction(unsigned short i);
	inline void _compaction2();
	inline void _sort(unsigned short num);

	unsigned short neighbor[4];
	std::vector<unsigned short> labeltable;
	std::vector<unsigned short> regionsize;
	cv::Mat label;
	bool isLabeled;

	struct _PairSort{
	public:
		bool operator()(const std::pair<unsigned short,unsigned short>& x,const std::pair<unsigned short,unsigned short>& y)const
			{return x.second>y.second;}
	};
};

#endif
