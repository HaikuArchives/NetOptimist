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

enum display_encoding {
	NO_WESTERN = 0x40,
	NO_UNICODE,
	NO_JAPANESE,
	NO_GREEK,
	NO_CYRILLIC,
	NO_CENTRAL_EUROPEAN
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
	
	// FIXME: Fonts/encodings
	
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
	const char *m_proxyName;
	bool m_online; 	// Allows http
	
	// Cache
	
	// Security
public:
	static Pref Default;

	Pref();
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
	void SetNewWindowAction(new_window_action action);
	const new_window_action NewWindowAction();	
	void SetCookieAction(cookie_action action);
	const cookie_action CookieAction();
	void SetLaunchDownloaded(bool launch);
	const bool LaunchDownloaded();
	void SetDaysInGo(int days);
	const int DaysInGo();
	
	// Display
	bool ShowImages();
	void SetShowImages(bool);
	bool ShowBgImages();
	void SetShowBgImages(bool);
	bool ShowAnimations();
	void SetShowAnimations(bool);
	bool UnderlineLinks();
	void SetUnderlineLinks(bool);
	bool HaikuErrors();
	void SetHaikuErrors(bool);
	bool UseFonts();
	void SetUseFonts(bool);
	bool UseBgColors();
	void SetUseBgColors(bool);
	bool UseFgColors();
	void SetUseFgColors(bool);
	bool UsePlaySounds();
	void SetUsePlaySounds(bool);
	bool FlickerFree();
	void SetFlickerFree(bool);
	bool UseJavaScript();
	void SetUseJavaScript(bool);
	
	
	void SetOnline(bool online);
	bool Online() const;
	const char *ProxyName();
	void SetProxyName(const char *);
	int ProxyPort();
	const char *CacheDir();

	// Save preferences
	void Save();

};

#endif
