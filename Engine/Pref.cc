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

// NOTE: when the new fields added both first constructor and = operator should be updated
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
	m_useJavaScript(true),
	m_enableProxies(false),
	m_httpProxyName(NULL),
	m_httpProxyPort(-1),
	m_ftpProxyName(NULL),
	m_ftpProxyPort(-1),
	m_maxConnections(3),
	m_cacheLocation(NULL),
	m_refreshCache(NO_REFRESH_ONCE_PER_SESSION),
	m_cacheSize(10),
	m_unsecureFormWarning(NO_WARN_ALWAYS),
	m_warnEnterSecureSite(true),
	m_warnLeaveSecureSite(true)
{
#ifdef __BEOS__
	appDir = NULL;
#endif
	for (int i=0; i<MAX_DISPLAY_ENCODINGS; i++) {
		m_fontFamily[i] = NULL;
		m_fontSize[i] = 12;
		m_fontMinSize[i]= 9;
		m_fixedFontFamily[i] = NULL;
		m_fixedFontSize[i] = 12;
		m_fixedFontMinSize[i]= 9;
	}
		
	// Connections
	m_online = true;
}

// Copy constructor (mind that strings should be initialized)
Pref::Pref(const Pref &p) : 
	m_homePage(NULL), 
	m_searchPage(NULL),
	m_downloadDirectory(NULL),
	m_httpProxyName(NULL),
	m_ftpProxyName(NULL),
	m_cacheLocation(NULL) 
{
	for (int i=0; i<MAX_DISPLAY_ENCODINGS; i++) {
		m_fontFamily[i] = NULL;
		m_fixedFontFamily[i] = NULL;
	}
	(*this) = p;
}

Pref& Pref::operator = (const Pref& p) {
	// free possibly allocated strings
	FREE(m_homePage);
	FREE(m_searchPage);
	FREE(m_downloadDirectory);
	FREE(m_httpProxyName);
	FREE(m_ftpProxyName);
	FREE(m_cacheLocation);
	for (int i=0; i<MAX_DISPLAY_ENCODINGS; i++) {
		FREE(m_fontFamily[i]);
		FREE(m_fixedFontFamily[i]);
	}
	// copy everything		
	m_homePage = strdup(p.m_homePage); 
	m_searchPage = strdup(p.m_searchPage);
	m_downloadDirectory = strdup(p.m_downloadDirectory);
	m_newWindowAction = p.m_newWindowAction;
	m_cookieAction = p.m_cookieAction;
	m_launchDownloaded = p.m_launchDownloaded;
	m_daysInGo = p.m_daysInGo;
	for (int i=0; i<MAX_DISPLAY_ENCODINGS; i++) {
		if (NULL != p.m_fontFamily[i]) m_fontFamily[i] = strdup(p.m_fontFamily[i]);
		m_fontSize[i] = p.m_fontSize[i];
		m_fontMinSize[i] = p.m_fontMinSize[i];
		if (NULL != p.m_fixedFontFamily[i]) m_fixedFontFamily[i] = strdup(p.m_fixedFontFamily[i]);
		m_fixedFontSize[i] = p.m_fixedFontSize[i];
		m_fixedFontMinSize[i] = p.m_fixedFontMinSize[i];
	}
	m_showImages = p.m_showImages;
	m_showBgImages = p.m_showBgImages;
	m_showAnimations = p.m_showAnimations;
	m_underlineLinks = p.m_underlineLinks;
	m_haikuErrors = p.m_haikuErrors;
	m_useFonts = p.m_useFonts;
	m_useBgColors = p.m_useBgColors;
	m_useFgColors = p.m_useFgColors;
	m_playSounds = p.m_playSounds;
	m_flickerFree = p.m_flickerFree;
	m_useJavaScript = p.m_useJavaScript;
	m_enableProxies = p.m_enableProxies;
	m_httpProxyName = strdup(p.m_httpProxyName);
	m_httpProxyPort = p.m_httpProxyPort;
	m_ftpProxyName = strdup(p.m_ftpProxyName);
	m_ftpProxyPort = p.m_ftpProxyPort;
	m_maxConnections = p.m_maxConnections;
	m_cacheLocation = strdup(p.m_cacheLocation);
	m_refreshCache = p.m_refreshCache;
	m_cacheSize = p.m_cacheSize;
	m_unsecureFormWarning = p.m_unsecureFormWarning;
	m_warnEnterSecureSite = p.m_warnEnterSecureSite;
	m_warnLeaveSecureSite = p.m_warnLeaveSecureSite;
	return *this;
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
	FREE(appDir);
	FREE(m_homePage);
	FREE(m_searchPage);
	FREE(m_downloadDirectory);
	FREE(m_httpProxyName);
	FREE(m_ftpProxyName);	
	for (int i=0; i<MAX_DISPLAY_ENCODINGS; i++) {
		FREE(m_fontFamily[i]);
		FREE(m_fixedFontFamily[i]);
	}
}

