#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOMINMAX
#define NOMSG
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class PolledFile {
	const char *const filename_;
	unsigned int seq_ = 0;
	char buffer_[65536];

#ifdef _WIN32
	struct Metadata {
		size_t size = 0;
		FILETIME time = { 0, 0 };

		bool valid() const {
			return size != 0 && time.dwHighDateTime != 0 && time.dwLowDateTime != 0;
		}

		bool operator==(const Metadata& rhs) const {
			return size == rhs.size
				&& time.dwHighDateTime == rhs.time.dwHighDateTime
				&& time.dwLowDateTime == rhs.time.dwLowDateTime;
		}
	};
#else
	struct Metadata {
		size_t size = 0;
		struct timespec time = { 0, 0 };

		bool valid() const {
			return size != 0 && time.tv_sec != 0 && time.tv_nsec != 0;
		}

		bool operator==(const Metadata& rhs) const {
			return size == rhs.size
				&& time.tv_sec == rhs.time.tv_sec
				&& time.tv_nsec == rhs.time.tv_nsec;
		}
	};
#endif

	Metadata metadata_;

	static Metadata readFileMetadata(const char *filename) {
#ifdef _WIN32
		Metadata metadata;
		const HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fh == INVALID_HANDLE_VALUE) {
			MSG("Cannot check file '%s' modification time: %x", filename, GetLastError());
			return metadata;
		}

		LARGE_INTEGER splurge_integer;
		if (!GetFileSizeEx(fh, &splurge_integer) || splurge_integer.QuadPart > 1024 * 1024) {
			MSG("Cannot get file '%s' size or size is too large", filename);
			goto exit;
		}

		FILETIME ft;
		if (!GetFileTime(fh, NULL, NULL, &ft)) {
			MSG("Cannot get file '%s' modification time", filename);
			goto exit;
		}

		metadata.size = splurge_integer.QuadPart;
		metadata.time = ft;

	exit:
		CloseHandle(fh);
		return metadata;
#else
	struct stat st;
	stat(filename, &st);
	Metadata metadata;
	metadata.size = st.st_size;
	metadata.time = st.st_mtim;
	return metadata;
#endif
	}

	static int readFileContents(const char *filename, char *buffer, size_t max_size) {
#ifdef _WIN32
		const HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fh == INVALID_HANDLE_VALUE) {
			MSG("Cannot open file '%s'", filename);
			return 0;
		}

		LARGE_INTEGER splurge_integer;
		if (!GetFileSizeEx(fh, &splurge_integer) || splurge_integer.QuadPart > 1024 * 1024) {
			MSG("Cannot get file '%s' size or size is too large", filename);
			return 0;
		}

		const size_t size = splurge_integer.QuadPart;
		DWORD bytes_read = 0;
		if (size >= max_size || !ReadFile(fh, buffer, size, &bytes_read, NULL) || bytes_read != size) {
			MSG("Cannot read %d bytes from file '%s'", size, filename);
			bytes_read = 0;
		}
		buffer[bytes_read] = '\0';

		CloseHandle(fh);
		return bytes_read != 0;
#else
	const int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		MSG("Cannot open file %s", filename);
		return 0;
	}

	struct stat st;
	stat(filename, &st);

	if ((int)st.st_size >= (int)max_size || read(fd, buffer, st.st_size) != st.st_size) {
		MSG("Cannot read %d bytes from file '%s'", (int)st.st_size, filename);
		st.st_size = 0;
	}
	buffer[st.st_size] = '\0';

	close(fd);
	return st.st_size != 0;

#endif
	}

public:
	PolledFile(const char *filename) : filename_(filename) {}

	unsigned int seq() const { return seq_; }
	const char* data() const { return buffer_; }
	size_t size() const { return metadata_.size; }

	bool poll() {
		const Metadata new_metadata = readFileMetadata(filename_);
		if (!new_metadata.valid())
			return false;

		if (new_metadata == metadata_)
			return false;

		if (!readFileContents(filename_, buffer_, sizeof(buffer_)))
			return false;

		metadata_ = new_metadata;
		++seq_;
		return true;
	}
};

class PollAdaptor {
	const PolledFile &file_;
	unsigned int seq_;

public:
	PollAdaptor(const PolledFile &file) : file_(file), seq_(file_.seq() - 1) {}

	const PolledFile &file() const { return file_; }

	bool consume() {
		if (file_.seq() != seq_) {
			seq_ = file_.seq();
			return true;
		}

		return false;
	}
};

template <int N>
class PolledProgram : public Program {
	PollAdaptor* const adaptors_;
	const char *sources[N];

public:
	PolledProgram(PollAdaptor *adaptors)
		: adaptors_(adaptors)
	{
		for (int i = 0; i < N; ++i)
			sources[i] = adaptors_[i].file().data();
	}

	void poll() {
		bool updated = false;
		for (int i = 0; i < N; ++i)
			updated |= adaptors_[i].consume();

		if (updated)
			load(ConstArrayView<const char*>(sources, N));
	}
};
