#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"

using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static std::string getCmd(std::string pid);
    static std::vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static float getSysUpTime();
    static std::string getProcUpTime(string pid);
    static std::string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static std::string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static std::string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
  
  
  //where does this go?
  	static float getSysActiveCpuTime(vector<string> values);
  	static float getSysIdleCpuTime(vector<string>values);
  	
};

bool ProcessParser::isPidExisting(string pid){
	vector<string> _list = ProcessParser::getPidList();
    for(string _pid : _list){
      if(_pid==pid)
        return true;
    }
  	return false;
}

string ProcessParser::getCmd(string pid){
  std::string out;
  ifstream stream = Util::getStream(  (Path::basePath() + pid + Path::cmdPath()));
  std::getline(stream,out);
  return out;
}

std::string ProcessParser::getVmSize(string pid){
  	float value;
  	string line;
  	string nm = "VmData"; 
    ifstream stream = Util::getStream( (Path::basePath() + pid + Path::statusPath()) );
  	
  	while(std::getline(stream,line)){
    	if( line.compare(0, nm.size() ,nm)==0 ){
            
          	vector<string> values = Util::splitLine(line);        
            value = (stof(values[1])/float(1024*1024));
          	break;
        }
    }
  	return to_string(value);
}

std::string ProcessParser::getCpuPercent(string pid){
  float res; 
  float utime  = stof(ProcessParser::getProcUpTime(pid)); //[ticks], CPU time spent in user code
  float uptime = ProcessParser::getSysUpTime(); //[s] uptime of system
  float w = sysconf(_SC_CLK_TCK); //[ticks/s]
  
  //extract stuff from stat file
  std::string line;
  ifstream stream = Util::getStream( (Path::basePath() + pid + Path::statPath())); //single line
  std::getline(stream,line);
  vector<string> values = Util::splitLine(line);        
  float stime = stof(values[14]);  //stime 14 [ticks], CPU time in kernel
  float cutime = stof(values[15]); //cutime 15 [ticks], waited for childrens cpu time in user
  float cstime = stof(values[16]); //cstime 16 [ticks], waited for childrens cpu time in kernel
  float starttime = stof(values[21]); //starttime 21 [ticks], time when process startet
  
  //calc cpu usage
  float ttotprog = utime + stime + cutime + cstime; //[ticks]  total time spent in program 
  float ttotsys  = uptime - (starttime/w); //[s]  total system time since program start
  
  res = 100.0*((ttotprog/w)/ttotsys);  //[%]
  return std::to_string(res);
  
}

std::vector<std::string> ProcessParser::getSysCpuPercent(std::string coreNumber){
	string line;
  	string nm = ("cpu"+coreNumber);
  	ifstream stream = Util::getStream((Path::basePath() + "stat"));
  	while(std::getline(stream,line)){
    	if( line.compare(0, nm.size() ,nm)==0 ){
        	vector<string> values = Util::splitLine(line);        
          	return values;
          	
        }
    }
  return (std::vector<std::string> ()); //empty if cpu not in list
}

float ProcessParser::getSysActiveCpuTime(vector<string> values){
  return(stof(values[S_USER]) + 
         stof(values[S_NICE]) +
         stof(values[S_SYSTEM]) +
         stof(values[S_IRQ]) + 
         stof(values[S_SOFTIRQ]) + 
         stof(values[S_STEAL]) + 
         stof(values[S_GUEST]) + 
         stof(values[S_GUEST_NICE]));
}
float ProcessParser::getSysIdleCpuTime(vector<string>values){
  return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string> values2){
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0*(activeTime / totalTime);
    return to_string(result);
}

int ProcessParser::getNumberOfCores(){
  	string line;
  	string nm = "cpu cores";
  	ifstream stream = Util::getStream((Path::basePath() + "cpuinfo"));
  	while(std::getline(stream,line)){
    	if( line.compare(0, nm.size() ,nm)==0 ){
        	vector<string> values = Util::splitLine(line);        
          	return stoi(values[3]);
          	
        }
    }
  	return 0;
}


std::string ProcessParser::getProcUpTime(string pid){
  //should return process up time in ticks
  std::string line;
  ifstream stream = Util::getStream( (Path::basePath() + pid + Path::statPath())); //single line
  std::getline(stream,line);
  vector<string> values = Util::splitLine(line);
  std::string uptime = values[13]; //[s]
  return std::to_string(float(stof(uptime)/sysconf(_SC_CLK_TCK)));
}

