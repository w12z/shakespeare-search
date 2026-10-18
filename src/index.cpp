#include<bits/stdc++.h>
#include"porter_stem.h"
using namespace std;
typedef unsigned long long ull;
struct Hit { 
    string term; 
    ull doc, line, pos; 
};
string readFile(string path) {
    if(!freopen(path.c_str(), "r", stdin)) return "";
    cin.clear();
    return string(istreambuf_iterator<char>(cin), istreambuf_iterator<char>());
}
void writeFile(string path, string text) {
    if(!freopen(path.c_str(), "w", stdout)) return;
    cout.clear(), cout << text, cout.flush();
}
ull num(string s) {
    return strtoull(s.c_str(), 0, 10);
}
string stem(string w) {
    if(w.empty()) return w;
    vector<char> buf(w.begin(), w.end());
    buf.push_back('\0');
    int k = porter_stem(buf.data(), 0, (int)w.size() - 1);
    return string(buf.data(), k + 1);
}
int main() {
    string text = readFile("index/tokens.tsv"), line;
    vector<Hit> hits;
    for(ull i = 0; i <= text.size(); i++) {
        if(i < text.size() && text[i] != '\n') { line += text[i]; continue; }
        if(!line.empty()) {
            ull t1 = line.find('\t'), t2 = line.find('\t', t1 + 1), t3 = line.find('\t', t2 + 1);
            hits.push_back({stem(line.substr(t3 + 1)), num(line.substr(0, t1)),
                            num(line.substr(t1 + 1, t2 - t1 - 1)),
                            num(line.substr(t2 + 1, t3 - t2 - 1))});
        }
        line.clear();
    }
    sort(hits.begin(), hits.end(), [](Hit a, Hit b) {
        if(a.term != b.term) return a.term < b.term;
        if(a.doc != b.doc) return a.doc < b.doc;
        if(a.line != b.line) return a.line < b.line;
        return a.pos < b.pos;
    });
    string out;
    for(ull i = 0; i < hits.size();) {
        ull j = i, tf = 0;
        string where;
        while(j < hits.size() && hits[j].term == hits[i].term && hits[j].doc == hits[i].doc) {
            if(tf) where += ",";
            where += to_string(hits[j].line) + ":" + to_string(hits[j].pos);
            tf++, j++;
        }
        out += hits[i].term + "\t" + to_string(hits[i].doc) + "\t" + to_string(tf) + "\t" + where + "\n";
        i = j;
    }
    writeFile("index/inverted.tsv", out);
}
