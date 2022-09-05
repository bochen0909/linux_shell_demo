#include <dirent.h>
#include <grp.h>
#include <math.h>
#include <pwd.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

using namespace std;

/**
 * struct of information for a file.
 */
struct Entry {
	string name;
	string path;
	char type;
	string perm;
	int nlink;
	string user;
	string group;
	long byte_size;
	long blocks;
	struct timespec modify_time;
	string display;
};

/**
 * return file type such as folder, regular file, link, etc
 */
char get_file_type(struct stat& sb);

/**
 * return the file permissions for user, group and others
 */
string get_perm_bits(struct stat& sb);

/**
 * format the timespec according to the ouput format
 */
string format_time(struct timespec& ts);

/**
 * get all the file info via entry argument given the file name and path
 */
int get_file_info(string name, string path, Entry& entry);

/**
 * output one line for a file
 */
void print_entry(struct Entry& entry);
/**
 * output a file list, usually for all files in a folder
 */
void print_entry(std::vector<struct Entry>& entrylist);

/**
 * list the files in the given path.
 */
int list_path(string& filepath);

/**
 * entry
 */
int main(int argc, char** argv) {
	string filepath = ".";

	if (argc <= 2) {
		filepath = ".";
		if (argc == 2) {
			filepath = argv[1];
		}
		return list_path(filepath);
	} else {
		int exit_code = 0;
		for (int i = 1; i < argc; i++) {
			filepath = argv[i];
			printf("%s:\n", filepath.c_str());
			exit_code = list_path(filepath) == 0 ? 0 : -1;
			if (i != argc - 1)
				printf("\n");
		}
		return exit_code;
	}
	return 0;
}

int list_path(string& filepath) {
	Entry entry;
	if (get_file_info(filepath, filepath, entry) != 0) {
		return -1;
	}

	if (entry.type != 'd') {
		print_entry(entry);
	} else {
		std::vector<struct Entry> filelist;
		DIR *d;
		struct dirent *dir;
		d = opendir(filepath.c_str());
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				struct Entry entry;
				if (get_file_info(dir->d_name, filepath + "/" + dir->d_name,
						entry) == 0) {
					filelist.push_back(entry);
				}
			}
			closedir(d);
		} else {
			fprintf(stderr,
					"myls: cannot access %s: No such file or directory\n",
					filepath.c_str());
			return -1;
		}
		print_entry(filelist);
	}
	return 0;
}

char get_file_type(struct stat& sb) {
	if (S_ISDIR(sb.st_mode)) {
		return 'd';
	} else if (S_ISCHR(sb.st_mode)) {
		return 'c';
	} else if (S_ISBLK(sb.st_mode)) {
		return 'b';
	} else if (S_ISREG(sb.st_mode)) {
		return '-';
	} else if (S_ISFIFO(sb.st_mode)) {
		return 'p';
	} else if (S_ISLNK(sb.st_mode)) {
		return 'l';
	} else if (S_ISSOCK(sb.st_mode)) {
		return 's';
	} else {
		return '?';
	}
}

string get_perm_bits(struct stat& sb) {
	string bits;

	bits += S_IRUSR & (sb.st_mode) ? 'r' : '-';
	bits += S_IWUSR & (sb.st_mode) ? 'w' : '-';
	bits += S_IXUSR & (sb.st_mode) ? 'x' : '-';

	bits += S_IRGRP & (sb.st_mode) ? 'r' : '-';
	bits += S_IWGRP & (sb.st_mode) ? 'w' : '-';
	bits += S_IXGRP & (sb.st_mode) ? 'x' : '-';

	bits += S_IROTH & (sb.st_mode) ? 'r' : '-';
	bits += S_IWOTH & (sb.st_mode) ? 'w' : '-';
	bits += S_IXOTH & (sb.st_mode) ? 'x' : '-';

	return bits;
}

string format_time(struct timespec& ts) {
	struct tm t;
	char buf[128];
	tzset();
	if (localtime_r(&(ts.tv_sec), &t) != NULL) {
		if (strftime(buf, 128, "%b %d %H:%M", &t))
			return buf;
	}
	return "";
}

int get_file_info(string name, string path, Entry& entry) {
	entry.name = name;
	entry.path = path;
	struct stat sb;
	if (lstat(path.c_str(), &sb) != 0) {
		fprintf(stderr, "myls: cannot access %s: No such file or directory\n",
				path.c_str());
		return -1;
	}
	entry.type = get_file_type(sb);
	entry.perm = get_perm_bits(sb);
	entry.nlink = sb.st_nlink;

	struct passwd *pw = getpwuid(sb.st_uid);
	if (pw) {
		entry.user = pw->pw_name;
	}

	struct group *grp = getgrgid(sb.st_gid);
	if (grp) {
		entry.group = grp->gr_name;
	}

	entry.byte_size = sb.st_size;
	entry.blocks = sb.st_blocks;
	entry.modify_time = sb.st_mtim;
	entry.display = entry.name;
	if (entry.type == 'l') {
		char buf[PATH_MAX];
		size_t len;
		if ((len = readlink(entry.path.c_str(), buf, sizeof(buf) - 1))
				!= (size_t) -1) {
			buf[len] = '\0';
			entry.display = entry.name + " -> " + buf;
		}
	}
	return 0;
}

void print_entry(std::vector<struct Entry>& entrylist) {
	long totalbytes = 0;

	int max_nlink = 1;
	size_t max_usrwidth = 1;
	size_t max_grpwidth = 1;
	long max_bytes = 1;
	for (size_t i = 0; i < entrylist.size(); i++) {
		struct Entry &entry = entrylist[i];
		totalbytes += entry.byte_size;
		max_nlink = max_nlink > entry.nlink ? max_nlink : entry.nlink;
		max_bytes = max_bytes > entry.byte_size ? max_bytes : entry.byte_size;
		max_usrwidth =
				max_usrwidth > entry.user.size() ?
						max_usrwidth : entry.user.size();
		max_grpwidth =
				max_grpwidth > entry.group.size() ?
						max_grpwidth : entry.group.size();
	}
	int max_nlink_width = round(log(max_nlink) / log(10) + 1);
	int max_bytes_width = round(log(max_bytes) / log(10) + 1);

	printf("total %ld bytes\n", totalbytes);
	for (size_t i = 0; i < entrylist.size(); i++) {
		struct Entry &entry = entrylist[i];
		fprintf(stdout, "%c%s %*d %-*s %-*s %*ld %s %s\n", entry.type,
				entry.perm.c_str(), max_nlink_width, entry.nlink,
				(int) max_usrwidth, entry.user.c_str(), (int) max_grpwidth,
				entry.group.c_str(), max_bytes_width, entry.byte_size,
				format_time(entry.modify_time).c_str(), entry.display.c_str());
	}
}

void print_entry(struct Entry& entry) {
	fprintf(stdout, "%c%s %d %s %s %ld %s %s\n", entry.type, entry.perm.c_str(),
			entry.nlink, entry.user.c_str(), entry.group.c_str(),
			entry.byte_size, format_time(entry.modify_time).c_str(),
			entry.display.c_str());
}