const char* Pref::AppDir() {
	return appDir;
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

void Pref::SetNewWindowAction(const new_window_action action) { m_newWindowAction = action; }
const new_window_action Pref::NewWindowAction() { return m_newWindowAction; }

void Pref::SetCookieAction(const cookie_action action) { m_cookieAction = action; }
const cookie_action Pref::CookieAction() { return m_cookieAction; }

void Pref::SetLaunchDownloaded(const bool launch) { m_launchDownloaded = launch; }
const bool Pref::LaunchDownloaded() { return m_launchDownloaded; }

void Pref::SetDaysInGo(const int days) { m_daysInGo = days; }
const int Pref::DaysInGo() { return m_daysInGo; }

// Display page
const char *Pref::FontFamily(display_encoding de) { return m_fontFamily[de]; }
void Pref::SetFontFamily(display_encoding de, const char* s) { 
	FREE(m_fontFamily[de]);
	m_fontFamily[de] = strdup(s);
}

const int Pref::FontSize(display_encoding de) { return m_fontSize[de]; }
void Pref::SetFontSize(display_encoding de, const int n) { m_fontSize[de] = n; }

const int Pref::FontMinSize(display_encoding de) { return m_fontMinSize[de]; }
void Pref::SetFontMinSize(display_encoding de, const int n) { m_fontMinSize[de] = n; }

const char *Pref::FixedFontFamily(display_encoding de) { return m_fixedFontFamily[de]; }
void Pref::SetFixedFontFamily(display_encoding de, const char* s) { 
	FREE(m_fixedFontFamily[de]);
	m_fixedFontFamily[de] = strdup(s);
}

const int Pref::FixedFontSize(display_encoding de) { return m_fixedFontSize[de]; }
void Pref::SetFixedFontSize(display_encoding de, const int n) { m_fixedFontSize[de] = n; }

const int Pref::FixedFontMinSize(display_encoding de) { return m_fixedFontMinSize[de]; }
void Pref::SetFixedFontMinSize(display_encoding de, const int n) { m_fixedFontMinSize[de] = n; }

const bool Pref::ShowImages() { return m_showImages; }
void Pref::SetShowImages(const bool b) { m_showImages = b; }

const bool Pref::ShowBgImages() { return m_showBgImages; }
void Pref::SetShowBgImages(const bool b) { m_showBgImages = b; }

const bool Pref::ShowAnimations() { return m_showAnimations; }
void Pref::SetShowAnimations(const bool b) { m_showAnimations = b; }

const bool Pref::UnderlineLinks() { return m_underlineLinks; }
void Pref::SetUnderlineLinks(const bool b) { m_underlineLinks = b; }

const bool Pref::HaikuErrors() { return m_haikuErrors; }
void Pref::SetHaikuErrors(const bool b) { m_haikuErrors = b; }

const bool Pref::UseFonts() { return m_useFonts; }
void Pref::SetUseFonts(const bool b) { m_useFonts = b; }

const bool Pref::UseBgColors() { return m_useBgColors; }
void Pref::SetUseBgColors(const bool b) { m_useBgColors = b; }

const bool Pref::UseFgColors() { return m_useFgColors; }
void Pref::SetUseFgColors(const bool b) { m_useFgColors = b; }

const bool Pref::PlaySounds() { return m_playSounds; }
void Pref::SetPlaySounds(const bool b) { m_playSounds = b; }

const bool Pref::FlickerFree() { return m_flickerFree; }
void Pref::SetFlickerFree(const bool b) { m_flickerFree = b; }

const bool Pref::UseJavaScript() { return m_useJavaScript; }
void Pref::SetUseJavaScript(const bool b) { m_useJavaScript = b; }

// Connections
const bool Pref::EnableProxies() { return m_enableProxies; }
void Pref::SetEnableProxies(const bool b) { m_enableProxies = b; }

const char * Pref::HttpProxyName() { return m_httpProxyName; }
void Pref::SetHttpProxyName(const char *s) {
	if (m_httpProxyName) free(m_httpProxyName);
	m_httpProxyName = strdup(s);
}

const int Pref::HttpProxyPort() { return m_httpProxyPort; }
void Pref::SetHttpProxyPort(const int n) { m_httpProxyPort = n; }

const char * Pref::FtpProxyName() { return m_ftpProxyName; }
void Pref::SetFtpProxyName(const char *s) {
	if (m_ftpProxyName) free(m_ftpProxyName);
	m_ftpProxyName = strdup(s);
}

const int Pref::FtpProxyPort() { return m_ftpProxyPort; }
void Pref::SetFtpProxyPort(const int n) { m_ftpProxyPort = n; } 

const int Pref::MaxConnections() { return m_maxConnections; }
void Pref::SetMaxConnections(const int n) { m_maxConnections = n; }

// Cache
const char *Pref::CacheLocation() { return m_cacheLocation; }
void Pref::SetCacheLocation(const char *s) {
	FREE(m_cacheLocation);
	m_cacheLocation = strdup(s);
}

const refresh_cache Pref::RefreshCache() { return m_refreshCache; }
void Pref::SetRefreshCache(const refresh_cache r) { m_refreshCache = r; }

const int Pref::CacheSize() { return m_cacheSize; }
void Pref::SetCacheSize(const int n) { m_cacheSize = n; } 

// Security
const unsecure_form_warning Pref::UnsecureFormWarning() { return m_unsecureFormWarning; }
void Pref::SetUnsecureFormWarning(const unsecure_form_warning u) { m_unsecureFormWarning = u; }

const bool Pref::WarnEnterSecureSite() { return m_warnEnterSecureSite; }
void Pref::SetWarnEnterSecureSite(const bool b) { m_warnEnterSecureSite = b; }

const bool Pref::WarnLeaveSecureSite() { return m_warnLeaveSecureSite; }
void Pref::SetWarnLeaveSecureSite(const bool b) { m_warnLeaveSecureSite = b; } 

// Misc
void Pref::Save() {
// FIXME:
}

Pref Pref::Default;
