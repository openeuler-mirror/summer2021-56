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
#include <sys/resource.h>
#include <omp.h>
#include <map>
typedef struct tree_node {
	std::string filename;
	std::vector<std::shared_ptr<tree_node> > child;
	std::shared_ptr<tree_node> father;
} node;
int count_runtimes = 0;
std::map<std::string, std::shared_ptr<node> > file_map;
std::set<std::string> write_set;
using namespace std::filesystem;
std::string c_header[] = {
	"assert.h",    "limits.h",	"signal.h",	  "stdlib.h",
	"ctype.h",     "locale.h",	"stdarg.h",	  "string.h",
	"errno.h",     "math.h",	"stddef.h",	  "time.h",
	"float.h",     "setjmp.h",	"stdio.h",	  "iso646.h",
	"wchar.h",     "wctype.h",	"complex.h",	  "inttypes.h",
	"stdint.h",    "tgmath.h",	"fenv.h",	  "stdbool.h",
	"stdalign.h",  "stdatomic.h",	"stdnoreturn.h",  "threads.h",
	"uchar.h",     "aio.h",		"libgen.h",	  "spawn.h",
	"sys/time.h",  "arpa/inet.h",	"limits.h",	  "stdarg.h",
	"sys/times.h", "assert.h",	"locale.h",	  "stdbool.h",
	"sys/types.h", "complex.h",	"math.h",	  "stddef.h",
	"sys/uio.h",   "cpio.h",	"monetary.h",	  "stdint.h",
	"sys/un.h",    "ctype.h",	"mqueue.h",	  "sys/utsname.h",
	"dirent.h",    "ndbm.h",	"stdlib.h",	  "sys/wait.h",
	"dlfcn.h",     "net/if.h",	"string.h",	  "syslog.h",
	"errno.h",     "netdb.h",	"strings.h",	  "tar.h",
	"fcntl.h",     "netinet/in.h",	"stropts.h",	  "termios.h",
	"fenv.h",      "netinet/tcp.h", "sys/ipc.h",	  "tgmath.h",
	"float.h",     "nl_types.h",	"sys/mman.h",	  "time.h",
	"fmtmsg.h",    "poll.h",	"sys/msg.h",	  "trace.h",
	"fnmatch.h",   "pthread.h",	"sys/resource.h", "ulimit.h",
	"ftw.h",       "pwd.h",		"sys/select.h",	  "unistd.h",
	"glob.h",      "regex.h",	"sys/sem.h",	  "utime.h",
	"grp.h",       "sched.h",	"sys/shm.h",	  "utmpx.h",
	"iconv.h",     "search.h",	"sys/socket.h",	  "wchar.h",
	"inttypes.h",  "semaphore.h",	"sys/stat.h",	  "wctype.h",
	"iso646.h",    "setjmp.h",	"sys/statvfs.h",  "wordexp.h",
	"langinfo.h",  "signal.h"
};
std::string linux_path[] = { "../../include/", "./include/"
					       "/include/" };
std::string exec(const char *cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}
std::vector<std::string> read_include(std::string path)
{
	std::vector<std::string> res;
	if (count_runtimes++ >= 20000)
		return res;
	std::regex linuix_file_regex("[a-z]+/.*");
	std::ifstream in(path);
	std::regex include_regex("#include.*");
	std::string s;
	int cnt = 0;
	while (!in.is_open() && cnt < 14) {
		in.open(linux_path[cnt++] + path);
	}
	if (!in.is_open()) {
		//std::cerr<<"can't open the file : "<<path<<std::endl;
	}
	while (std::getline(in, s)) {
		s.erase(remove(s.begin(), s.end(), ' '), s.end());
		if (std::regex_match(s, include_regex)) {
			s.erase(remove(s.begin(), s.end(), '\"'), s.end());
			s.erase(remove(s.begin(), s.end(), '\"'), s.end());
			s.erase(remove(s.begin(), s.end(), '<'), s.end());
			s.erase(remove(s.begin(), s.end(), '>'), s.end());
			s.erase(0, 8);
			res.push_back(s);
		} else if (s != "" && s[0] != '/' && s[0] != '*' && s[0] != '#')
			break;
	}
	in.close();
	return res;
}
auto list_file(std::string path)
{
	std::vector<std::string> res;
	std::regex valid_filename(".+\\.cpp||.+\\.hpp||.+\\.c||.+\\.h");
	for (const auto &entry : directory_iterator(path)) {
		std::string s = entry.path().filename().u8string();
		if (std::regex_match(s, valid_filename) && s != "project.cpp") {
			res.push_back(s);
		}
	}
	return res;
}
std::shared_ptr<node> tree_create(std::string path,
				  std::shared_ptr<node> parent = nullptr)
{
	auto root = std::make_shared<node>();
	if (count_runtimes >= 20000)
		return root;
	root->filename = path;
	root->father = parent;
	auto vec = read_include(path);
	int limit = vec.size();
#pragma omp parallel for
	for (int i = 0; i < limit; i++) {
		if (std::find(c_header, c_header + 111, vec[i]) ==
		    c_header + 111) {
			auto it = file_map.find(vec[i]);
			if (it != file_map.end()) {
				root->child.push_back(it->second);
				continue;
			}
			std::shared_ptr<node> n = std::make_shared<node>();
			n->filename = vec[i];
			n->father = root;
			auto node = tree_create(vec[i], n);
			root->child.push_back(node);
			file_map[vec[i]] = node;
		}
	}
	return root;
}
void write_image(std::ofstream &out, std::shared_ptr<node> n)
{
	int limit = n->child.size();
#pragma omp parallel for
	for (int i = 0; i < limit; i++) {
#pragma omp critical
		{
			out << "\"" << n->filename << "\""
			    << " -> "
			    << "\"" << n->child[i]->filename << "\""
			    << ";" << std::endl;
		}
		if (write_set.find(n->child[i]->filename) != write_set.end())
			continue;
		write_set.emplace(n->child[i]->filename);
		write_image(out, n->child[i]);
	}
}
void add_filename(std::set<std::string> &child_file, std::shared_ptr<node> root)
{
	for (auto c : root->child) {
		child_file.emplace(c->filename);
		add_filename(child_file, c);
	}
}

void generate_image(std::shared_ptr<node> root)
{
	std::ofstream outfile;
	outfile.open("./write.dot", std::ios::out);
	outfile << "digraph {" << std::endl;
	for (auto c : root->child) {
		outfile << "\"" << root->filename << "\""
			<< " -> "
			<< "\"" << c->filename << "\""
			<< ";" << std::endl;
		write_image(outfile, c);
	}
	outfile << "}" << std::endl;
	outfile.close();
}
int main(int argc, char *argv[])
{
	if (argc == 1) {
		//std::cerr << "fatal error: can't find the input files" << std::endl;
		auto vec = list_file("./");
		for (auto c : vec) {
			tree_create(std::string("./") + c);
		}
		exec("dot -Tsvg write.dot -o output.svg");
	} else if (argc == 2) {
		std::shared_ptr<node> root = tree_create(argv[1]);
		generate_image(root);
		exec("dot -Tsvg write.dot -o output.svg");
	} else if (argc == 3) {
		std::shared_ptr<node> root = tree_create(argv[1]);
		std::set<std::string> child_file;
		add_filename(child_file, root);
		if (std::find(child_file.begin(), child_file.end(), argv[2]) !=
		    child_file.end()) {
			std::cout << "already include this file" << std::endl;
			return 0;
		} else {
			std::cout << "doesn't include this file" << std::endl;
		}
	}
}