#!/usr/bin/env bash
cd "$(dirname "$0")/.." || exit 1

pkill -x server 2>/dev/null
sleep 1

gcc -std=c11 -c src/porter_stem.c -o porter_stem.o || exit 1
g++ -std=c++17 -Wall -Wextra -O2 src/extract_text.cpp -o extract_text || exit 1
g++ -std=c++17 -Wall -Wextra -O2 src/tokenize.cpp     -o tokenize     || exit 1
g++ -std=c++17 -Wall -Wextra -O2 src/index.cpp  porter_stem.o -o indexer || exit 1
g++ -std=c++17 -Wall -Wextra -O2 src/search.cpp porter_stem.o -o search  || exit 1
g++ -std=c++17 -Wall -Wextra -O2 src/server.cpp porter_stem.o -o server  || exit 1
echo "编译通过"

echo
rm -rf corpus index
./extract_text
./tokenize
./indexer
echo "corpus $(find corpus -name '*.txt' | wc -l) 篇"
md5sum index/docs.tsv index/tokens.tsv index/inverted.tsv

echo
echo "### 现在还剩哪些注释 ###"
grep -n '//' src/text.h src/engine.h src/extract_text.cpp src/tokenize.cpp src/index.cpp src/search.cpp src/server.cpp \
    | grep -v 'http://'
