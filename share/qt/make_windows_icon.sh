#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/EmpireCoin.png
ICON_DST=../../src/qt/res/icons/EmpireCoin.ico
convert ${ICON_SRC} -resize 16x16 EmpireCoin-16.png
convert ${ICON_SRC} -resize 32x32 EmpireCoin-32.png
convert ${ICON_SRC} -resize 48x48 EmpireCoin-48.png
convert EmpireCoin-16.png EmpireCoin-32.png EmpireCoin-48.png ${ICON_DST}

