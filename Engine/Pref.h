#ifndef PREF_H
#define PREF_H

#include "platform.h"

enum new_window_action {
	NO_CLONE_CURRENT = 0x01,
	NO_OPEN_BLANK,
	NO_OPEN_HOME
};

enum cookie_action {
	NO_ACCEPT_COOKIE = 0x10,
	NO_REJECT_COOKIE,
	NO_ASK_COOKIE
};


#define MAX_DISPLAY_ENCODINGS 6
enum display_encoding {
	NO_CENTRAL_EUROPEAN = 0x40,
	NO_CYRILLIC,
	NO_GREEK,
	NO_JAPANESE,
	NO_UNICODE,
	NO_WESTERN
};


enum refresh_cache {
	NO_REFRESH_EVERY_TIME = 0x60,
	NO_REFRESH_ONCE_PER_SESSION,
	NO_REFRESH_ONCE_PER_DAY,
	NO_REFRESH_NEVER
};

enum unsecure_form_warning {
	NO_WARN_NEVER = 0x100,
	NO_WARN_MORE_THAN_ONE_LINE,
	NO_WARN_ALWAYS
};

class Pref {
	char *appDir;
	
	// General page
	char *m_homePage;
	char *m_searchPage;
	char *m_downloadDirectory;
	new_window_action m_newWindowAction; // clone current/open blank/open home
	cookie_action m_cookieAction; // accept/reject/ask
	bool m_launchDownloaded; // automatically launch files after download
	int m_daysInGo; // days to keep files in Go menu
	
	// Display
	char *m_fontFamily[MAX_DISPLAY_ENCODINGS];
	int m_fontSize[MAX_DISPLAY_ENCODINGS];
	int m_fontMinSize[MAX_DISPLAY_ENCODINGS];
	char *m_fixedFontFamily[MAX_DISPLAY_ENCODINGS];
	int m_fixedFontSize[MAX_DISPLAY_ENCODINGS];
	int m_fixedFontMinSize[MAX_DISPLAY_ENCODINGS];
	bool m_showImages;
	bool m_showBgImages;
	bool m_showAnimations;
	bool m_underlineLinks;
	bool m_haikuErrors;
	bool m_useFonts;
	bool m_useBgColors;
	bool m_useFgColors;
	bool m_playSounds;
	bool m_flickerFree;
	bool m_useJavaScript;

	// Connections
	bool m_enableProxies;
	char *m_httpProxyName;
	int m_httpProxyPort;
	char *m_ftpProxyName;
	int m_ftpProxyPort;
	int m_maxConnections;

	// Cache
	char *m_cacheLocation;
	refresh_cache m_refreshCache;
	int m_cacheSize;
	
	// Security
	unsecure_form_warning m_unsecureFormWarning;
	bool m_warnEnterSecureSite;
	bool m_warnLeaveSecureSite;
	
	bool m_online; 	// Allows http

public:
	static Pref Default;

	Pref();
	Pref(const Pref&);
	Pref& operator = (const Pref&);
		
#ifdef __BEOS__
	~Pref();
	const char *AppDir();
	void Init();
#endif
	// General
	void SetHomePage(const char *);
	const char * HomePage();
	void SetSearchPage(const char *);
	const char * SearchPage();
	void SetDownloadDirectory(const char *);
	const char * DownloadDirectory();
	void SetNewWindowAction(const new_window_action action);
	const new_window_action NewWindowAction();	
	void SetCookieAction(const cookie_action action);
	const cookie_action CookieAction();
	void SetLaunchDownloaded(const bool launch);
	const bool LaunchDownloaded();
	void SetDaysInGo(const int days);
	const int DaysInGo();
	
	// Display
	const char *FontFamily(display_encoding);
	void SetFontFamily(display_encoding, const char*);
	const int FontSize(display_encoding);
	void SetFontSize(display_encoding, const int);
	const int FontMinSize(display_encoding);
	void SetFontMinSize(display_encoding, const int);
	const char *FixedFontFamily(display_encoding);
	void SetFixedFontFamily(display_encoding, const char*);
	const int FixedFontSize(display_encoding);
	void SetFixedFontSize(display_encoding, const int);
	const int FixedFontMinSize(display_encoding);
	void SetFixedFontMinSize(display_encoding, const int);
	const bool ShowImages();
	void SetShowImages(const bool);
	const bool ShowBgImages();
	void SetShowBgImages(const bool);
	const bool ShowAnimations();
	void SetShowAnimations(const bool);
	const bool UnderlineLinks();
	void SetUnderlineLinks(const bool);
	const bool HaikuErrors();
	void SetHaikuErrors(const bool);
	const bool UseFonts();
	void SetUseFonts(const bool);
	const bool UseBgColors();
	void SetUseBgColors(const bool);
	const bool UseFgColors();
	void SetUseFgColors(const bool);
	const bool PlaySounds();
	void SetPlaySounds(const bool);
	const bool FlickerFree();
	void SetFlickerFree(const bool);
	const bool UseJavaScript();
	void SetUseJavaScript(const bool);
	
	// Connections
	const bool EnableProxies();
	void SetEnableProxies(const bool);
	const char *HttpProxyName();
	void SetHttpProxyName(const char *);
	const int HttpProxyPort();
	void SetHttpProxyPort(const int);
	const char *FtpProxyName();
	void SetFtpProxyName(const char *);
	const int FtpProxyPort();
	void SetFtpProxyPort(const int);
	const int MaxConnections();
	void SetMaxConnections(const int);
		
	// Cache
	const char *CacheLocation();
	void SetCacheLocation(const char *);
	const refresh_cache RefreshCache();
	void SetRefreshCache(const refresh_cache);
	const int CacheSize();
	void SetCacheSize(const int);
	
	// Security
	const unsecure_form_warning UnsecureFormWarning();
	void SetUnsecureFormWarning(const unsecure_form_warning);
	const bool WarnEnterSecureSite();
	void SetWarnEnterSecureSite(const bool);
	const bool WarnLeaveSecureSite();
	void SetWarnLeaveSecureSite(const bool);

	// Misc...
	void SetOnline(bool online);
	bool Online() const;
	
	// FIXME: we have methods for this now. remove!
	const char *CacheDir();
	
	// Save preferences
	void Save();

};

#endif
