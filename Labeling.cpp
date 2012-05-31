#include "Labeling.h"
#include <limits>

Labeling::Labeling():isLabeled(false){
}

Labeling::~Labeling(){
}

Labeling::Labeling(const Labeling& other)
	:labeltable(other.labeltable),regionsize(other.regionsize),label(other.label.clone()),isLabeled(other.isLabeled){
}

Labeling& Labeling::operator=(const Labeling& other){
	if(this==&other) return *this;
	this->labeltable=other.labeltable;
	this->regionsize=other.regionsize;
	this->label=other.label.clone();
	this->isLabeled=other.isLabeled;
	return *this;
}

unsigned int Labeling::operator()(const cv::Mat& img, int connect){
	labeltable.resize(0);
	labeltable.push_back(0);

	label=cv::Mat_<unsigned int>::zeros(img.rows,img.cols);
	unsigned int currentlabel=0;

	const unsigned char *line=img.ptr<unsigned char>(0);
	unsigned int *line_label=label.ptr<unsigned int>(0), *line_label_prev=NULL;

	// 少なくとも2x2以上の画像でないとだめ
	if(img.rows < 2 || img.cols < 2) throw "Image size is invalid.";

	for(int y=0; y<img.rows; y++, line_label_prev=line_label){
		/// 行ごとの処理
		line=img.ptr<unsigned char>(y);
		line_label=label.ptr<unsigned int>(y);

		for(int x=0; x<img.cols; x++){
			if(line[x]!=0){
				neighbor[0]= y>0 && x>0 && connect==LABELING_CONNECT_8 ? line_label_prev[x-1] : 0;
				neighbor[1]= y>0                   ? line_label_prev[x]   : 0;
				neighbor[2]= y>0 && x < img.cols-1 && connect==LABELING_CONNECT_8 ? line_label_prev[x+1] : 0;
				neighbor[3]= x>0                   ? line_label[x-1]      : 0;

				line_label[x]=_checkNeighbor();
				if(line_label[x]==0){
					/// 周りに領域がない場合
					if(currentlabel==std::numeric_limits<unsigned int>::max()){
						throw "Label number overflow.";
					}
					currentlabel++;
					line_label[x]=currentlabel;
					labeltable.push_back(currentlabel);
				}
			}
		}
	}

	// ラベル数を数えて，ラベルの再配置を行う
	int num=_compaction();

	// 領域の大きさ順にソートして，ラベルの再割り当てを行う
	_sort(num);

	// 領域分割画像にラベル再割当てを反映
	for(int y=0;y<label.rows;y++){
		unsigned int *label_line=label.ptr<unsigned int>(y);
		for(int x=0;x<label.cols;x++){
			label_line[x]=labeltable[label_line[x]];
		}
	}

	isLabeled=true;
	return num;
}

cv::Mat_<unsigned int> Labeling::getLabel()const{
	if(!isLabeled) throw "call operator() first";
	return label;
}

unsigned int Labeling::getRegionSize(unsigned int i)const{
	if(!isLabeled) throw "call operator() first";
	return regionsize[i];
}

unsigned int Labeling::_compaction(){
	for(unsigned int i=0;i<labeltable.size();i++){
		labeltable[i]=_compaction(i);
	}
	_compaction2();
	return *(std::max_element(labeltable.begin(),labeltable.end()))+1;
}

unsigned int Labeling::_compaction(unsigned int i){
	if(i==labeltable[i]) return i;
	
	labeltable[i]=_compaction(labeltable[i]);
	return labeltable[i];
}

unsigned int Labeling::_checkNeighbor(){
	unsigned int val=std::numeric_limits<unsigned int>::max();

	for(unsigned int i=0;i<4;i++){
		if(neighbor[i]==0)continue;
		if(val >= labeltable[neighbor[i]]) val=labeltable[neighbor[i]];
	}

	// 領域が複数ある場合テーブルを書き換える
	if(val!=std::numeric_limits<unsigned int>::max()){
		for(unsigned int i=0;i<4;i++){
			if(neighbor[i]==0)continue;
			labeltable[neighbor[i]]=val;
		}
		return val;
	}
	return 0;
}

void Labeling::_compaction2(){
	std::map<unsigned int,unsigned int> usedlabel;
	usedlabel[0]=0;
	std::map<unsigned int,unsigned int>::iterator prev;
	unsigned int i=0;

	for(std::vector<unsigned int>::iterator it=labeltable.begin();it!=labeltable.end();it++){
		prev=usedlabel.find(*it);
		if(prev==usedlabel.end()){
			i++;
			usedlabel[*it]=i;
			*it=i;
		}else{
			*it=prev->second;
		}
	}
}

void Labeling::_sort(unsigned int num){
	std::vector<std::pair<unsigned int,unsigned int> > regiontemp;

	regionsize.resize(num);

	// 画素値のカウント
	for(int y=0;y<label.rows;y++){
		const unsigned int *label_line=label.ptr<unsigned int>(y);
		for(int x=0;x<label.cols;x++){
			// 背景でなければカウント
			if(label_line[x]!=0) regionsize[labeltable[label_line[x]]]++;
		}
	}

	// ソートのためmapからvectorへ
	unsigned int i=0;
	for(std::vector<unsigned int>::iterator it=regionsize.begin(); it!=regionsize.end(); it++){
		regiontemp.push_back(std::make_pair(i,*it));
		i++;
	}

	// 2個目以降を大きさ順にソート
	std::vector<std::pair<unsigned int,unsigned int> >::iterator it=regiontemp.begin();it++;
	std::sort(it, regiontemp.end(), _PairSort());

	// ラベルの再割り当てマップを作る
	i=0;
	regionsize.clear();
	std::map<unsigned int,unsigned int> remap; // <割り当て前,割り当て後>
	for(std::vector<std::pair<unsigned int,unsigned int> >::iterator it2=regiontemp.begin(); it2!=regiontemp.end(); it2++){
		remap[it2->first]=i;
		regionsize.push_back(it2->second);
		i++;
	}

	// ラベルの再割り当て
	for(std::vector<unsigned int>::iterator lt=labeltable.begin(); lt!=labeltable.end(); lt++){
		*lt=remap[*lt];
	}
}
