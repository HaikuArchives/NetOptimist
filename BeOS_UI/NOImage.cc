#include <Bitmap.h>
#include <TranslatorRoster.h>
#include <BitmapStream.h>
#include <File.h>
#include <stdio.h>

#include "NOImage.h"
#include "HTMLView.h"

int32 find_constant(BTranslatorRoster *roster, const char *mime) 
{
	translator_id *translators; 
	int32 num_translators;

	roster->GetAllTranslators(&translators, &num_translators); 

	for (int32 i=0;i<num_translators;i++) { 
		const translation_format *fmts; 
		int32 num_fmts; 

		roster->GetOutputFormats(translators[i], &fmts, &num_fmts); 

		for (int32 j=0;j<num_fmts;j++) { 
			if (!strcasecmp(fmts[j].MIME, mime)) 
				printf("match: %s type %8.8x (%4.4s)n", fmts[j].name, fmts[j].type, &fmts[j].type);
			return fmts[j].type;
		} 
	}
	return 0;
} 

NOImage::NOImage(const char *url, int w, int h) {
	BBitmap *bmp = new BBitmap(BRect(0,0,w,h), B_CMAP8, true);
	HTMLView *htmlView = new HTMLView(NULL, BRect(0,0,w,h));
	bmp->AddChild(htmlView);
	htmlView->LockLooper();
	htmlView->SetUrl(url);
	htmlView->Draw(BRect(0,0,w,h));
	htmlView->UnlockLooper();

	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BBitmapStream stream(bmp); // init with contents of bitmap
	BFile file("/tmp/t1.png", B_CREATE_FILE | B_WRITE_ONLY);

	find_constant(roster, "image/png");
	roster->Translate(&stream, NULL, NULL, &file, find_constant(roster, "image/png"));
}
