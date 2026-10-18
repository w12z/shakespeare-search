#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.." || exit 1
mkdir -p src resources

# 停用词表
curl -sS -o resources/stopwords.txt https://snowballstem.org/algorithms/english/stop.txt

# Porter stemmer：砍掉文件自带的 demo 部分，入口 stem() 改名为 porter_stem()
curl -sS -o src/porter_stem.c https://tartarus.org/martin/PorterStemmer/c.txt
awk '/stemmer definition ends here/ {exit} {print}' src/porter_stem.c > src/porter_stem.tmp
mv src/porter_stem.tmp src/porter_stem.c
sed -i 's/^int stem(char \* p, int i, int j)/int porter_stem(char * p, int i, int j)/' src/porter_stem.c

echo "done"
wc -l src/porter_stem.c resources/stopwords.txt
