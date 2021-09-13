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
#define ARCH x86
#define STR1(x) #x
#define STR(x) STR1(x)
#define CAT(x, y) x##y
#define COMBINE(x, y) CAT(x, y)
typedef struct tree_node
{
	std::string filename;
	std::vector<std::shared_ptr<tree_node>> child;
} node;
int count_runtimes = 0;
std::map<std::string, std::shared_ptr<node>> file_map;
std::set<std::string> write_set;
using namespace std::filesystem;
std::string c_header[] = {
	"assert.h", "limits.h", "signal.h", "stdlib.h",
	"ctype.h", "locale.h", "stdarg.h", "string.h",
	"errno.h", "math.h", "stddef.h", "time.h",
	"float.h", "setjmp.h", "stdio.h", "iso646.h",
	"wchar.h", "wctype.h", "complex.h", "inttypes.h",
	"stdint.h", "tgmath.h", "fenv.h", "stdbool.h",
	"stdalign.h", "stdatomic.h", "stdnoreturn.h", "threads.h",
	"uchar.h", "aio.h", "libgen.h", "spawn.h",
	"sys/time.h", "arpa/inet.h", "limits.h", "stdarg.h",
	"sys/times.h", "assert.h", "locale.h", "stdbool.h",
	"sys/types.h", "complex.h", "math.h", "stddef.h",
	"sys/uio.h", "cpio.h", "monetary.h", "stdint.h",
	"sys/un.h", "ctype.h", "mqueue.h", "sys/utsname.h",
	"dirent.h", "ndbm.h", "stdlib.h", "sys/wait.h",
	"dlfcn.h", "net/if.h", "string.h", "syslog.h",
	"errno.h", "netdb.h", "strings.h", "tar.h",
	"fcntl.h", "netinet/in.h", "stropts.h", "termios.h",
	"fenv.h", "netinet/tcp.h", "sys/ipc.h", "tgmath.h",
	"float.h", "nl_types.h", "sys/mman.h", "time.h",
	"fmtmsg.h", "poll.h", "sys/msg.h", "trace.h",
	"fnmatch.h", "pthread.h", "sys/resource.h", "ulimit.h",
	"ftw.h", "pwd.h", "sys/select.h", "unistd.h",
	"glob.h", "regex.h", "sys/sem.h", "utime.h",
	"grp.h", "sched.h", "sys/shm.h", "utmpx.h",
	"iconv.h", "search.h", "sys/socket.h", "wchar.h",
	"inttypes.h", "semaphore.h", "sys/stat.h", "wctype.h",
	"iso646.h", "setjmp.h", "sys/statvfs.h", "wordexp.h",
	"langinfo.h", "signal.h"};
std::string linux_path[6];
// void confirm_path(const char *c)
// {
// 	auto s = std::string(c);
// 	linux_path.push_back(std::string("../arch/") + s +
// 			     std::string("/include/"));
// 	linux_path.push_back(std::string("../../arch/") + s +
// 			     std::string("/include/"));
// 	linux_path.push_back(std::string(".../../../arch/") + s +
// 			     std::string("/include/"));
// 	linux_path.push_back(std::string("/arch/") + s +
// 			     std::string("/include/"));
// }
int PATH_SIZE = 6;
void split(const std::string &s, std::vector<std::string> &tokens,
		   const std::string &delimiters = " ")
{
	std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	std::string::size_type pos = s.find_first_of(delimiters, lastPos);
	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		tokens.emplace_back(s.substr(lastPos, pos - lastPos));
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}
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
template <typename T>
void print(T data)
{
	for (auto c : data)
	{
		std::cout << c << std::endl;
	}
}
std::vector<std::string> identifier{"#define",
									"int",
									"int*",
									"struct",
									"struct*"
									"void",
									"char",
									"char*",
									"long",
									"long*",
									"enum",
									"union"};
std::string modifier[] = {"static", "inline", "const", "extern",
						  "register", "volatile", "unsigned"};
