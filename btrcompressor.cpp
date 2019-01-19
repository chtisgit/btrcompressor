/*
	MIT License

	Copyright (c) 2019 Christian Fiedler

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/


#include <string>
#include <set>
#include <deque>

#include <cstdio>
#include <cstdarg>
#include <cctype>

#include <getopt.h>
#include <sys/xattr.h>

#include "uboostfs/filesystem.h"
namespace fs = boostfs;

static void error(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	printf("error: ");
	vprintf(fmt, va);
	va_end(va);
	printf("\n\n");
}

#ifndef ENOATTR
#define ENOATTR ENODATA
#endif

const std::string XATTR_BTRFS_PREFIX = "btrfs.";
constexpr int XATTR_BTRFS_PREFIX_LEN = (sizeof(XATTR_BTRFS_PREFIX) - 1);


static bool compress(const fs::Path* const path, const std::string& xattr_name, const std::string& compression)
{
	auto sret = setxattr(path->c_str(), xattr_name.c_str(), compression.c_str(), compression.size(), 0);
	if (sret < 0) {
		return false;
	}

	return true;
}

struct Config {
	int threads = 1;
	bool verbose = false;
};


static void compress_recursively(const char *path, const std::string& xattr_name, const std::string& compression, const Config& cfg)
{
	std::deque<fs::directory_iterator> stack;
	stack.emplace_back(fs::Path(path));

	const char *enabledisable = (compression.size() == 0) ? "disable" : "enable";
	while(!stack.empty()){
		auto& dir = stack.back();
		if(dir == fs::directory_iterator()){
			stack.pop_back();
			continue;
		}
		auto p = (*dir).path();
		if((p.filename().string() == "." && stack.size() != 1) || p.filename().string() == ".."){
			++dir;
			continue;
		}

		if(cfg.verbose)
			printf("%s compression on %s\n", enabledisable, p.c_str());

		compress(&p, xattr_name, compression);
		++dir;
		if(fs::is_directory(p) && p.filename().string() != "."){
			stack.emplace_back(p);
		}
	}

}

static void usage(const char *progname)
{
	fprintf(stderr,	"Usage: %s [-v] [-j <threads>] <-c compression | -d> \n\n"
			"       -c <compression> ... choose the compression to set (zlib,lzo,zstd)\n"
			"       -d               ... disable compression\n"
			"       -j <threads>     ... number of threads to use (not implemented)\n"
			"       -v               ... verbose output\n"
			"\n", progname);
}

// accepts empty string, "zlib", "zstd", "lzo" and returns true
// on other input this method returns false
static bool valid_algorithm(const std::string& alg)
{
	static const std::set<std::string> available = { "", "zlib", "zstd", "lzo" };

	if(available.count(alg) == 1)
		return true;
	return false;
}

int main(int argc, char **argv)
{
	int opt;
	std::string compression("invalid");
	Config cfg;

	while ((opt = getopt(argc, argv, "c:dj:v")) != -1) {
		switch (opt) {
		case 'c':
			compression = optarg;
			break;
		case 'd':
			compression.clear();
			break;
		case 'j':
			cfg.threads = atoi(optarg);
			printf("warning: threading is not implemented yet. %s will run with 1 thread.\n", argv[0]);
			break;
		case 'v':
			cfg.verbose = true;
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}
	if(optind >= argc){
		error("no file or directory given", 0);
		usage(argv[0]);
		return 1;
	}
	if(cfg.threads <= 0){
		error("invalid number of threads (%d)", cfg.threads);
		usage(argv[0]);
		return 1;
	}
	if(compression == "no" || compression == "none"){
		// be compatible with btrfs-progs, though I don't like this
		compression.clear();
	}
	if(!valid_algorithm(compression)){
		error("invalid compression chosen (%s)", compression.c_str());
		usage(argv[0]);
		return 1;
	}

	const std::string xattr_name = XATTR_BTRFS_PREFIX + "compression";
	compress_recursively(argv[optind], xattr_name, compression, cfg);

	return 0;
}
