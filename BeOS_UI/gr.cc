#include <stdio.h>
#include <iostream.h>
#include "NOWindow.h"
#include "NOImage.h"
#include "UIMessages.h"
#include "Pref.h"
#include "Url.h"
#include <be/app/Roster.h>
#include <be/storage/File.h>
#include <be/storage/Node.h>
#include <be/storage/Resources.h>
#include <be/storage/Entry.h>
#include <be/storage/Path.h>
#include <be/storage/AppFileInfo.h>
#include <Alert.h>
#if defined(__BEOS__) || defined(__HAIKU__)
#include <NodeInfo.h>
#endif
#include <Application.h>

class NetOptimist : public BApplication 
{
	void CreateWindow(const char *url_text);
public :
	NetOptimist(const char *url_text, const char *filename);
	NetOptimist(const char *url_text = NULL);
	void MessageReceived(BMessage *message);
	void RefsReceived(BMessage *message);
	void ArgvReceived(int32 argc, char ** argv);
	void ReadyToRun();
};

NetOptimist::NetOptimist(const char *url_text) : BApplication(app_signature) {
	Pref::Default.Init();
	Cache::cache.Init();

	if (url_text) {
		CreateWindow(url_text);
	}
}

NetOptimist::NetOptimist(const char *url_text, const char *filename) : BApplication(app_signature) {
	Pref::Default.Init();
	Cache::cache.Init();

	new NOImage(url_text, 640, 480);
	exit(0);
}

void NetOptimist::CreateWindow(const char *url_text) {
	int WinW = 600;
	int WinH = 400;
	BRect windowfr(10, 30, WinW+10, WinH+30);
	NOWindow *main = new NOWindow(windowfr);
	main->Show();
	main->SetUrl(url_text);
}

#if defined(__BEOS__) || defined (__HAIKU__)
void NetOptimist::RefsReceived(BMessage *message) {
	entry_ref file;
	int nb = 0;
	while(message->FindRef("refs", nb, &file) == B_OK) {
		char type[256];
		BNode node(&file);
		BNodeInfo info(&node);
		info.GetType(type);
		if (!strcmp(type, "application/x-vnd.Be-bookmark")) {
			// This is a bookmark file
			char url[256];		
			if (0 < node.ReadAttr("META:url", B_STRING_TYPE, 0, url, sizeof url-1)) {
				CreateWindow(url);
			}
		} else if ((!strcmp(type, "text/html")) || (!strcmp(type, "text/plain"))) {
			BEntry entry;
			entry.SetTo(&file);
			BPath path;
			entry.GetPath(&path);
			char url[strlen(path.Path())];
			sprintf(url, "file://%s", path.Path());
			CreateWindow(url);
		} else {
			char text[1000];
			sprintf(text, "NetOptimist was unable to handle '%s' (unknown type)", file.name);
			(new BAlert("NetOptimist", text, "Ok"))->Go();
		}

		nb++;
	}
}
#endif

void NetOptimist::ArgvReceived(int32 argc, char ** argv) {
		// XXX Can this happen ? -> YES !
		//(new BAlert("NetOptimist", "argv recv", "Ok"))->Go();
}

void NetOptimist::ReadyToRun() {
	if (CountWindows()==0) {
		CreateWindow(Pref::Default.HomePage()); 
	}
}

void NetOptimist::MessageReceived(BMessage *message) {
	switch (message->what) {
	
/* XXX ouais ben ca ce n'est pas bon du tout :
	il faut trouver un moyen de designer la target
	du bouton. Le probleme est que les boutons sont
	des replicants et que donc un ptr sur la HTMLView
	n'est pas suffisant.
		case bmsgButtonBACK:
			main->PostMessage(message);
			break;
		case bmsgButtonNEXT:
			main->PostMessage(message);
			break;
		case bmsgButtonSTOP:
			cout <<"STOP\n";
			break;
		case bmsgButtonRELOAD:
			main->PostMessage(message);
			break;
		case bmsgButtonHOME:
			main->PostMessage(message);
			break;
		case bmsgButtonSAVE:
			cout <<"SAVE\n";
			break;				
*/
		case B_CLOSE_REQUESTED:
			fprintf(stderr, "B_CLOSE_REQUESTED\n");
			break;
		case B_ABOUT_REQUESTED:
			fprintf(stderr, "about\n");
			break;
		default:
			//fprintf(stderr, "Message received %4.4s in app\n", (char *)&message->what);
			BApplication::MessageReceived(message);
	}
}

#if defined(__BEOS__) || defined (__HAIKU__)
int main(int argc, char* argv[]) {
	const char *url = NULL;
	const char *filename = NULL;
	if (argc>=2)
		url = argv[1];
	if (argc==3)
		filename = argv[2];
	
	if (filename)
		new NetOptimist(url, filename);
	else
		new NetOptimist(url);
	be_app->Run();
	delete be_app;
	return 0;
}
#endif
