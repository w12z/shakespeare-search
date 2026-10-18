#include"engine.h"
#include<unistd.h>
int main() {
    int tty = dup(0);
    loadIndex();
    dup2(tty, 0), close(tty), clearerr(stdin);
    printf("loaded %llu documents, %llu terms\n", (ull)path.size(), (ull)post.size());
    string q;
    while(1) {
        printf("> ");
        fflush(stdout);
        cin.clear();
        if(!getline(cin, q) || q.empty()) break;
        Result r = search(q);
        printf("\n%llu hits\n\n", r.total);
        for(ull i = 0; i < r.rows.size(); i++) {
            Row h = r.rows[i];
            printf("[%llu] score=%.4f  cos=%.4f  near=%llu/%llu  %s  line %llu\n    %s\n    %s\n\n",
                   i + 1, h.score, h.cs, h.near, r.pairs, path[h.doc].c_str(), h.line,
                   title[h.doc].c_str(), lineAt("corpus/" + path[h.doc], h.line).c_str());
        }
    }
}
