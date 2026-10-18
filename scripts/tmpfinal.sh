#!/usr/bin/env bash
cd "$(dirname "$0")/.." || exit 1

rm -f scripts/tmpshow.sh

printf '%s\n' '精简注释，删掉误提交的临时脚本' > /tmp/m.txt
git add -A
git commit -q -F /tmp/m.txt
rm -f /tmp/m.txt

echo "=== 提交 ==="
git show --stat --oneline HEAD | head -12

echo
echo "=== 推送 ==="
git push origin main 2>&1 | tail -3

echo
echo "=== 对齐检查 ==="
echo "本地  $(git rev-parse main)"
echo "远端  $(git ls-remote origin refs/heads/main | cut -f1)"
echo
git status -sb | head -3

echo
echo "=== 仓库里 scripts/ 只剩什么 ==="
git ls-tree -r --name-only origin/main | grep '^scripts/'
