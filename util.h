#include <string>
#include <fstream>
#include <iterator>
#include <iostream>


// Classic helper function
class Util {

public:

static std::string convertToTime ( long int input_seconds );
static std::string getProgressBar(std::string percent);
static std::ifstream getStream(std::string path);
static std::vector<std::string> splitLine(std::string line);
};

std::string Util::convertToTime (long int input_seconds){
long minutes = input_seconds / 60;
long hours = minutes / 60;
long seconds = int(input_seconds%60);
minutes = int(minutes%60);
std::string result = std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
return result;
}
// constructing string for given percentage
// 50 bars is uniformly streched 0 - 100 %
// meaning: every 2% is one bar(|)
std::string Util::getProgressBar(std::string percent){

    std::string result = "0%% ";
    int _size= 50;
    int  boundaries;
    try {
        boundaries = (stof(percent)/100)*_size;
    } catch (...){
    boundaries = 0;
    }

    for(int i=0;i<_size;i++){
        if(i<=boundaries){
        result +="|";
        }
        else{
        result +=" ";
        }
    }

    result +=" " + percent.substr(0,5) + " /100%%";
    return result;
}

// wrapper for creating streams
std::ifstream Util::getStream(std::string path){
  	std::ifstream stream(path);
    if (!stream){
      	std::cout << path << "\n";
        throw std::runtime_error("Non - existing PID");
    }
    return stream;
}

std::vector<std::string> Util::splitLine(std::string line){
    std::istringstream buffer(line); 
    std::istream_iterator<std::string> st(buffer), end; //splicing
    std::vector<std::string> values(st,end);
	return values;
}


