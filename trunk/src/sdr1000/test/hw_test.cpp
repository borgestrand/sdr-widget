// hw_test.cpp : Defines the entry point for the console application.
//

#include "hw_sdr1000.h"
#include <assert.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv)
{
	SDR1000 *sdr = new SDR1000("test", TRUE, TRUE, FALSE, 0x378);
	sdr->PowerOn();
	sdr->SetFreq(5.0 - 0.011025);
	cin.get();
	return 0;
}

