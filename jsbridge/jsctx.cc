
#include "jsbridge/jsctx.h"
#include "js.h"
#include "Html.h"
#include "NOWindow.h"
#include "HTMLView.h"
#include "traces.h"
#include "Url.h"

/* XXX TODO
	- We need to ask the user if automatic window open is not allowed
	- User agent, appName, ... should have a Mozilla-compatible behaviour
*/

/* I/O function for the standard error stream. */
int io_stderr (void *context, unsigned char *buffer, unsigned int amount) {
	return fwrite (buffer, 1, amount, stderr);
}
        
static JSMethodResult
nojs_document_write_helper(void *instance_context, int argc, JSType *argv,
		char *error_return, bool newline) {
	if (argc < 1)
	{
		strcpy (error_return, "wrong amount of arguments");
		return JS_ERROR;
	}
	DocFormater *doc = (DocFormater*)instance_context;

	for (int i = 0; i<argc; i++) {
		JSType *arg = &argv[i];
		switch(arg->type)
		{
		case JS_TYPE_STRING:
			doc->InsertText((char*)arg->u.s->data, arg->u.s->len);
			break;
		case JS_TYPE_DOUBLE:
			{
			char num[100];
			int nb = sprintf(num, "%f", arg->u.d);
			doc->InsertText(num, nb);
			}
			break;
		case JS_TYPE_INTEGER:
			{
			char num[30];
			int nb = sprintf(num, "%ld", arg->u.i);
			doc->InsertText(num, nb);
			}
			break;
		case JS_TYPE_NULL:
			{
			const char nullstring[] = "(null)";	// XXX not sure we should display this
			doc->InsertText(nullstring, sizeof(nullstring)-1);
			}
			break;
		case JS_TYPE_UNDEFINED:
			{
			const char nullstring[] = "(undefined)";	// XXX not sure what we should display here
			doc->InsertText(nullstring, sizeof(nullstring)-1);
			}
			break;
		default:
			strcpy (error_return, "illegal argument type");
			return JS_ERROR;
		}
	}
	if (newline) doc->InsertText("\n", 2);

	return JS_OK;
}

static JSMethodResult 
nojs_document_write(JSClassPtr , void *instance_context, JSInterpPtr ,
	    int argc, JSType *argv, JSType *, char *error_return) {
	return nojs_document_write_helper(instance_context, argc, argv, error_return, false);
}

static JSMethodResult 
nojs_document_writeln(JSClassPtr , void *instance_context, JSInterpPtr ,
	    int argc, JSType *argv, JSType *, char *error_return) {
	return nojs_document_write_helper(instance_context, argc, argv, error_return, true);
}

static JSMethodResult 
nojs_window_open(JSClassPtr , void *instance_context, JSInterpPtr ,
	    int argc, JSType *argv, JSType *, char *error_return) {
	if (argc < 1 || argc > 3) {
		strcpy (error_return, "wrong amount of arguments");
		return JS_ERROR;
	}

	if (argv[0].type != JS_TYPE_STRING) {
		strcpy (error_return, "wrong argument(1) type");
		return JS_ERROR;
	}

	char *url = new char[argv[0].u.s->len + 1];
	strncpy(url, (char *)argv[0].u.s->data, argv[0].u.s->len);
	url[argv[0].u.s->len] = '\0';

	NOWindow *win = new NOWindow(BRect(10,100,600,600));
	win->SetUrl(url);
	win->Show();

	return JS_OK;
}

static JSMethodResult 
nojs_navigator_javaEnabled(JSClassPtr , void *instance_context, JSInterpPtr ,
	    int argc, JSType *argv, JSType *return_value, char *error_return) {
	if (argc >= 1) {
		strcpy (error_return, "wrong amount of arguments");
		return JS_ERROR;
	}
	return_value->type = JS_TYPE_BOOLEAN;
	return_value->u.i = false;	// No, Java is not supported :-(
	return JS_OK;
}

static JSMethodResult
nojs_window_url(JSClassPtr , void *instance_context, JSInterpPtr interp,
		int setp, JSType *value, char *error_return) {

	HTMLWindow *window = (HTMLWindow *)instance_context;
	if (!window) {
		strcpy(error_return, "no window for document");
		return JS_ERROR;
	}

	if (setp) {
		fprintf(stderr, "nojs_window_url : trying to set property\n");
		if (value->type != JS_TYPE_STRING) {
			strcpy (error_return, "wrong value type");
			return JS_ERROR;
		}

		// XXX Not implemented !
		// Warning : we should post a message to set new url and not
		// call SetUrl (or whatever) directly because the js code
		// is run within the context of the window.
		strcpy(error_return, "not implemented");
		return JS_ERROR;
	} else {
		const char * url = window->MainFrame()->m_document.CurrentUrl()->DisplayName()->Str();
		js_type_make_string (interp, value, (unsigned char*)url, strlen(url));

		return JS_OK;
	}
}

