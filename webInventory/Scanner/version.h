#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "08";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2009";
	static const double UBUNTU_VERSION_STYLE = 9.09;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 1;
	static const long BUILD = 5;
	static const long REVISION = 32;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 225;
	#define RC_FILEVERSION 0,1,5,32
	#define RC_FILEVERSION_STRING "0, 1, 5, 32\0"
	static const char FULLVERSION_STRING[] = "0.1.5.32";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 4;
	

}
#endif //VERSION_h
