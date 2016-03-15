
git submodule init
git submodule update
cd waf
./waf-light --tools=compat15,boost
cd ../protobuf-2.6.1
./configure --disable-shared