static JSMethodResult
nojs_navigator_appCodeName(JSClassPtr , void *instance_context, JSInterpPtr interp,
		int setp, JSType *value, char *error_return) {
	if (setp) {
		fprintf(stderr, "nojs_navigator_appCodeName : trying to set read-only property\n");
		return JS_ERROR;
	} else {
		const char appCodeName[] = "NetO";
		js_type_make_string (interp, value, (unsigned char*)appCodeName, sizeof(appCodeName)-1);
		return JS_OK;
	}
}

static JSMethodResult
nojs_navigator_userAgent(JSClassPtr , void *instance_context, JSInterpPtr interp,
		int setp, JSType *value, char *error_return) {
	if (setp) {
		fprintf(stderr, "nojs_navigator_useAgent : trying to set read-only property\n");
		return JS_ERROR;
	} else {
		const char userAgent[] = "NetOptimist/dev";
		js_type_make_string (interp, value, (unsigned char*)userAgent, sizeof(userAgent)-1);
		return JS_OK;
	}
}

static JSMethodResult
nojs_navigator_appName(JSClassPtr , void *instance_context, JSInterpPtr interp,
		int setp, JSType *value, char *error_return) {
	if (setp) {
		fprintf(stderr, "nojs_navigator_appName : trying to set read-only property\n");
		return JS_ERROR;
	} else {
		const char appName[] = "NetOptimist";
		js_type_make_string (interp, value, (unsigned char*)appName, sizeof(appName)-1);
		return JS_OK;
	}
}

struct JsData {
	JSInterpPtr interp;
};

JsCtx::JsCtx() {
	m_jsdata = NULL;
}

JsCtx::~JsCtx() {
	if (m_jsdata != NULL) {
		// XXX memory allocation bug... see solaris fix !
		// XXX js_destroy_interp(m_jsdata->interp);
		delete m_jsdata;
	}
}

void JsCtx::Init(DocFormater *document) {
	JSInterpOptions options;

	m_jsdata = new JsData;
	
	js_init_default_options (&options);
	if (ISTRACE(DEBUG_JAVASCRIPT)) {
		options.verbose = 2;
	}
	options.s_stderr = io_stderr;
	options.s_context = NULL;

	m_jsdata->interp = js_create_interp (&options);

	JSClassPtr cls;

	// document class
	cls = js_class_create (NULL, NULL, 0 /* no_auto_destroy */, NULL /*ctor*/);
	js_class_define_method (cls, "write", 0, nojs_document_write);
	js_class_define_method (cls, "writeln", 0, nojs_document_writeln);
	js_define_class (m_jsdata->interp, cls, "NetOptimistDocument");

	// document object
	JSType * documentObject = new JSType;
	js_instantiate_class (m_jsdata->interp, cls, document, NULL, documentObject);
	js_set_var (m_jsdata->interp, "document", documentObject);
	
	// navigator class
	cls = js_class_create (NULL, NULL, 0 /* no_auto_destroy */, NULL /*ctor*/);
	js_class_define_property (cls, "appCodeName", JS_CF_IMMUTABLE, nojs_navigator_appCodeName);
	js_class_define_property (cls, "appName", JS_CF_IMMUTABLE, nojs_navigator_appName);
	js_class_define_property (cls, "userAgent", JS_CF_IMMUTABLE, nojs_navigator_userAgent);
	js_class_define_method (cls, "javaEnabled", JS_CF_STATIC, nojs_navigator_javaEnabled);
	js_define_class (m_jsdata->interp, cls, "NetOptimistNavigator");

	// navigator object
	JSType * navigatorObject = new JSType;
	js_instantiate_class (m_jsdata->interp, cls, NULL, NULL, navigatorObject);
	js_set_var (m_jsdata->interp, "navigator", navigatorObject);
}

void JsCtx::SetWindow(HTMLWindow *window) {
	JSClassPtr cls;

	// window class
	cls = js_class_create (NULL, NULL, 0 /* no_auto_destroy */, NULL /*ctor*/);
	js_class_define_method (cls, "open", JS_CF_STATIC, nojs_window_open);
	js_class_define_property (cls, "url", 0, nojs_window_url); // XXX replace by a Location object
	js_define_class (m_jsdata->interp, cls, "NetOptimistWindow");

	// window object
	JSType * windowObject = new JSType;
	js_instantiate_class (m_jsdata->interp, cls, window, NULL, windowObject);
	js_set_var (m_jsdata->interp, "window", windowObject);
};

void JsCtx::Execute(const char *t) {
	if (!js_eval(m_jsdata->interp, (char*)t)) {
		fprintf (stderr, ">> JsCtx::Execute failed: %s\n", js_error_message (m_jsdata->interp));
	}
	if (ISTRACE(DEBUG_JAVASCRIPT)) {
		fprintf (stdout, ">> JsCtx::Execute code:\n%s\n", t);
	}
}

#ifdef STD_ALONE
int main() {

	JsCtx *jsctx = new JsCtx;
	jsctx->Init();
	jsctx->Execute("System.stdout.writeln(\"truc\")");
	delete jsctx;

}
#endif