bool check(std::set<std::string> v1, std::set<std::string> v2)
{
	std::vector<std::string> res;
	std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
						  std::inserter(res, res.begin()));
	return res.empty();
}
std::vector<std::string> check_ret(std::set<std::string> v1, std::set<std::string> v2)
{
	std::vector<std::string> res;
	std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
						  std::inserter(res, res.begin()));
	return res;
}
std::set<std::string> read_function_declare(std::string path)
{
	std::set<std::string> res;
	if (path.empty())
		return res;
	if (path.substr(path.size() - 2, 2).compare(".h") != 0)
	{
		std::cerr << "input file isn't a header file" << std::endl;
		return res;
	}
	std::ifstream in(path);
	std::string s;
	int cnt = 0;
	while (!in.is_open() && cnt < PATH_SIZE)
	{
		in.open(linux_path[cnt++] + path);
	}
	if (!in.is_open())
	{
		std::cerr << "can't open the file : " << path << std::endl;
		res.emplace("wrong");
		return res;
	}
	cnt = 0;
	while (std::getline(in, s))
	{
		if (s.substr(0, 2).compare("//") == 0 ||
			s.substr(0, 2).compare("/*") == 0 ||
			s.substr(0, 2).compare(" *") == 0)
		{
			continue;
		}
		if (s.substr(0, 7).compare("typedef") == 0)
		{
			std::vector<std::string> sp;
			split(s, sp);
			sp[sp.size() - 1].pop_back();
			res.emplace(sp[sp.size() - 1]);
			identifier.push_back(sp[sp.size() - 1]);
			cnt++;
		}
		for (auto c : modifier)
		{
			if (s.substr(0, c.size()).compare(c.c_str()) == 0)
			{
				s.erase(0, c.size() + 1);
			}
		}
		for (auto c : identifier)
		{
			if (s.substr(0, c.size()).compare(c.c_str()) == 0)
			{
				for (int i = c.size() + 1; i < s.size(); i++)
				{
					if (s[i] == '(' || s[i] == ' ' ||
						s[i] == '\t' || s[i] == '\n' ||
						s[i] == '{')
					{
						res.emplace(s.substr(
							c.size() + 1,
							i - c.size() - 1));
						break;
					}
				}
			}
		}
		while (cnt-- > 0)
		{
			identifier.pop_back();
		}
	}
	return res;
}
std::set<std::string> read_function_used(std::string path)
{
	// exec((std::string("gcc -aux-info output.info ")+path).c_str());
	// auto ret=exec("grep -Ev \"/usr|compiled from\" output.info");
	// exec("rm output.info");
	std::set<std::string> res;
	if (path.empty())
		return res;
	if (path.substr(path.size() - 2, 2).compare(".c") != 0)
	{
		std::cerr << "input file isn't a source file" << std::endl;
		return res;
	}
	std::ifstream in(path);
	std::string s;
	int cnt = 0;
	while (!in.is_open() && cnt < PATH_SIZE)
	{
		in.open(linux_path[cnt++] + path);
	}
	if (!in.is_open())
	{
		std::cerr << "can't open the file : " << path << std::endl;
		return res;
	}
	std::regex call("[a-zA-Z_]+\\(.*\\)");
	std::regex call_search("[a-zA-Z_]+\\(");
	std::smatch result;
	while (std::getline(in, s))
	{
		if (std::regex_search(s, call))
		{
			std::regex_search(s, result, call_search);
			auto str = result[0].str();
			str.pop_back();
			res.emplace(str);
		}
	}
	return res;
}
std::vector<std::string> read_include(std::string path)
{
	std::vector<std::string> res;
	// if (count_runtimes++ >= 20000000)
	// 	return res;
	std::regex linux_file_regex("[a-z]+/.*");
	std::ifstream in(path);
	std::regex include_regex("#include.*.h>||#include.*.h\"");
	std::string s;
	int cnt = 0;
	while (!in.is_open() && cnt < PATH_SIZE)
	{
		//std::cout<<linux_path[cnt] + path<<std::endl;
		in.open(linux_path[cnt++] + path);
	}
	if (!in.is_open())
	{
		std::cerr << "can't open the file : " << path << std::endl;
		return res;
	}
	while (std::getline(in, s))
	{
		if (path.substr(0, 2).compare("/*") == 0)
			continue;
		s.erase(remove(s.begin(), s.end(), ' '), s.end());
		if (std::regex_match(s, include_regex))
		{
			s.erase(remove(s.begin(), s.end(), '\"'), s.end());
			s.erase(remove(s.begin(), s.end(), '\"'), s.end());
			s.erase(remove(s.begin(), s.end(), '<'), s.end());
			s.erase(remove(s.begin(), s.end(), '>'), s.end());
			auto index = s.find("/*");
			if (index != std::string::npos)
			{
				auto index_r = s.rfind("*/");
				s.erase(index - 1, index_r - index + 3);
			}
			s.erase(0, 8);
			res.push_back(s);
		}
		else if (s != "" && s[0] != '/' && s[0] != '*' && s[0] != '#')
			break;
	}
	in.close();
	return res;
}
void list_file(std::string path, std::vector<std::string> &res)
{
	std::regex valid_filename(".+\\.c");
	for (const auto &entry : directory_iterator(path))
	{
		if (std::filesystem::is_directory(entry.path()))
		{
			list_file(entry.path().string(), res);
			continue;
		}
		std::string s = entry.path().string();
		if (std::regex_match(s, valid_filename) && s != "project.cpp")
		{
			res.push_back(s);
		}
	}
}
std::shared_ptr<node> tree_create(std::string path,
								  std::shared_ptr<node> parent = nullptr)
{
	auto root = std::make_shared<node>();
	// if (count_runtimes >= 20000000)
	// 	return root;
	root->filename = path;
	auto vec = read_include(path);
	int limit = vec.size();
#pragma omp parallel for
	for (int i = 0; i < limit; i++)
	{
		if (std::find(c_header, c_header + 111, vec[i]) ==
			c_header + 111)
		{
			auto it = file_map.find(vec[i]);
			if (it != file_map.end())
			{
				root->child.push_back(it->second);
				continue;
			}
			std::shared_ptr<node> n = std::make_shared<node>();
			n->filename = vec[i];
			file_map[vec[i]] = n;
			root->child.push_back(tree_create(vec[i], n));
		}
	}
	return root;
}
void write_image(std::ofstream &out, std::shared_ptr<node> n)
{
	int limit = n->child.size();
#pragma omp parallel for
	for (int i = 0; i < limit; i++)
	{
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
	for (auto c : root->child)
	{
		child_file.emplace(c->filename);
		add_filename(child_file, c);
	}
}
std::vector<std::string> check_if_remove_non_recursive(std::string path_source,
													   std::string path_des)
{
	auto set_source = read_function_used(path_source);
	//print(set_source);
	//std::cout<<"-----------------"<<std::endl;
	auto set_des = read_function_declare(path_des);
	//print(set_des);
	return check_ret(set_source, set_des);
}
std::set<std::string> check_if_remove_recursive(std::string path_source,
												std::string path_des)
{
	std::map<std::string, std::set<std::string>> symbol_table;
	auto root = tree_create(path_des);
	std::set<std::string> included_file_des;
	add_filename(included_file_des, root);
	for (auto c : included_file_des)
	{
		symbol_table[c] = read_function_declare(c);
	}
	std::set<std::string> res;
	auto set_source = read_function_used(path_des);
	auto set_des = read_function_declare(path_source);
	if (!check(set_source, set_des))
	{
		res.emplace(path_des);
	}
	for (auto c : included_file_des)
	{
		if (!check(symbol_table[c], set_des))
		{
			res.emplace(c);
		}
	}
	return res;
}
std::map<std::pair<std::string, std::string>, std::vector<std::string>> check_if_remove_file(std::string path)
{
	std::map<std::pair<std::string, std::string>, std::vector<std::string>> ret;
	if (std::filesystem::is_directory(path))
	{
		std::vector<std::string> res;
		list_file(path, res);
		for (auto c : res)
		{
			auto index = c.rfind("/");
			if (index == std::string::npos)
			{
				linux_path[sizeof(linux_path) / sizeof(linux_path[0]) - 1] = "./";
			}
			else
			{
				linux_path[sizeof(linux_path) / sizeof(linux_path[0]) - 1] = c.substr(0, index + 1);
			}
			auto vec = read_include(c);
			if (!vec.empty())
			{
				for (auto k : vec)
				{
					ret[std::make_pair(c, k)] = check_if_remove_non_recursive(c, k);
				}
			}
		}
	}
	else
	{
		auto vec = read_include(path);
		if (!vec.empty())
		{
			for (auto c : vec)
			{
				ret[std::make_pair(path, c)] = check_if_remove_non_recursive(path, c);
			}
		}
	}
	return ret;
}
void generate_image(std::shared_ptr<node> root)
{
	std::ofstream outfile;
	outfile.open("./write.dot", std::ios::app);
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
}
int main(int argc, char *argv[])
{
	const rlim_t kStackSize =
		1024L * 1024L * 1024L; // min stack size = 1024 Mb
	struct rlimit rl;
	int result = getrlimit(RLIMIT_STACK, &rl);
	if (result == 0)
	{
		if (rl.rlim_cur < kStackSize)
		{
			rl.rlim_cur = kStackSize;
			result = setrlimit(RLIMIT_STACK, &rl);
			if (result != 0)
			{
				fprintf(stderr,
						"setrlimit returned result = %d\n",
						result);
			}
		}
	}
	linux_path[sizeof(linux_path) / sizeof(linux_path[0]) - 1] = "";
	int include = 0;
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-i"))
		{
			argc -= 2;
			linux_path[0] = std::string(argv[i + 1]) + std::string("include/");
			linux_path[1] = std::string(argv[i + 1]) + std::string("arch/") + std::string(STR(ARCH)) + std::string("/include/");
			linux_path[2] = std::string(argv[i + 1]) + std::string("include/uapi/");
			linux_path[3] = std::string(argv[i + 1]) + std::string("arch/") + std::string(STR(ARCH)) + std::string("/include/uapi/");
			include = 1;
			break;
		}
	}
	if (!include)
	{
		linux_path[0] = std::string("include/");
		linux_path[1] = std::string("arch/") + std::string(STR(ARCH)) + std::string("/include/");
		linux_path[2] = std::string("include/uapi/");
		linux_path[3] = std::string("arch/") + std::string(STR(ARCH)) + std::string("/include/uapi/");
		std::cout << "haven't input the include path, so we use the default path" << std::endl;
		for (int i = 0; i < sizeof(linux_path) / sizeof(linux_path[0]); i++)
		{
			std::cout << linux_path[i] << std::endl;
		}
	}
	if (argc == 1)
	{
		std::vector<std::string> vec;
		list_file("./", vec);
		for (auto c : vec)
		{
			generate_image(tree_create(std::string("./") + c));
		}
		exec("dot -Tsvg write.dot -o output.svg");
		exec("rm write.dot");
		return 0;
	}
	std::string path(argv[2 + 2 * include]);
	auto index = path.rfind("/");
	if (index == std::string::npos)
	{
		linux_path[sizeof(linux_path) / sizeof(linux_path[0]) - 2] = "./";
	}
	else
	{
		linux_path[sizeof(linux_path) / sizeof(linux_path[0]) - 2] = path.substr(0, index + 1);
	}
	if (argc == 2)
	{
		std::shared_ptr<node> root = tree_create(argv[1 + 2 * include]);
		generate_image(root);
		exec("dot -Tsvg write.dot -o output.svg");
		exec("rm write.dot");
	}
	else if (argc == 3 && strcmp(argv[1 + 2 * include], "-move") == 0)
	{
		auto table = check_if_remove_file(argv[2 + 2 * include]);
		for (auto c : table)
		{
			if (c.second.empty())
			{
				std::cout << c.first.second << " can be removed from" << c.first.first << std::endl;
			}
			else
			{
				std::cout << c.first.second << " can't be removed from" << c.first.first << std::endl;
				std::cout << "the following functions are needed" << std::endl;
				print(c.second);
			}
			std::cout << "----------------------------" << std::endl;
		}
	}
	else if (argc == 3)
	{
		std::shared_ptr<node> root = tree_create(argv[1 + 2 * include]);
		std::set<std::string> child_file;
		add_filename(child_file, root);
		if (std::find(child_file.begin(), child_file.end(), argv[2 + 2 * include]) !=
			child_file.end())
		{
			std::cout << "already include this file" << std::endl;
			return 0;
		}
		else
		{
			std::cout << "doesn't include this file" << std::endl;
		}
	}
	else if (argc == 4 && strcmp(argv[1 + 2 * include], "-move") == 0)
	{
		auto result = check_if_remove_non_recursive(argv[2 + 2 * include], argv[3 + 2 * include]);
		if (result.empty())
		{
			std::cout << argv[3 + 2 * include] << " can be removed from "
					  << argv[2 + 2 * include] << std::endl;
		}
		else
		{
			std::cout << argv[3 + 2 * include] << " can't be removed from "
					  << argv[2 + 2 * include] << std::endl;
			std::cout << "the follow functions are needed" << std::endl;
			print(result);
		}
	}
	else if (argc == 4 && strcmp(argv[1 + 2 * include], "-mover") == 0)
	{
		auto vec = check_if_remove_recursive(argv[2 + 2 * include], argv[3 + 2 * include]);
		if (vec.empty())
		{
			std::cout << argv[3 + 2 * include] << " can be removed from "
					  << argv[2 + 2 * include] << std::endl;
		}
		else
		{
			std::cout << argv[3 + 2 * include] << " can't be removed from "
					  << argv[2 + 2 * include] << std::endl;
			std::cout << "the following file are needed"
					  << std::endl;
			for (auto c : vec)
			{
				std::cout << c << std::endl;
			}
		}
	}
	else
	{
		std::cout << "the tool can only be used in the following ways"
				  << std::endl;
		std::cout
			<< "input nothing and it will produce the DAG of the directory"
			<< std::endl;
		std::cout
			<< "input one file parameter and it will produce the included tree of the specific file"
			<< std::endl;
		std::cout
			<< "input two file parameter and it will check whether the first file has included the second file"
			<< std::endl;
		std::cout
			<< "input -move and two path to the file and it will check whether the second file can be \nremoved from the first one"
			<< std::endl;
		std::cout
			<< "input -move ,-r and two path to the file and it will function recursively to check \nwhether the second file can be removed from the first one"
			<< std::endl;
		std::cout
			<< "input -i and the include path then the program will use the path you have specified, otherwise the program will use the default path" << std::endl;
	}

	return 0;
}