std::vector<std::string> ProcessParser::getPidList(){
  string cmd;
  DIR* dir; //pointer to directory
  vector<string> container;
  if (!(dir = opendir("/proc"))){
    throw std::runtime_error(std::strerror(errno));
  }
  
  while(dirent* dirp = readdir(dir)){ //dirp is struc with dir contents
    if (dirp->d_type != DT_DIR){ //not folder
      continue; 
    }
    if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){return std::isdigit(c);})){
      container.push_back(dirp->d_name);
    }  
  }

  if(closedir(dir)){
    throw std::runtime_error(std::strerror(errno)); //not really necessary 
  return container;
  }
}

float ProcessParser::getSysUpTime(){
  //should return [s] uptime of system
  std::string line;
  ifstream stream = Util::getStream( (Path::basePath() + Path::upTimePath()) ); //single line
  std::getline(stream,line);
  vector<string> values = Util::splitLine(line);
  return stof(values[0]);
}
  
std::string ProcessParser::getProcUser(std::string pid){
  //return user of process pid
  std::string line;
  std::string nm = "Uid:";
  std::string res;
  ifstream stream = Util::getStream( (Path::basePath() + pid + Path::statusPath())); //single line
  while(std::getline(stream,line)){
    if( line.compare(0, nm.size() ,nm)==0 ){
      vector<string> values = Util::splitLine(line);
      nm = ("x:" + values[1]);
      break;
    }
  }
  //look into user list to get the name from the number
  stream = Util::getStream( "/etc/passwd");
  while(std::getline(stream,line)){
    if( line.find(nm) != std::string::npos ){ //if find doesn't end at line end
      res = line.substr(0,line.find(":"));
      return res;
    }
  }
}
  
  
std::string ProcessParser::getSysKernelVersion(){
	string line;
  	ifstream stream = Util::getStream( (Path::basePath() + Path::versionPath()) );
  	std::getline(stream,line);
    vector<string> values = Util::splitLine(line);
  	return values[2];
}


std::string ProcessParser::getOSName(){
  string line;
  string nm = "PRETTY_NAME=";
  ifstream stream = Util::getStream(("/etc/os-release"));
  while(std::getline(stream,line)){
    if( line.compare(0, nm.size() ,nm)==0 ){
      std::size_t idx = line.find("=");
      idx++;
      string name = line.substr(idx);
      name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
      return name;
    }
  }  
}

int ProcessParser::getTotalThreads(){
  
  int totalthreads = 0;
  string nm = "Threads:";
  string line;
  vector<string> pidlist = ProcessParser::getPidList();
  
  for(string pid : pidlist){
    ifstream stream = Util::getStream( (Path::basePath() + pid + Path::statusPath()) );
    while (std::getline(stream, line)) {
      if (line.compare(0, nm.size() ,nm)==0 ){
        vector<string> values = Util::splitLine(line);
        totalthreads += stoi(values[1]);
      	break;
      }
  	}
  }
  return totalthreads;
}

int ProcessParser::getTotalNumberOfProcesses(){
  
  string nm = "processes";
  string line;
  ifstream stream = Util::getStream( (Path::basePath() + "stat"));
  while (std::getline(stream, line)) {
    if (line.compare(0, nm.size() ,nm)==0 ){
      vector<string> values = Util::splitLine(line);
      return stoi(values[1]);
    }
  }
}

int ProcessParser::getNumberOfRunningProcesses(){
  
  string nm = "procs_running";
  string line;
  ifstream stream = Util::getStream( (Path::basePath() + "stat"));
  while (std::getline(stream, line)) {
    if (line.compare(0, nm.size() ,nm)==0 ){
      vector<string> values = Util::splitLine(line);
      return stoi(values[1]);
    }
  }
}
  
  
float ProcessParser::getSysRamPercent(){
  
  ifstream stream = Util::getStream( (Path::basePath() + Path::memInfoPath()));
  string line;
  string nm1 = "MemAvailable:";
  string nm2 = "MemFree:";
  string nm3 = "Buffers:";
  
  float total_mem = 0;
  float free_mem = 0;
  float buffers = 0;
  
  while (std::getline(stream, line)) {
    if (total_mem!=0 && free_mem!=0 && buffers!=0)
      break;
    if (line.compare(0, nm1.size() ,nm1)==0 ){
      vector<string> values = Util::splitLine(line);
      total_mem = stof(values[1]);
    }
    if (line.compare(0, nm2.size() ,nm2)==0 ){
      vector<string> values = Util::splitLine(line);
      free_mem = stof(values[1]);
    }
	if (line.compare(0, nm3.size(), nm3) == 0) {
      vector<string> values = Util::splitLine(line);
      buffers = stof(values[1]);
    }
  }
  float res = 100.0*(1-(free_mem/(total_mem-buffers)));
  return res;
}

  



