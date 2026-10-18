#pragma once
#include<bits/stdc++.h>
#include"porter_stem.h"
using namespace std;
typedef unsigned long long ull;
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
set<string> loadStop() {
    set<string> s;
    for(string line : split(readFile("resources/stopwords.txt"), '\n')) {
        ull a = line.find_first_not_of(" \t\r");
        if(a != string::npos && line[a] != '|')
            s.insert(line.substr(a, line.find_first_of(" \t\r", a) - a));
    }
    return s;
}
ull tfOf(string s) {
    ull n = 1;
    for(char c : s) if(c == ',') n++;
    return n;
}
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
