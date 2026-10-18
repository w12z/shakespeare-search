#pragma once
#include<bits/stdc++.h>
#include"porter_stem.h"
using namespace std;
typedef unsigned long long ull;
// 读整个文件。内部 freopen 到 stdin，读完 stdin 就停在这个文件上了
string readFile(string path) {
    if(!freopen(path.c_str(), "r", stdin)) return "";
    cin.clear();
    return string(istreambuf_iterator<char>(cin), istreambuf_iterator<char>());
}
ull num(string s) {
    return strtoull(s.c_str(), 0, 10);
}
bool wordChar(char c) {
    return isalnum((unsigned char)c) != 0;
}
// 词干化。porter_stem 只认小写字母，传进来的必须已经转过小写
string stem(string w) {
    if(w.empty()) return w;
    vector<char> buf(w.begin(), w.end());
    buf.push_back('\0');
    int k = porter_stem(buf.data(), 0, (int)w.size() - 1);
    return string(buf.data(), k + 1);
}
vector<string> split(string s, char sep) {
    vector<string> v;
    string t;
    for(ull i = 0; i < s.size(); i++) {
        if(s[i] != sep) t += s[i];
        else v.push_back(t), t.clear();
    }
    if(!t.empty()) v.push_back(t);
    return v;
}
// 分词：连续的字母数字算一个词，撇号只在两个字母中间才保留，统一转小写
// 例：beauty's -> beauty's   'tis -> tis   to-night -> to, night
vector<string> tokenize(string s) {
    vector<string> w;
    for(ull i = 0; i < s.size();) {
        if(!wordChar(s[i])) { i++; continue; }
        string t;
        while(i < s.size()) {
            if(wordChar(s[i])) t += tolower((unsigned char)s[i]), i++;
            else if(s[i] == '\'' && i + 1 < s.size() && wordChar(s[i + 1]) && wordChar(t.back())) t += '\'', i++;
            else break;
        }
        w.push_back(t);
    }
    return w;
}
// 读停用词表。| 开头的行是注释，词后面也可能跟着 | 写说明
set<string> loadStop() {
    set<string> s;
    for(string line : split(readFile("resources/stopwords.txt"), '\n')) {
        ull a = line.find_first_not_of(" \t\r");
        if(a != string::npos && line[a] != '|')
            s.insert(line.substr(a, line.find_first_of(" \t\r", a) - a));
    }
    return s;
}
// 位置串形如 "91:4,120:0"，逗号数加一就是 tf
ull tfOf(string s) {
    ull n = 1;
    for(char c : s) if(c == ',') n++;
    return n;
}
// "行:列" 打包成 行*2^20+列。这样判断两个词挨着只要 x+1，取行号只要 x>>20，
// 不用去解析字符串。（一行不可能有 2^20 个词，所以不会串行）
ull enc(string loc) {
    ull c = loc.find(':');
    return num(loc.substr(0, c)) * 1048576 + num(loc.substr(c + 1));
}
string esc(string s) {
    string o;
    for(char c : s) {
        if(c == '"' || c == '\\') o += '\\', o += c;
        else if((unsigned char)c < 32) o += ' ';
        else o += c;
    }
    return o;
}
// 取文件第 n 行（从 1 开始）。这里故意不用 readFile，那个会把 stdin 抢走，
// 交互模式下读完摘要就再也读不到下一条查询了
string lineAt(string path, ull n) {
    FILE* f = fopen(path.c_str(), "r");
    if(!f) return "";
    string s;
    ull k = 1;
    int c;
    while((c = fgetc(f)) != EOF) {
        if(c == '\n') { if(k == n) break; k++, s.clear(); }
        else if(k == n) s += (char)c;
    }
    fclose(f);
    return s;
}
