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

unsigned short Labeling::operator()(const cv::Mat& img, int connect){
	labeltable.resize(0);
	labeltable.push_back(0);

	label=cv::Mat::zeros(img.rows,img.cols,CV_16U);
	unsigned short currentlabel=0;

	const unsigned char *line=img.ptr<unsigned char>(0);
	unsigned short *line_label=label.ptr<unsigned short>(0), *line_label_prev=NULL;

	// at least 2x2 pixels
	if(img.rows < 2 || img.cols < 2) throw "Image size is invalid.";

	for(int y=0; y<img.rows; y++, line_label_prev=line_label){
		line=img.ptr<unsigned char>(y);
		line_label=label.ptr<unsigned short>(y);

		for(int x=0; x<img.cols; x++){
			if(line[x]!=0){
				neighbor[0]= y>0 && x>0 && connect==LABELING_CONNECT_8 ? line_label_prev[x-1] : 0;
				neighbor[1]= y>0                   ? line_label_prev[x]   : 0;
				neighbor[2]= y>0 && x < img.cols-1 && connect==LABELING_CONNECT_8 ? line_label_prev[x+1] : 0;
				neighbor[3]= x>0                   ? line_label[x-1]      : 0;

				line_label[x]=_checkNeighbor();
				if(line_label[x]==0){
					if(currentlabel==std::numeric_limits<unsigned short>::max()){
						throw "Label number overflow.";
					}
					currentlabel++;
					line_label[x]=currentlabel;
					labeltable.push_back(currentlabel);
				}
			}
		}
	}

	int num=_compaction();

	_sort(num);

	for(int y=0;y<label.rows;y++){
		unsigned short *label_line=label.ptr<unsigned short>(y);
		for(int x=0;x<label.cols;x++){
			label_line[x]=labeltable[label_line[x]];
		}
	}

	isLabeled=true;
	return num;
}

cv::Mat Labeling::getLabel()const{
	if(!isLabeled) throw "call operator() first";
	return label.clone();
}

unsigned short Labeling::getRegionSize(unsigned short i)const{
	if(!isLabeled) throw "call operator() first";
	return regionsize[i];
}

unsigned short Labeling::_compaction(){
	for(unsigned short i=0;i<labeltable.size();i++){
		labeltable[i]=_compaction(i);
	}
	_compaction2();
	return *(std::max_element(labeltable.begin(),labeltable.end()))+1;
}

unsigned short Labeling::_compaction(unsigned short i){
	if(i==labeltable[i]) return i;
	
	labeltable[i]=_compaction(labeltable[i]);
	return labeltable[i];
}

unsigned short Labeling::_checkNeighbor(){
	unsigned short val=std::numeric_limits<unsigned short>::max();

	for(unsigned short i=0;i<4;i++){
		if(neighbor[i]==0)continue;
		if(val >= labeltable[neighbor[i]]) val=labeltable[neighbor[i]];
	}

	if(val!=std::numeric_limits<unsigned short>::max()){
		for(int i=0;i<4;i++){
			if(neighbor[i]==0)continue;
			labeltable[neighbor[i]]=val;
		}
		return val;
	}
	return 0;
}

void Labeling::_compaction2(){
	std::map<unsigned short,unsigned short> usedlabel;
	usedlabel[0]=0;
	std::map<unsigned short,unsigned short>::iterator prev;
	unsigned short i=0;

	for(std::vector<unsigned short>::iterator it=labeltable.begin();it!=labeltable.end();it++){
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

void Labeling::_sort(unsigned short num){
	std::vector<std::pair<unsigned short,unsigned short> > regiontemp;

	regionsize.resize(num);

	// count the number of pixels in 'num'-th region
	for(int y=0;y<label.rows;y++){
		const unsigned short *label_line=label.ptr<unsigned short>(y);
		for(int x=0;x<label.cols;x++){
			if(label_line[x]!=0) regionsize[labeltable[label_line[x]]]++;
		}
	}

	unsigned short i=0;
	for(std::vector<unsigned short>::iterator it=regionsize.begin(); it!=regionsize.end(); it++){
		regiontemp.push_back(std::make_pair(i,*it));
		i++;
	}

	std::vector<std::pair<unsigned short,unsigned short> >::iterator it=regiontemp.begin();it++;
	std::sort(it, regiontemp.end(), _PairSort());

	i=0;
	regionsize.clear();
	std::map<unsigned short,unsigned short> remap; // <before, after>
	for(std::vector<std::pair<unsigned short,unsigned short> >::iterator it2=regiontemp.begin(); it2!=regiontemp.end(); it2++){
		remap[it2->first]=i;
		regionsize.push_back(it2->second);
		i++;
	}

	// re-labeling
	for(std::vector<unsigned short>::iterator lt=labeltable.begin(); lt!=labeltable.end(); lt++){
		*lt=remap[*lt];
	}
}
