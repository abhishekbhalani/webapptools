set PATH=c:\tools\python31;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

mkdir build
cd build
cmake ../watools -DBUILD_DOMSHELL=1 -DBUILD_EXAMPLES=0 -DNEED_GENERATE_DOM=1 -G "Visual Studio 10"





