#!/usr/bin/env bash
cd "$(dirname "$0")/.." || exit 1

echo "########## 源码里的注释 ##########"
grep -n '//' src/text.h src/engine.h src/extract_text.cpp src/tokenize.cpp src/index.cpp src/search.cpp src/server.cpp \
    | grep -v 'http://'

echo
echo "########## 程序打印的提示 ##########"
grep -n 'printf' src/search.cpp src/server.cpp

echo
echo "########## 网页里的提示文字 ##########"
grep -n 'textContent = ' web/search.html

echo
echo "########## README 里的说明性语句 ##########"
grep -n '不删的话\|——\|说明\|注意\|有用的\|否则\|不然' README.md
