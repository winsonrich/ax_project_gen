#include "Glob.h"

namespace ax_pjgen {


#if ax_OS_Windows //------------------------------------------------------------------

class Dir : public NonCopyable {
public:
	~Dir() { close(); }

	bool open(const StrView& path) {
		close();
		_path.setUtf(path);
		_path.appendUtf("/*");		
		_done = false;
		return true;
	}

	void close() {
		if (isValid()) {
			FindClose(_h);
			_h = INVALID_HANDLE_VALUE;
			_done = true;
		}
	}

	bool getEntry(String& name) {
		if (_done) return false;

		WIN32_FIND_DATA data;
		for (;;) {
			if (!isValid()) {
				_h = FindFirstFileExW(_path.c_str(), FindExInfoStandard, &data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
				if (!isValid()) {
					Log::error("cannot open directory ", _path);
					return false;
				}
			}else{
				if (!FindNextFile(_h, &data)) {
					close();
					return false;
				}
			}

			const auto* f = data.cFileName;
			if (f[0] == '.' && f[1] == 0) continue;
			if (f[0] == '.' && f[1] == '.' && f[2] == 0) continue;
			name.setUtf(f, (int)wcslen(f));
			return true;
		}

	}

	bool isValid() const { return _h != INVALID_HANDLE_VALUE; }

	explicit operator bool() const { return isValid(); }

private:
	HANDLE _h {INVALID_HANDLE_VALUE};
	WString _path;
	bool _done {false};
};

#else //--------------------------------------------------------------

class Dir : public NonCopyable {
public:
	~Dir() { close(); }

	bool open(const StrView& path) {
		close();
		String pathA;
		pathA = path;

		_h = ::opendir(pathA.c_str());
		if (!_h) {
			Log::error("cannot open directory ", path);
			return false;			
		}

		return true;
	}

	void close() {
		if (_h) {
			::closedir(_h);
			_h = nullptr;
		}
	}

	bool getEntry(String& name) {
		if (!_h) return false;

		::dirent _entry;
		::dirent* p = nullptr;

		for (;;) {
			if (0 != ::readdir_r(_h, &_entry, &p)) {
				throw Error("error get directory entry");
			}
			if (!p) return false;

			const auto* f = p->d_name;
			if (f[0] == '.' && f[1] == 0) continue;
			if (f[0] == '.' && f[1] == '.' && f[2] == 0) continue;
			name = StrView_c_str(f);
			return true;
		}
	}

	bool isValid() const { return _h != nullptr; }
	explicit operator bool() const { return isValid(); }	

	::DIR*	_h {nullptr};
};

#endif

class GlobHelper : public NonCopyable {
public:
	void search(Vector<String>& out_paths, const StrView& path, bool needDir, bool needFile, bool needHidden) {
		_needDir    = needDir;
		_needFile   = needFile;
		_needHidden = needHidden;

		_out_paths = &out_paths;
		String absPath;
		Path::getAbs(absPath, path);

		auto p = absPath.view();
		auto start = p.splitByCharInList("*?").first
					  .splitEndByChar('/').first;

		_curPath = start;
		auto remain = p.slice(start.size() + 1);
		_step(remain);
	}

private:
	void _step(const StrView& path) {
		auto s = path.splitByChar('/');
		_step2(s.first, s.second);
	}

	void _step2(const StrView& name, const StrView& remain) {
		if (name.indexOfAnyCharIn("*?") < 0) {
			auto oldSize = _curPath.size();
			if (name) {
				_curPath += '/';
				_curPath += name;
			}

			if (!remain) {
				if ((_needFile && Path::fileExists(_curPath))
				  ||(_needDir  && Path::dirExists(_curPath)))
				{
					_out_paths->append(_curPath);
				}
			}else{
				_step(remain);
			}
			_curPath.resize(oldSize);
			return;
		}
		
		if (name == "**") {
			_step(remain);
		}
		
		if (!Path::dirExists(_curPath)) return;

		Dir dir;
		if (!dir.open(_curPath)) return;

		String entry;
		while (dir.getEntry(entry)) {
			if (!_needHidden && entry.view().startsWith(".")) {
				continue;
			}

			if (name == "**") {
				auto oldSize = _curPath.size();
				if (name) {
					_curPath += '/';
					_curPath += entry;
				}
				_step2(name, remain);
				_curPath.resize(oldSize);

				continue;
			}

			if (matchWildcard(entry, name)) {
				_step2(entry, remain);
			}
		}
	}

	bool matchWildcard(const StrView& sv, const StrView& wildcard) const {
		const auto* p   = sv.data();
		const auto* end = sv.end();

		const auto* w    = wildcard.begin();
		const auto* wEnd = wildcard.end();

		while (p < end && w < wEnd) {
			if (*w == *p || *w == '?'){
				p++;
				w++;
			}else if(*w == '*') {
				auto w1 = w + 1;
				if (w1 >= wEnd) break; // * is the last

				auto p1 = p + 1;
				if (p1 >= end) return false;

				p = p1;
				if (*p1 == *w1) {
					w = w1; //next wildcard
				}
			}else{
				return false;
			}
		}
		return true;
	}

	Vector<String>* _out_paths;
	String _curPath;
	bool _needDir;
	bool _needFile;
	bool _needHidden;
};

void Glob::search(Vector<String>& out_paths, const StrView& path, bool needDir, bool needFile, bool needHidden) {
	out_paths.clear();
	int index = path.indexOfAnyCharIn("*?");
	if (index < 0) {
		out_paths.append(path);
		return;
	}

	GlobHelper g;
	g.search(out_paths, path, needDir, needFile, needHidden);
}

} //namespace
