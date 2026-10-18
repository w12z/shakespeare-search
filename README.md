# Shakespeare 检索

输入几个词，返回最相关的若干段落。
建索引和检索分开，索引建一次可以反复查。

## 用到的现成代码和借助 AI 部分

词干化直接用 Martin Porter 的 ANSI C 实现（tartarus.org 上那份）。
只做了两处机械修改：砍掉文件自带的 demo 驱动、把入口从 `stem()` 改名成 `porter_stem()`。
算法主体一行没动。源码 `src/porter_stem.c`，声明 `src/porter_stem.h`。

停用词表直接下载的 snowballstem.org 的英文表。

同时 HTML 摘除的正则也借助了 AI 写；server.cpp 这个网页交互前端也大量借助了 AI（不是很会 C++ 的 Web，而且这个交互前端其实可有可无）。

## 编译

```bash
gcc -std=c11 -c src/porter_stem.c -o porter_stem.o

g++ -std=c++17 -O2 src/extract_text.cpp -o extract_text
g++ -std=c++17 -O2 src/tokenize.cpp     -o tokenize
g++ -std=c++17 -O2 src/index.cpp  porter_stem.o -o indexer
g++ -std=c++17 -O2 src/search.cpp porter_stem.o -o search
g++ -std=c++17 -O2 src/server.cpp porter_stem.o -o server
```

`index.cpp` 编出来的可执行文件得叫 `indexer`，不能叫 `index`，因为 `index/` 已经是目录了。

`porter_stem.c` 是 C 文件，必须用 `gcc` 编。直接丢给 `g++` 会因为
"ISO C++ forbids converting a string constant to char*" 报错。

## 运行

前两步是建索引，只在语料变了的时候才需要重跑。

```bash
./extract_text     # web/    -> corpus/   HTML 抽正文
./tokenize         # corpus/ -> index/    分词
./indexer          # index/  -> index/    建倒排索引

./search                                # 命令行，打开后在提示符后面输查询
./server                                # 网页，浏览器开 http://127.0.0.1:8080/
```

`./search` 会把索引读进内存，然后一直等输入，这是一个控制台形式的前端。

空行或者 Ctrl-D 退出。也可以直接管道喂给它：`printf 'love death\n' | ./search`。

`./server` 启动时同样把索引读进内存，然后在搜索框中输入进行搜索，这是一个网页形式的前端。

## 目录

```
project1/
├── web/           下载的原始网站。search.html 是搜索页，放在这里是为了能直接跳到原文
├── corpus/        抽取出来的正文，extract_text 生成
├── index/         tokenize 和 indexer 生成，检索时读这个
├── resources/     停用词表
├── src/           源码
├── scripts/       下载依赖的脚本
└── README.md
```

`src/` 里九个文件：

```
extract_text.cpp   HTML -> 正文
tokenize.cpp       正文 -> 词元
index.cpp          词元 -> 倒排索引
engine.h           读索引 + 打分，search 和 server 共用
search.cpp         命令行检索
server.cpp         网页检索
text.h             分词、读文件这些底层工具
porter_stem.c      词干化，Martin Porter 的 ANSI C 实现
porter_stem.h
```

## 正文怎么抽出来的

一页 HTML 先删掉注释、`head`/`script`/`style`，再删掉顶上那个 `class="nav"` 的导航表格，
剩下的标签当成分隔符，最后解码 HTML 实体。`<br>` `<p>` `<tr>` 这类标签额外产生一个换行，
好把台词和角色名分成不同的行 —— 后面摘要要按行取。

导航栏不删的话，"Previous scene" "Next scene" "Shakespeare homepage" 这些会进词表。

只跳过 `full.html`（全文页，和分场文件内容重复）、`news.html` 和 `test.html`。
**目录页都留着** —— 各剧本的场次列表、`works.html`、站点首页都进了语料，
因为检索时搜到目录页也是有用的。

`Comedy/`、`History/`、`Tragedy/` 是同一批文件的镜像，按内容去重，保留顶层那份；
有几个文件两个版本内容不同（比如 `lingers`/`fingers`），那属于不同版本，两个都留着。

## 分词规则

词元的定义是"连续的字母数字"，撇号只在两个字母中间才保留：

```
beauty's   ->  beauty's
o'er       ->  o'er
'tis       ->  tis        开头的撇号是省音记号，丢掉
to-night   ->  to, night  连字符当分隔符
2s         ->  2s         数字保留
loveth     ->  loveth     古语词不动
```

然后统一转小写，再过一遍 Porter 词干化。查询走的是**完全一样**的这套规则。

## 打分

```
score = 余弦 + 0.5 × near / pairs
```

**余弦部分。** 把查询和文档都看成"词 -> 次数"的向量，算夹角的余弦：

```
cos = (F(q), F(D)) / (‖F(q)‖ · ‖F(D)‖)
```

查询侧会先丢掉停用词；如果全被丢光（比如 `to be or not to be`），就全保留，
否则查询向量是空的。文档侧不丢，因为下一块要用到停用词的位置。

**相邻部分。** 把查询里每一对相邻词拎出来，看文档里有没有"位置 p 和 p+1 同时出现"。
命中的对数除以总对数就是 `near`，乘 0.5 加到余弦上。

位置在内存里打包成一个整数：`行号 * 2^20 + 行内位置`。这样判断"挨着"就是 `x + 1`，
取行号就是 `x >> 20`，不用去解析字符串。

`to be or not to be` 这句全靠相邻部分才能搜到：六个词全是停用词，在词袋模型里余弦只有 0.31，
排到 91 名；加上相邻（5/5 命中）之后才第一，正好是 Hamlet 5.2 第 91 行那句。
`The rest is silence` 更极端，余弦 0.0069、排第 172，相邻 3/3 之后第一。

**摘要行**取相邻命中最多的那一行。没有相邻命中就取最早的正文行 —— 跳过第 1 行，
那行是剧本标题。

## 已知问题

- **各剧本的场次列表被收录了两遍**。`hamlet/hamlet.html` 和 `hamlet/index.html` 是同一个列表，
  只差一句亚马逊的买书广告（`You can buy the Arden text of this play from...`），
  字节不同所以去重去不掉。37 个剧本都有这一对，搜 `hamlet` 时两页会一起出现。
  要么去重改成比抽取后的正文，要么把那句广告当样板文字删掉。
- **相邻判断用的是词干**，所以 `lovely death` 会命中 `love death`（`lovely` 的词干是 `love`）。
  搜 `love death` 排第一的是 `Death, death; O amiable lovely death!`，原文里并没有这个词组。
  要修得把位置索引改成按原词存。
- **每个 txt 的第 1 行是剧本标题，也进了索引**。同一个剧本的每个场景文件都带同一句标题，
  相当于把标题里的词放大了一遍。建索引时跳过第 1 行就能解决。
- **余弦是完整的长度归一化，短文档占优**。搜 `love death` 时前 8 名全是十四行诗，
  因为 120 词里出现 9 次 `love` 的"密度"很高。换 BM25 的部分长度归一化能解决，
  需要的数据（tf、文档长度、df）索引里现成都有。
- 只返回前 10 条，没有分页，也没有 AND/OR/引号之类的查询语法。
- 相邻判断要求同一个词对在同一行内相邻，跨行的词组匹配不到。
