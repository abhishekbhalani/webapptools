
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86

mkdir build
cd build
cmake ../watools -DBUILD_DOMSHELL=0 -DBUILD_EXAMPLES=0 -DNEED_GENERATE_DOM=1





