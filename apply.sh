#!/bin/bash

g++ -Wall main.cpp -o prinit -lstdc++fs
mv prinit bin/prinit
rm -r ~/prinit
cp -r bin ~/prinit
