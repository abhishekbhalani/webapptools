#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "07";
	static const char MONTH[] = "08";
	static const char YEAR[] = "2009";
	static const double UBUNTU_VERSION_STYLE = 9.08;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 1;
	static const long BUILD = 20;
	static const long REVISION = 108;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 324;
	#define RC_FILEVERSION 0,1,20,108
	#define RC_FILEVERSION_STRING "0, 1, 20, 108\0"
	static const char FULLVERSION_STRING[] = "0.1.20.108";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 0;
	

}
#endif //VERSION_h
