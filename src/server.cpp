#include"engine.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
string json(string q) {
    Result r = search(q);
    string out;
    for(ull i = 0; i < r.rows.size(); i++) {
        Row h = r.rows[i];
        char sc[32];
        snprintf(sc, sizeof(sc), "%.4f", h.score);
        string u = path[h.doc];
        u = u.substr(0, u.rfind('.')) + ".html";
        out += i ? "," : "";
        out += "{\"score\":" + string(sc) + ",\"cos\":" + to_string(h.cs);
        out += ",\"near\":\"" + to_string(h.near) + "/" + to_string(r.pairs) + "\"";
        out += ",\"title\":\"" + esc(title[h.doc]) + "\",\"url\":\"/" + u + "\"";
        out += ",\"line\":" + to_string(h.line);
        out += ",\"text\":\"" + esc(lineAt("corpus/" + path[h.doc], h.line)) + "\"}";
    }
    return "{\"query\":\"" + esc(q) + "\",\"count\":" + to_string(r.total) + ",\"hits\":[" + out + "]}";
}
string urlDecode(string s) {
    string o;
    for(ull i = 0; i < s.size(); i++) {
        if(s[i] == '+') o += ' ';
        else if(s[i] == '%' && i + 2 < s.size()) o += (char)strtol(s.substr(i + 1, 2).c_str(), 0, 16), i += 2;
        else o += s[i];
    }
    return o;
}
string mime(string p) {
    ull d = p.rfind('.');
    string e = d == string::npos ? "" : p.substr(d);
    return e == ".html" ? "text/html" : e == ".css" ? "text/css"
         : e == ".gif" ? "image/gif" : e == ".ico" ? "image/x-icon" : "text/plain";
}
void sendAll(int fd, string s) {
    for(ull off = 0; off < s.size();) {
        ssize_t n = write(fd, s.data() + off, s.size() - off);
        if(n <= 0) return;
        off += n;
    }
}
int main() {
    loadIndex();
    printf("loaded %llu documents, %llu terms\n", (ull)path.size(), (ull)post.size());
    int s = socket(AF_INET, SOCK_STREAM, 0), one = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in a{};
    a.sin_family = AF_INET, a.sin_addr.s_addr = INADDR_ANY, a.sin_port = htons(8080);
    bind(s, (sockaddr*)&a, sizeof(a));
    listen(s, 16);
    printf("http://localhost:8080/\n");
    fflush(stdout);
    while(1) {
        int c = accept(s, 0, 0);
        if(c < 0) continue;
        string req;
        char buf[8192];
        while(req.find("\r\n\r\n") == string::npos && req.size() < 65536) {
            ssize_t n = read(c, buf, sizeof(buf));
            if(n <= 0) break;
            req.append(buf, n);
        }
        string t;
        ull p1 = req.find(' '), p2 = req.find(' ', p1 + 1);
        if(p1 != string::npos && p2 != string::npos) t = req.substr(p1 + 1, p2 - p1 - 1);
        string body, ct;
        ull qm = t.find('?');
        string route = qm == string::npos ? t : t.substr(0, qm);
        if(route == "/") {
            body = readFile("web/search.html"), ct = "text/html";
        } else if(route == "/api/search") {
            string a = t.substr(qm == string::npos ? t.size() : qm + 1);
            body = json(a.rfind("q=", 0) == 0 ? urlDecode(a.substr(2)) : ""), ct = "application/json";
        } else if(t.find("..") == string::npos) {
            body = readFile("web" + route), ct = mime(route);
        }
        if(body.empty()) body = "not found", ct = "text/plain";
        sendAll(c, "HTTP/1.1 200 OK\r\nContent-Type: " + ct + "; charset=utf-8\r\nContent-Length: "
                   + to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
        close(c);
    }
}
