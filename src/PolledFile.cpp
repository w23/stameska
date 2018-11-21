#include "PolledFile.h"
#include "utils.h"

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

#include <string>

class PolledFile::Impl {
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
		const int result = stat(filename, &st);
		Metadata metadata;
		if (result == 0) {
			metadata.size = st.st_size;
			metadata.time = st.st_mtim;
		}
		return metadata;
#endif
	}

	static int readFileContents(const char *filename, std::vector<unsigned char>& out_buffer) {
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
		std::vector<unsigned char> buffer;
		buffer.resize(size);
		DWORD bytes_read = 0;
		if (!ReadFile(fh, buffer.data(), size, &bytes_read, NULL) || bytes_read != size) {
			MSG("Cannot read %d bytes from file '%s'", size, filename);
			bytes_read = 0;
		}

		CloseHandle(fh);
		out_buffer.swap(buffer);
		return bytes_read != 0;
#else
		const int fd = open(filename, O_RDONLY);
		if (fd < 0) {
			MSG("Cannot open file %s", filename);
			return 0;
		}

		struct stat st;
		stat(filename, &st);

		std::vector<unsigned char> buffer;
		buffer.resize(st.st_size);
		if (read(fd, buffer.data(), st.st_size) != st.st_size) {
			MSG("Cannot read %d bytes from file '%s'", (int)st.st_size, filename);
			st.st_size = 0;
		}

		close(fd);
		out_buffer.swap(buffer);
		return st.st_size != 0;
#endif
	}

public:
	Impl(string_view filename) : filename_(filename) {}

	bool poll() {
		const Metadata new_metadata = readFileMetadata(filename_.c_str());
		if (!new_metadata.valid())
			return false;

		if (new_metadata == metadata_)
			return false;

		if (!readFileContents(filename_.c_str(), buffer_))
			return false;

		metadata_ = new_metadata;
		return true;
	}

	const std::vector<unsigned char>& buffer() const { return buffer_; }

private:
	const std::string filename_;
	std::vector<unsigned char> buffer_;
	Metadata metadata_;
};

PolledFile::PolledFile(string_view filename) : impl_(new Impl(filename)) {}
PolledFile::~PolledFile() {}

bool PolledFile::poll(unsigned int poll_seq) {
	if (beginUpdate(poll_seq) && impl_->poll())
		return endUpdate();

	return false;
}

const std::vector<unsigned char>& PolledFile::data() const {
	return impl_->buffer();
}
