#include "fsoperator.h"

#include <algorithm>
namespace ftpServer {
using namespace boost::filesystem;
bool start_with(const string& s, const string& head) {
    if (s.size() < head.size()) return false;
    for (size_t i = 0; i < head.size(); ++i) {
        if (s[i] != head[i]) return false;
    }
    return true;
}
bool find_file(const path& dir_path, const path& file_name, path& path_found) {
    const directory_iterator end;
    const auto it = std::find_if(directory_iterator(dir_path), end,
                                 [&file_name](const directory_entry& e) {
                                     return e.path().filename() == file_name;
                                 });
    if (it == end) {
        return false;
    } else {
        path_found = it->path();
        return true;
    }
}

fileOperator::fileOperator() : _path(boost::filesystem::current_path()) {}
path fileOperator::real_path(path& cur, const string& str) {
    int len = str.size();
    if (start_with(str, "..")) {
        int cnt = 0;
        while (start_with(str, "..")) cnt++;
        auto j = cur.end();
        for (int i = 0; i < cnt; ++i) {
            --j;
        }
        path tmp();
    } else if (start_with(str, ".")) {
        cout << str.substr(2, len - 2) << endl;
        return real_path(cur, str.substr(2, len - 2));
    }
    return path(cur /= str);
}

void fileOperator::ls() {
    if (is_directory(_path)) {
        for (directory_entry& x : directory_iterator(_path)) {
            if (is_directory(x))
                cout << "    " << x.path().filename().string() << "   dir"
                     << endl;
            else
                cout << "    " << x.path().filename().string() << endl;
        }
    }
}

bool fileOperator::cd(const string& _path_) {
    Path tmp(_path);
    if (start_with(_path_, "..")) {
        if (tmp.has_parent_path()) tmp = tmp.parent_path();
        // cout<<tmp.string()<<endl;
        _path = tmp;
    } else if (start_with(_path_, ".")) {
        return true;
    } else {
        auto pos = _path_.find('/');
        if (pos != string::npos) {
            auto sub_path = _path_.substr(0, pos);
            path tmp_path;
            if (find_file(_path, sub_path, tmp_path)) {
                tmp = tmp_path;
            } else {
                cout << sub_path << ": No such file or directory" << endl;
                return false;
            }
            if (!is_directory(tmp_path)) {
                cout << tmp_path.filename().string() << " is not a dir" << endl;
            }
            _path = tmp_path;
        }
    }
}
}  // namespace ftpServer