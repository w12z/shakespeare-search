#include"engine.h"
#include<unistd.h>
// 交互式检索。索引在启动时读一次，之后一直读查询，空行或 Ctrl-D 退出
int main() {
    // loadIndex 里每个 readFile 都会把 stdin 重定向到文件，先把真正的 stdin 存一份，
    // 读完再换回来，不然下面第一轮就读不到输入了
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
