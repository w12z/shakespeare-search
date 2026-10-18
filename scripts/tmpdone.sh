#!/usr/bin/env bash
cd "$(dirname "$0")/.." || exit 1

echo "=== 索引规模（上一轮是 967 docs / 958658 tokens / 339340 postings）==="
echo "  $(wc -l < index/docs.tsv) docs / $(wc -l < index/tokens.tsv) tokens / $(wc -l < index/inverted.tsv) postings"

echo
echo "=== 检索自检 ==="
printf 'The rest is silence\nmy kingdom for a horse\nhamlet\n' | ./search | grep -E '^(>|[0-9]+ hits|\[1\])'

echo
echo "=== 提交 ==="
printf '%s\n' '去掉多余的注释' > /tmp/m.txt
git add -A
git commit -q -F /tmp/m.txt
rm -f /tmp/m.txt
git log --oneline | head -5

echo
echo "=== 推送 ==="
git push origin main 2>&1 | tail -3
git status -sb | head -2
