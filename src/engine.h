#pragma once
#include"text.h"
struct Row { double score, cs; ull doc, near, line; };
struct Result { ull total, pairs; vector<Row> rows; };
vector<string> path, title;
vector<double> sq;
set<string> stop;
unordered_map<string, vector<pair<ull, string>>> post;
void loadIndex() {
    stop = loadStop();
    for(string l : split(readFile("index/docs.tsv"), '\n')) {
        if(l.empty()) continue;
        ull t1 = l.find('\t'), t2 = l.find('\t', t1 + 1);
        path.push_back(l.substr(t1 + 1, t2 - t1 - 1)), title.push_back(l.substr(t2 + 1));
    }
    sq.assign(path.size(), 0);
    for(string l : split(readFile("index/inverted.tsv"), '\n')) {
        if(l.empty()) continue;
        ull t1 = l.find('\t'), t2 = l.find('\t', t1 + 1), t3 = l.find('\t', t2 + 1);
        ull doc = num(l.substr(t1 + 1, t2 - t1 - 1)), tf = num(l.substr(t2 + 1, t3 - t2 - 1));
        sq[doc] += (double)tf * tf;
        post[l.substr(0, t1)].push_back({doc, l.substr(t3 + 1)});
    }
}
Result search(string q) {
    Result res{};
    vector<string> raw = tokenize(q), all, seq;
    for(string w : raw) all.push_back(stem(w));
    for(ull i = 0; i < raw.size(); i++) if(!stop.count(raw[i])) seq.push_back(all[i]);
    if(seq.empty()) seq = all;
    if(all.empty()) return res;
    map<string, ull> qv;
    double qn = 0;
    for(string w : seq) qv[w]++;
    for(auto kv : qv) qn += (double)kv.second * kv.second;
    qn = sqrt(qn);
    map<string, map<ull, set<ull>>> pos;
    set<string> need(all.begin(), all.end());
    for(string t : need) {
        auto it = post.find(t);
        if(it == post.end()) continue;
        for(auto p : it->second) {
            set<ull> s;
            for(string loc : split(p.second, ',')) s.insert(enc(loc));
            pos[t][p.first] = s;
        }
    }
    ull pairs = all.size() - 1;
    res.pairs = pairs;
    map<ull, ull> near;
    map<ull, map<ull, ull>> linehits;
    for(ull i = 0; i + 1 < all.size(); i++) {
        auto a = pos.find(all[i]), b = pos.find(all[i + 1]);
        if(a == pos.end() || b == pos.end()) continue;
        for(auto kv : a->second) {
            auto jt = b->second.find(kv.first);
            if(jt == b->second.end()) continue;
            for(ull x : kv.second)
                if(jt->second.count(x + 1)) {
                    near[kv.first]++, linehits[kv.first][x >> 20]++;
                    break;
                }
        }
    }
    map<ull, double> ip;
    map<ull, ull> bodyline;
    for(auto kv : qv) {
        auto it = post.find(kv.first);
        if(it == post.end()) continue;
        for(auto p : it->second) {
            ull doc = p.first;
            ip[doc] += (double)kv.second * tfOf(p.second);
            for(string loc : split(p.second, ',')) {
                ull l = enc(loc) >> 20;
                if(l > 1 && (!bodyline.count(doc) || l < bodyline[doc])) bodyline[doc] = l;
            }
        }
    }
    vector<Row> rank;
    for(auto kv : ip) {
        double c = kv.second / (qn * sqrt(sq[kv.first]));
        if(c > 1) c = 1;
        if(c < 0) c = 0;
        rank.push_back({c + (pairs ? 0.5 * near[kv.first] / pairs : 0), c, kv.first, near[kv.first], 1});
    }
    sort(rank.begin(), rank.end(), [](Row a, Row b) {
        return a.score != b.score ? a.score > b.score : a.doc < b.doc;
    });
    res.total = rank.size();
    if(rank.size() > 10) rank.resize(10);
    for(Row& r : rank) {
        ull ln = 0, best = 0;
        for(auto kv : linehits[r.doc]) if(kv.second > best) best = kv.second, ln = kv.first;
        r.line = ln ? ln : (bodyline.count(r.doc) ? bodyline[r.doc] : 1);
    }
    res.rows = rank;
    return res;
}
