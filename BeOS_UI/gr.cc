#include <Application.h>

#include <iostream.h>
#include "NOWindow.h"
#include "UIMessages.h"
#include "Pref.h"
#include <be/app/Roster.h>
#include <be/storage/File.h>
#include <be/storage/Node.h>
#include <be/storage/Resources.h>
#include <be/storage/Entry.h>
#include <be/storage/AppFileInfo.h>
#include <Alert.h>
#include <NodeInfo.h>

class NetOptimist : public BApplication 
{

private:
	NOWindow *main;
public :
	NetOptimist(const char *url_text = NULL);
	void MessageReceived(BMessage *message);
	void RefsReceived(BMessage *message);
	void ArgvReveived(int32 argc, char ** argv);
};

NetOptimist::NetOptimist(const char *url_text = NULL) : BApplication(app_signature) {
	BMessage msg((uint32)0); 
	
	// Register this app as able to handle N+ bookmarks
	app_info appInfo; 
	BFile file; 
	BAppFileInfo appFileInfo; 
	
	Pref::Default.Init();
	
/*
	be_app->GetAppInfo(&appInfo); 
	file.SetTo(&appInfo.ref, B_READ_WRITE); 
	appFileInfo.SetTo(&file);
	assert(appFileInfo.InitCheck()==B_OK);
	 
	const char *ptr;
	uint32 i=0;
	if (appFileInfo.GetSupportedTypes(&msg) == B_OK) {
		while (msg.FindString("types", i++, &ptr) == B_OK) 
			fprintf(stderr, "Supported Type: %s\n", ptr); 
	} else {
		BMessage msg2;
		const char BOOKMARK_SIG[] = "application/x-vnd.Be-bookmark";
		fprintf(stderr, "no supported types\n");
		if (msg2.AddString("types", BOOKMARK_SIG) != B_OK) {
			(new BAlert("Error", "The message is corrupted", "Ok"))->Go();
		}
		status_t ret = appFileInfo.SetSupportedTypes(&msg);
		if (ret != B_OK) {
			fprintf(stderr, "ret  = %ld 0x%lx\n", ret, ret);
			(new BAlert("Error", "Cannot register as supporting bookmarks", "Ok"))->Go();
		}
	}
*/
	// Create main window
	int WinW = 600;
	int WinH = 400; // STAS: was 700
	BRect windowfr(10, 30, WinW+10, WinH+30);
	main = new NOWindow(windowfr);

	main->Show();

	if (url_text)
		main->SetUrl(url_text);
}

void NetOptimist::RefsReceived(BMessage *message) {
	(new BAlert("NetOptimist", "ref recv", "Ok"))->Go();
	if (message->what == B_REFS_RECEIVED) {
#if B_BEOS_VERSION >= 0x0510
				const char *name; // BMessage::GetInfo changed in Dano : it now requires a const
#else
				char *name;
#endif
				uint32 t;
				int32 count;

		for ( int32 i = 0;
			  message->GetInfo(B_ANY_TYPE, i, &name, &t, &count) == B_OK;
			  i++ ) {
			  fprintf(stderr, "Data message : %s type %x\n", name, (int)t);
		}
		entry_ref file;
		int nb = 0;
		while(message->FindRef("refs", nb, &file) == B_OK) {
			char type[256];
			char text[1000];
			BNode node(&file);
			BNodeInfo info(&node);
			info.GetType(type);
			
			sprintf(text, "refs in app : %s type %s\n", file.name, type);
			(new BAlert("NetOptimist", text, "Ok"))->Go();
			// XXX Open windows...
			nb++;
		}
	}
}
void NetOptimist::ArgvReveived(int32 argc, char ** argv) {
		// XXX Can this happen ?
		(new BAlert("NetOptimist", "argv recv", "Ok"))->Go();
}

void NetOptimist::MessageReceived(BMessage *message) {
	switch (message->what) {
	
		case bmsgButtonBACK:
			/* XXX ouais ben ca ce n'est pas bon du tout :
					il faut trouver un moyen de designer la target
					du bouton. Le probleme est que les boutons sont
					des replicants et que donc un ptr sur la HTMLView
					n'est pas suffisant.
			*/
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
		case B_CLOSE_REQUESTED:
			fprintf(stderr, "B_CLOSE_REQUESTED\n");
			break;
		case B_ABOUT_REQUESTED:
			fprintf(stderr, "about\n");
			break;
		default:
			fprintf(stderr, "Message received %4s in app\n", (char *)&message->what);
			BApplication::MessageReceived(message);
	}
}


int main(int argc, char* argv[]) {
	static const char *progname;
	progname = argv[0];
	if (argc<2)
		new NetOptimist(Pref::Default.HomePage()); 
	else 
		new NetOptimist(argv[1]);
	be_app->Run();
	delete be_app;
	return 0;
}

