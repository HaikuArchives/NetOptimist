#include "Pref.h"
#include <stdlib.h>
#include <string.h> 
#include <stdio.h> 

#include <be/app/Application.h>
#ifdef __BEOS__
#include <app/Roster.h>
#include <storage/Path.h> 
#include <storage/Entry.h> 
#include <storage/FindDirectory.h>
#endif

Pref::Pref() : 
	m_homePage(NULL), 
	m_searchPage(NULL),
	m_downloadDirectory(NULL),
	m_newWindowAction(NO_CLONE_CURRENT),
	m_cookieAction(NO_ACCEPT_COOKIE),
	m_launchDownloaded(true),
	m_daysInGo(1),
	m_showImages(true),
	m_showBgImages(true),
	m_showAnimations(true),
	m_underlineLinks(true),
	m_haikuErrors(true),
	m_useFonts(true),
	m_useBgColors(true),
	m_useFgColors(true),
	m_playSounds(true),
	m_flickerFree(true),
	m_useJavaScript(true)
	
	 {
#ifdef __BEOS__
	appDir = NULL;
#endif

	// Connections
	m_proxyName = NULL;
	m_online = true;
}

#ifdef __BEOS__
void Pref::Init() {
	app_info info; 
	BPath path; 
	be_app->GetAppInfo(&info); 
	BEntry entry(&info.ref); 
	entry.GetPath(&path); 
	path.GetParent(&path); 
	appDir = strdup(path.Path());
	printf("--- current dir %s\n", appDir);

	// FIXME: read em from config file
	m_homePage = strdup("http://www.be.com");
	m_searchPage = strdup("http://www.google.com");
	m_downloadDirectory = strdup("/boot/home/");
}

Pref::~Pref() {
	if (appDir) free(appDir);
}

const char* Pref::AppDir() {
	return appDir;
}

int Pref::ProxyPort() {
	return 0;
}

const char* Pref::CacheDir() {
	static char cacheDir[1024];
	// FIXME: shouldn't calculate it every time (!!!)
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append("NetOptimist/NetCache/");
	strcpy(cacheDir, path.Path());
	strcat(cacheDir, "/");
	return cacheDir;
}

#else

int Pref::ProxyPort() {
	return 3128;
}

const char* Pref::CacheDir() {
	static char cacheDir[1024];
/*
	struct stat st;
	sprintf(cacheDir, "/local/%s/NetOptimist/NetCache/", getenv("USER"));
	if (stat(cacheDir, &st)==0 && (st.st_mode & S_IFDIR == S_IFDIR)) {
		return cacheDir;
	}
*/
	sprintf(cacheDir, "%s/%s", getenv("HOME"), ".w3m/");
	return cacheDir;
}

#endif

void Pref::SetProxyName(const char *proxy) {
	m_proxyName = proxy;
}

const char* Pref::ProxyName() {
	if (m_proxyName)
		return m_proxyName;
	return NULL;
}

bool Pref::Online() const {
	return m_online;
}

void Pref::SetOnline(bool set) {
	m_online = set;
}


// General
void Pref::SetHomePage(const char *s) {
	if (m_homePage) free(m_homePage);
	m_homePage = strdup(s);
}
const char * Pref::HomePage() { return m_homePage; }

void Pref::SetSearchPage(const char *s) {
	if (m_searchPage) free(m_searchPage);
	m_searchPage = strdup(s);
}
const char * Pref::SearchPage() { return m_searchPage; }

void Pref::SetDownloadDirectory(const char *s) {
	if (m_downloadDirectory) free(m_downloadDirectory);
	m_downloadDirectory = strdup(s);
}
const char * Pref::DownloadDirectory() { return m_downloadDirectory; }

void Pref::SetNewWindowAction(new_window_action action) { m_newWindowAction = action; }
const new_window_action Pref::NewWindowAction() { return m_newWindowAction; }

void Pref::SetCookieAction(cookie_action action) { m_cookieAction = action; }
const cookie_action Pref::CookieAction() { return m_cookieAction; }

void Pref::SetLaunchDownloaded(bool launch) { m_launchDownloaded = launch; }
const bool Pref::LaunchDownloaded() { return m_launchDownloaded; }

void Pref::SetDaysInGo(int days) { m_daysInGo = days; }
const int Pref::DaysInGo() { return m_daysInGo; }

// Display page
bool Pref::ShowImages() { return m_showImages; }
void Pref::SetShowImages(bool b) { m_showImages = b; }

bool Pref::ShowBgImages() { return m_showBgImages; }
void Pref::SetShowBgImages(bool b) { m_showBgImages = b; }

bool Pref::ShowAnimations() { return m_showAnimations; }
void Pref::SetShowAnimations(bool b) { m_showAnimations = b; }

bool Pref::UnderlineLinks() { return m_underlineLinks; }
void Pref::SetUnderlineLinks(bool b) { m_underlineLinks = b; }

bool Pref::HaikuErrors() { return m_haikuErrors; }
void Pref::SetHaikuErrors(bool b) { m_haikuErrors = b; }

bool Pref::UseFonts() { return m_useFonts; }
void Pref::SetUseFonts(bool b) { m_useFonts = b; }

bool Pref::UseBgColors() { return m_useBgColors; }
void Pref::SetUseBgColors(bool b) { m_useBgColors = b; }

bool Pref::UseFgColors() { return m_useFgColors; }
void Pref::SetUseFgColors(bool b) { m_useFgColors = b; }

bool Pref::UsePlaySounds() { return m_playSounds; }
void Pref::SetUsePlaySounds(bool b) { m_playSounds = b; }

bool Pref::FlickerFree() { return m_flickerFree; }
void Pref::SetFlickerFree(bool b) { m_flickerFree = b; }

bool Pref::UseJavaScript() { return m_useJavaScript; }
void Pref::SetUseJavaScript(bool b) { m_useJavaScript = b; }

void Pref::Save() {
// FIXME:
}

Pref Pref::Default;
