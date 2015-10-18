#!/bin/bash
TARGET_BUILD_FOLDER=../build

mkdir $TARGET_BUILD_FOLDER
#rm -rf $TARGET_BUILD_FOLDER/pixy
mkdir $TARGET_BUILD_FOLDER/pixy

cd $TARGET_BUILD_FOLDER/pixy
cmake ../../src/pixy
make
