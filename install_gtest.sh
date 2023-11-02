#/bin/bash

sudo apt-get install libgtest-dev 
cd /usr/src/gtest 

sudo mkdir build 
cd build 

sudo cmake .. 
sudo make 
sudo cp libgtest* /usr/lib/ 
sudo mkdir /usr/local/lib/gtest 
sudo ln -s /usr/lib/libgtest.a /usr/local/lib/libgtest.a 
sudo ln -s /usr/lib/libgtest_main.a /usr/local/lib/libgtest_main.a