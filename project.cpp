#include <iostream>
#include <regex>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <cstdio>
#include <set>
typedef struct tree_node
{
    std::string filename;
    std::vector<std::shared_ptr<tree_node> > child;
    std::shared_ptr<tree_node> father;
} node;
using namespace std::filesystem;
std::string c_header[] = {"assert.h", "limits.h", "signal.h", "stdlib.h", "ctype.h", "locale.h", "stdarg.h", "string.h", "errno.h", "math.h", "stddef.h", "time.h", "float.h", "setjmp.h", "stdio.h", "iso646.h", "wchar.h", "wctype.h", "complex.h", "inttypes.h", "stdint.h", "tgmath.h", "fenv.h", "stdbool.h", "stdalign.h", "stdatomic.h", "stdnoreturn.h", "threads.h", "uchar.h", "aio.h", "libgen.h", "spawn.h", "sys/time.h", "arpa/inet.h", "limits.h", "stdarg.h", "sys/times.h", "assert.h", "locale.h", "stdbool.h", "sys/types.h", "complex.h", "math.h", "stddef.h", "sys/uio.h", "cpio.h", "monetary.h", "stdint.h", "sys/un.h", "ctype.h", "mqueue.h", "sys/utsname.h", "dirent.h", "ndbm.h", "stdlib.h", "sys/wait.h", "dlfcn.h", "net/if.h", "string.h", "syslog.h", "errno.h", "netdb.h", "strings.h", "tar.h", "fcntl.h", "netinet/in.h", "stropts.h", "termios.h", "fenv.h", "netinet/tcp.h", "sys/ipc.h", "tgmath.h", "float.h", "nl_types.h", "sys/mman.h", "time.h", "fmtmsg.h", "poll.h", "sys/msg.h", "trace.h", "fnmatch.h", "pthread.h", "sys/resource.h", "ulimit.h", "ftw.h", "pwd.h", "sys/select.h", "unistd.h", "glob.h", "regex.h", "sys/sem.h", "utime.h", "grp.h", "sched.h", "sys/shm.h", "utmpx.h", "iconv.h", "search.h", "sys/socket.h", "wchar.h", "inttypes.h", "semaphore.h", "sys/stat.h", "wctype.h", "iso646.h", "setjmp.h", "sys/statvfs.h", "wordexp.h", "langinfo.h", "signal.h"};
std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}
std::vector<std::string> read_include(std::string path)
{
    std::vector<std::string> res;
    std::ifstream in(path);
    std::regex include_regex("#include.*");
    std::string s;
    if (!in.is_open())
    {
        std::cerr << "open file failed" << std::endl;
    }
    while (std::getline(in, s))
    {
        if (std::regex_match(s, include_regex))
        {
            s.erase(remove(s.begin(), s.end(), '\"'), s.end());
            s.erase(remove(s.begin(), s.end(), '\"'), s.end());
            s.erase(remove(s.begin(), s.end(), ' '), s.end());
            s.erase(0, 8);
            s.erase(remove(s.begin(), s.end(), '<'), s.end());
            s.erase(remove(s.begin(), s.end(), '>'), s.end());
            res.push_back(s);
        }
    }
    in.close();
    return res;
}
auto list_file(std::string path)
{
    std::vector<std::string> res;
    std::regex valid_filename(".+\\.cpp||.+\\.hpp||.+\\.c||.+\\.h");
    for (const auto &entry : directory_iterator(path))
    {
        std::string s = entry.path().filename().u8string();
        if (std::regex_match(s, valid_filename))
        {
            res.push_back(s);
        }
    }
    return res;
}
std::shared_ptr<node>  tree_create(std::string path, std::shared_ptr<node> parent  = nullptr)
{
    auto root = std::make_shared<node>();
    root->filename = path;
    root->father = parent;
    for (auto &c : read_include(path))
    {   
        if(std::find(c_header,c_header+111,c)!=c_header+111){
            std::shared_ptr<node>  n=std::make_shared<node>();
            n->filename=c;
            n->father=root;
            root->child.push_back(n);
            continue;
        }
        root->child.push_back(tree_create(c, root));
    }
    return root;
}
void write_image(std::ofstream &out, std::shared_ptr<node> n)
{
    for (auto c : n->child)
    {
        out << "\"" << n->filename << "\""
            << " -> "
            << "\"" << c->filename << "\""
            << ";" << std::endl;
        write_image(out, c);
    }
}
void add_filename(std::set<std::string> &child_file,std::shared_ptr<node> root){
    for(auto c:root->child){
        child_file.emplace(c->filename);
        add_filename(child_file,c);
    }
}
void generate_image(std::shared_ptr<node> root)
{
    std::ofstream outfile;
    outfile.open("./write.dot", std::ios::out);
    outfile << "digraph {" << std::endl;
    for (auto c : root->child)
    {
        outfile << "\"" << root->filename << "\""
                << " -> "
                << "\"" << c->filename << "\""
                << ";" << std::endl;
        write_image(outfile, c);
    }
    outfile << "}" << std::endl;
    outfile.close();
    exec("dot -Tsvg write.dot -o output.svg");
}
int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        std::cerr << "fatal error: can't find the input files" << std::endl;
    }
    else if (argc == 2)
    {
        std::shared_ptr<node> root = tree_create(argv[1]);
        generate_image(root);
    }
    else if (argc==3){
        std::shared_ptr<node> root = tree_create(argv[1]);
        std::set<std::string> child_file;
        add_filename(child_file,root);
        if(std::find(child_file.begin(),child_file.end(),argv[2])!=child_file.end()){
            std::cout<<"already include this file"<<std::endl;
            return 0;
        }else{
            std::cout<<"doesn't include this file"<<std::endl;
        }
    }
}