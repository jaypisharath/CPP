/******************************************************************************
 * Streaming Autocorrelation
 * Author: Jay Pisharath
 * Version: 1.8
 * Last Modified: 2019/06/12

*******************************************************************************/
#include <stdio.h>
#include <iostream>
#include <new>
using namespace std;
#include <thread>
#include <functional>
#include <unistd.h>
#include <cmath>



/***************************
 * Data Stream Generation *
 **************************/
class Stream
{
public:
    int *dataStream;
    int nPoints;
    double *timeStamp;
	
  /*** Generate Stream  ***/
  void generateStream (int interval, int numPoints)
  {
    dataStream = new int[numPoints];
	auto start = std::chrono::high_resolution_clock::now();
    nPoints = numPoints;
	timeStamp = new double[numPoints];
	
    if ((dataStream == nullptr) || (timeStamp == nullptr))
    {
	    cout << "Error: Memory could not be allocated";
    }
    else
    {
	    for (int i = 0; i < numPoints; i++)
	    {
            //Generate stream at interval
            if(i%interval != 0) 
                dataStream[i] = 0; 
            else dataStream[i] = 1;
			auto end = std::chrono::high_resolution_clock::now();
    		std::chrono::duration<double, std::milli> elapsed = end-start;
			//std::cout << "Waited " << elapsed.count() << " ms\n";
			timeStamp[i] = elapsed.count();
	
	    }
        
    }
  }
 
  /*** Print Stream  ***/
  void printStream() {
	  cout << "Total number of points in stream = " << nPoints << "\n";
      for (int i=0; i<nPoints; i++) {
		cout << "Data: "<< dataStream[i];
		cout << ", Time: " << timeStamp[i] << "\n";
	  }
	}
};


/*********************
 * Window Processing *
 *********************/
 
class X_window {
public:
	int *xWin;
    int nPoints;
	double maxTau = 1;
	
	
	/*** Window Identification  ***/
	void generateWin(Stream inStream, double winSize){
		int i=0;
		nPoints = findHops (inStream, winSize);
		//cout << nPoints << " points in hops"<<"\n";
		xWin = new int[nPoints];
		int inStreamCount = inStream.nPoints;
		
		while (i < nPoints) {
			xWin[i] = inStream.dataStream[inStreamCount-i-1];
			//cout << inStream.dataStream[inStreamCount-i-1] << ", " << xWin[i];
			i++; 
		}
	}
	
	/*** Autocorrelation function for a givean Tau  ***/
	double autoCorrelate(int tau) {
		double reg = 0;
		for(int i=0; i+tau<nPoints; i++) {
			reg = reg + (xWin[i]*xWin[i+tau]);
		}
		reg =  reg/(nPoints-tau);
		return reg;
	}
	
	/*** Finding the max Tau  ***/
	void findMaxTau() {
		double reg = 0;
		double prevReg = 0;
		
		//cout << "\nnPoints = " << nPoints;
		for(int i=1; i<nPoints-1; i++) {
			reg = autoCorrelate(i);
			if(i==1) prevReg = reg;
			if (reg > prevReg) {
				prevReg = reg;
				maxTau = i;
			}
		}
	}

	int findHops(Stream inStream, double winSize) {
		double endTime, startTime, temp, hops;
		endTime = inStream.timeStamp[inStream.nPoints-1];
		hops = 0;
		startTime = inStream.timeStamp[0];
		if (winSize > (endTime-startTime)) {
			cout << "Window range exceeds total range of timestamp in stream \n";
			exit(-1);
		}
		for (int i=inStream.nPoints-2; i>0; i--) {
			startTime = inStream.timeStamp[i];
			temp = endTime - startTime;
			if (temp <= winSize) {
				hops++;
			}
			else {
				i=0;
			}
		}
		return hops;
		
	}
	
	/*** Print Window Data  ***/
	 void printWindow() {
		 cout << "Total number of points in window = " << nPoints << "\n";
      for (int i=0; i<nPoints; i++) {
			cout << xWin[i]<<"\n";
	  }
	}

	};


/*********************
  		MAIN 		  
 *********************/

int main ()
{
  using namespace std::this_thread; 
  Stream stream1;
  float freq;
  int numPoints, interval;
  bool flag= true;
  double winSize;
  int dataSetCount=0;
  double waitTime = 3;
  
  cout << "\n\nEnter frequency of data stream in Hz: ";
  cin >> freq;
  freq = freq/1000;
  
  cout << "\nEnter number of sample data points in stream: ";
  cin >> numPoints;
  
  cout << "\nEnter interval of samples with value 1 (every x entry): ";
  cin >> interval;
  
  // Data Generation and Max Autocorrelation
  while (flag) {
	  
	auto start = std::chrono::high_resolution_clock::now();

	cout << "\n*********\n Generating data set #" << dataSetCount << "\n*********";
	thread t1(&Stream::generateStream, stream1,interval, numPoints);
	t1.join();
	dataSetCount++;
	//serial version => stream1.generateStream(interval, numPoints);

	//Get window size in millisecs
	cout << "\nEnter window size in millisecs: ";
	cin >> winSize;
	winSize = winSize;
	
	//Generate window
	X_window xWin1;
	xWin1.generateWin(stream1, winSize);
	
	//Find max tau		
	thread t2(&X_window::findMaxTau, xWin1, nullptr);
	t2.join();
	//Serial Version => xWin1.findMaxTau();
	cout << "Max Tau for this dataset = " << xWin1.maxTau << "\n";
	
	// Auto-launch after requested Hz value
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end-start;
	if(elapsed.count() < freq) 
		{
			usleep(freq-elapsed.count());		
		}
  }
  return 0;
}

