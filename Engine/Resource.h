#ifndef RESSOURCE_H
#define RESSOURCE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <DataIO.h>
#include "platform.h"
#include "Pref.h"
#include "StrPlus.h"

class ResourceInfo {
	char * m_mimetype;
protected:
	size_t m_size;
public:
	time_t m_date;
	time_t m_expires;
	time_t m_modified;
	char *m_location;
	bool m_cacheOnDisk;
	ResourceInfo() {
		m_mimetype = NULL;
		m_date = time(0);
		m_expires = m_modified = 0;
		m_size = 0;
		m_location = NULL;
		m_cacheOnDisk = true;
	}
	void SetMimeType(const char *mimetype) {
		if (m_mimetype)
			free(m_mimetype);
		m_mimetype = StrDup(mimetype);
	}
	size_t Size() const { return m_size; }
	void SetSize(size_t size) { m_size = size; }
	const char *MimeType() {
		return m_mimetype ? m_mimetype : "notype";
	}
	virtual ~ResourceInfo() {
		if (m_mimetype)
			StrFree(m_mimetype);
		if (m_location)
			free(m_location);
	}
};

class Resource : public ResourceInfo {
public:
	virtual bool Open() =0;
	virtual void Close() =0;
	virtual int nextchar() =0;
	virtual size_t ReadChar(char *buff, size_t s) =0;
	virtual const char *CachedFile() const =0;
	virtual BPositionIO* Data() const =0;
	virtual bool SaveToFile(const char *filename);
};


class FileResource : public Resource {
protected:
	bool closed;
	FILE *fd;
	char m_name[FILENAME_MAX];
public:
	FileResource(const char *name);
	virtual ~FileResource() {
		Close();
	}
	bool Open();
	void Close();
	int nextchar();
	virtual size_t ReadChar(char *buff, size_t s);
	virtual BPositionIO* Data() const { return NULL; }
	virtual const char * CachedFile() const {
		if (m_name[0] == '\0') {
			return NULL;
		} else {
			return m_name;
		}
	}
};

class CacheResource : public FileResource {
	char *m_absFileName;
public:
	const char * CachedFile() const {
		if (m_name[0] == '\0') {
			return NULL;
		} else {
			return m_absFileName;
		}
	}
	CacheResource(const char *name, size_t size) : FileResource(name) {
		m_size = size;
		if (name && name[0]=='/') {
			m_absFileName = new char[strlen(m_name) + 1];
			strcpy(m_absFileName, m_name);
		} else {
			m_absFileName = new char[strlen(Pref::Default.CacheDir()) + strlen(m_name) + 1];
			strcpy(m_absFileName, Pref::Default.CacheDir());
			strcat(m_absFileName, m_name);
		}
	}
	~CacheResource() { if (m_absFileName) delete[] m_absFileName; }
};

class MemoryResource : public Resource {
	BPositionIO *m_data;
public:
	MemoryResource(BPositionIO *data = NULL) : Resource() {
		SetData(data);
	}
	void SetData(BPositionIO *data) {
		m_data = data;
	}
	virtual ~MemoryResource() {
		if (m_data)
			delete m_data;
	}
	bool Open() {
		if (m_data) {
			m_data->Seek(SEEK_SET, 0);
		} else {
			fprintf(stderr, "Warning : no data when openning MemoryResource\n");
		}
		return m_data != NULL;
	}
	void Close() { }
	int nextchar() { 
		char c;
		if (m_data->Read(&c, 1) != 1)
			return -1;
		else
			return c;
	}
	virtual size_t ReadChar(char *buff, size_t s) {
		return m_data->Read(buff, s);
	}
	virtual const char * CachedFile() const {
		return NULL;
	}
	virtual BPositionIO* Data() const { return m_data; }
};

#endif
