#!/bin/bash
TARGET_BUILD_FOLDER=../build

#rm -rf $TARGET_BUILD_FOLDER
mkdir $TARGET_BUILD_FOLDER

cd $TARGET_BUILD_FOLDER
cmake ../src/camera
make
