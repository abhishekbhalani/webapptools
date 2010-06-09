// dbtest.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <weHelper.h>

using namespace std;
using namespace webEngine;

int main(int argc, char* argv[])
{
    LibInit(); // for initialize logging

    LibClose();
	return 0;
}
