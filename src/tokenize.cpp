#include<bits/stdc++.h>
#include<dirent.h>
#include<sys/stat.h>
using namespace std;
typedef unsigned long long ull;
string readFile(string path) {
    if(!freopen(path.c_str(), "r", stdin)) return "";
    cin.clear();
    return string(istreambuf_iterator<char>(cin), istreambuf_iterator<char>());
}
void writeFile(string path, string text) {
    if(!freopen(path.c_str(), "w", stdout)) return;
    cout.clear(), cout << text, cout.flush();
}
bool wordChar(char c) {
    return isalnum((unsigned char)c) != 0;
}
void walk(string dir, string rel, vector<string>& out) {
    DIR* d = opendir(dir.c_str());
    if(!d) return;
    vector<string> names;
    dirent* e;
    while((e = readdir(d)))
        if(string(e->d_name) != "." && string(e->d_name) != "..") names.push_back(e->d_name);
    closedir(d), sort(names.begin(), names.end());
    for(ull i = 0; i < names.size(); i++) {
        string full = dir + "/" + names[i];
        string r = rel.empty() ? names[i] : rel + "/" + names[i];
        struct stat st;
        if(stat(full.c_str(), &st) != 0) continue;
        if(S_ISDIR(st.st_mode)) walk(full, r, out);
        else out.push_back(r);
    }
}
int main() {
    string inDir = "corpus", outDir = "index";
    vector<string> all, files;
    walk(inDir, "", all);
    for(ull i = 0; i < all.size(); i++)
        if(all[i].substr(all[i].size() - 4) == ".txt") files.push_back(all[i]);
    sort(files.begin(), files.end());
    string docs, tokens;
    for(ull id = 0; id < files.size(); id++) {
        string rel = files[id], text = readFile(inDir + "/" + rel);
        string title;
        ull lineNo = 0, i = 0;
        while(i < text.size()) {
            string line;
            while(i < text.size() && text[i] != '\n') line += text[i++];
            i++, lineNo++;
            if(lineNo == 1) title = line;
            if(line.empty()) continue;
            ull pos = 0, p = 0;
            while(p < line.size()) {
                if(!wordChar(line[p])) { p++; continue; }
                string w;
                while(p < line.size()) {
                    if(wordChar(line[p])) w += tolower((unsigned char)line[p]), p++;
                    else if(line[p] == '\'' && p + 1 < line.size() && wordChar(line[p + 1]) && wordChar(w.back()))
                        w += '\'', p++;
                    else break;
                }
                tokens += to_string(id) + "\t" + to_string(lineNo) + "\t" + to_string(pos) + "\t" + w + "\n";
                pos++;
            }
        }
        if(title.empty()) title = rel;
        docs += to_string(id) + "\t" + rel + "\t" + title + "\n";
    }
    mkdir(outDir.c_str(), 0755);
    writeFile(outDir + "/docs.tsv", docs), writeFile(outDir + "/tokens.tsv", tokens);
}
