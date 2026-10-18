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
// 整个字符串写到文件里
void writeFile(string path, string text) {
    if(!freopen(path.c_str(), "w", stdout)) return;
    cout.clear(), cout << text, cout.flush();
}
string lower(string s) {
    for(ull i = 0; i < s.size(); i++) s[i] = tolower((unsigned char)s[i]);
    return s;
}
// HTML 实体解码。常见的具名实体查表，&#39; 这种数字的用 strtol 解
string decode(string s) {
    map<string, string> named;
    named["&amp;"] = "&", named["&lt;"] = "<", named["&gt;"] = ">";
    named["&quot;"] = "\"", named["&apos;"] = "'", named["&nbsp;"] = " ";
    named["&mdash;"] = "--", named["&ndash;"] = "-", named["&rsquo;"] = "'";
    named["&lsquo;"] = "'", named["&ldquo;"] = "\"", named["&rdquo;"] = "\"";
    string out;
    for(ull i = 0; i < s.size();) {
        if(s[i] != '&') { out += s[i], i++; continue; }
        ull e = s.find(';', i);
        if(e == string::npos || e - i > 10) { out += s[i], i++; continue; }
        string ent = lower(s.substr(i, e - i + 1));
        if(named.count(ent)) { out += named[ent], i = e + 1; continue; }
        if(ent.size() > 3 && ent[1] == '#') {
            bool hex = ent[2] == 'x';
            string digits = ent.substr(hex ? 3 : 2, ent.size() - (hex ? 4 : 3));
            char* end;
            long code = strtol(digits.c_str(), &end, hex ? 16 : 10);
            if(*end == 0 && code > 0 && code < 256) { out += (char)code, i = e + 1; continue; }
        }
        out += s[i], i++;
    }
    return out;
}
// 数有多少个词。这里把撇号和连字符也算词内字符，所以 to-night 记 1 个而不是 2 个
ull countWords(string s) {
    ull n = 0;
    bool in = false;
    for(ull i = 0; i < s.size(); i++) {
        bool w = isalpha((unsigned char)s[i]) || s[i] == '\'' || s[i] == '-';
        if(w && !in) n++;
        in = w;
    }
    return n;
}
struct Page { string text; ull words; };
// 一页 HTML 转成正文
Page parsePage(string html) {
    // 先正则删掉整块：注释、head/script/style、顶部那个 class="nav" 的导航表格
    html = regex_replace(html, regex("<!--[\\s\\S]*?-->"), " ");
    for(string tag : {"head", "script", "style"})
        html = regex_replace(html, regex("<\\s*" + tag + "\\b[^>]*>[\\s\\S]*?<\\s*/\\s*" + tag + "\\s*>", regex::icase), " ");
    html = regex_replace(html, regex("<td\\b[^>]*class\\s*=\\s*[\\\"']?nav[\\\"']?[^>]*>[\\s\\S]*?<\\s*/\\s*table\\s*>", regex::icase), " ");
    Page p;
    // 剩下的标签用扫描丢掉：见到 < 就跳到对应的 >。
    // 注意跳过的标签不补空格——源文件里有 match</A>'d 这种被标签劈开的词，
    // 补空格会变成 "match 'd"，什么都不补才拼得回 match'd
    string t;
    for(ull i = 0; i < html.size();) {
        if(html[i] != '<') { t += html[i], i++; continue; }
        ull e = html.find('>', i);
        string tag = lower(html.substr(i, e == string::npos ? html.size() - i : e - i + 1));
        // 块级标签要额外吐一个换行，后面摘要按行取
        for(string b : {"<br", "<p", "<h", "<hr", "<tr", "<td", "</blockquote", "</p"})
            if(tag.compare(0, b.size(), b) == 0) { t += '\n'; break; }
        i = (e == string::npos) ? html.size() : e + 1;
    }
    t = decode(t);
    // 把连续空白压成一个空格，连续换行压成一个换行
    string out;
    bool nl = false, sp = false;
    for(ull i = 0; i < t.size(); i++) {
        char c = t[i];
        if(c == '\n' || c == '\r') { nl = true, sp = false; continue; }
        if(isspace((unsigned char)c)) { if(!nl) sp = true; continue; }
        if(nl && !out.empty()) out += '\n';
        else if(sp && !out.empty()) out += ' ';
        nl = sp = false, out += c;
    }
    p.text = out + "\n", p.words = countWords(out);
    return p;
}
// 递归列出目录下所有文件的相对路径
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
// 逐级建目录
void makeDirs(string path) {
    string cur;
    for(ull i = 0; i < path.size(); i++) {
        cur += path[i];
        if(path[i] == '/') mkdir(cur.c_str(), 0755);
    }
}
// 遍历 web/ 下所有 html，抽取正文写到 corpus/
int main() {
    string inDir = "web", outDir = "corpus";
    vector<string> all, files;
    walk(inDir, "", all);
    for(ull i = 0; i < all.size(); i++)
        if(all[i].substr(all[i].size() - 5) == ".html") files.push_back(all[i]);
    auto mirror = [](const string& p) {
        return p.find("Comedy/") == 0 || p.find("History/") == 0 || p.find("Tragedy/") == 0;
    };
    sort(files.begin(), files.end(), [&](string a, string b) {
        if(mirror(a) != mirror(b)) return mirror(b);
        return a < b;
    });
    // 全文页不要（和分场文件内容重复）。目录页留着——搜到目录页也是有用的
    set<string> skip = {"full.html", "news.html", "test.html"};
    set<ull> seen;
    for(ull i = 0; i < files.size(); i++) {
        string rel = files[i], name = rel.substr(rel.find_last_of('/') + 1);
        if(skip.count(lower(name))) continue;
        // 内容相同的只留一份
        string raw = readFile(inDir + "/" + rel);
        ull h = hash<string>{}(raw);
        if(seen.count(h)) continue;
        seen.insert(h);
        Page p = parsePage(raw);
        if(p.words == 0) continue;
        string dst = outDir + "/" + rel;
        dst = dst.substr(0, dst.find_last_of('.')) + ".txt";
        makeDirs(dst), writeFile(dst, p.text);
    }
}
