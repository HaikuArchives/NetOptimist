#include <errno.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#include "Resource.h"


FileResource::FileResource(const char *name) : Resource() {
	/* We are given a (sub part of ) url and we must get the corresponding
	 * file name on disk :
	 *  - replace %20 by space
	 *  - ignore after '#' (delimiter for target in document)
	 */
	char *out = m_name;
	// XXX this should be put somewhere else
	for (const char *ptr=name; *ptr && *ptr!='#'; ) {
		if (*ptr=='%') {
			fprintf(stderr, "decode name\n");
			// we have to convert %20 in a space, etc...
			// XXX error : possible bug if buffer overflow
			// XXX we need encode_url() and decode_url() functions
			if (isxdigit(*(ptr+1)) && isxdigit(*(ptr+2))) {
				int ch;
				sscanf(ptr+1, "%2x", &ch);
				*out++ = (char)ch;
				ptr+=3;
			}
		} else {
			*(out++) = *(ptr++);
		}
	}
	*out = '\0';
	closed = true;
	fd = NULL;

	struct stat buf;
	if (stat (name, &buf) >= 0 && (buf.st_mode & S_IFREG) == S_IFREG) {
		m_size = buf.st_size;
	} else {
		m_size = 0;
	}
}

bool FileResource::Open() {
	closed = false;
	if (CachedFile()!=NULL) {
		fd = fopen(CachedFile(),"r");
		if (!fd)
			fprintf(stdout, "could not open file %s\n", CachedFile());
	}
	return fd!=NULL;
}

void FileResource::Close() {
	if (!closed) {
		if (fd) fclose(fd);
		closed = true;
	}
}
int FileResource::nextchar() {
	return fgetc(fd);
}

size_t FileResource::ReadChar(char *buff, size_t t) {
	return fread(buff, 1, t, fd);
}

bool Resource::SaveToFile(const char *filename) {
	char buffer[1024];
	int nb;
	BPositionIO* data;
	int fd;

	data = Data();
	if (!data) {
		fprintf(stderr, "Resource::SaveToFile : no data\n");
		return false;
	}

	fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if (fd<0) {
		fprintf(stderr, "Could not save ressource into file %s, %s(%d)\n",
			filename, strerror(errno), errno);
		return false;
	}

	data->Seek(SEEK_SET,0);

	while( (nb=data->Read(buffer, sizeof(buffer))) > 0) {
		write(fd, buffer, nb);
	}

	close(fd);
	return true;
}
