
#include "jsbridge/jsctx.h"
#include "js.h"
#include "Html.h"

/* I/O function for the standard error stream. */
int io_stderr (void *context, unsigned char *buffer, unsigned int amount) {
	return fwrite (buffer, 1, amount, stderr);
}
        
static JSMethodResult
nojs_document_write_helper(void *instance_context, int argc, JSType *argv,
		char *error_return, bool newline) {
	if (argc != 1)
	{
		strcpy (error_return, "wrong amount of arguments");
		return JS_ERROR;
	}

	DocFormater *doc = (DocFormater*)instance_context;
	switch(argv[0].type)
	{
	case JS_TYPE_STRING:
		doc->InsertText((char*)argv[0].u.s->data, argv[0].u.s->len);
		break;
	case JS_TYPE_DOUBLE:
		{
		char num[100];
		int nb = sprintf(num, "%f", argv[0].u.d);
		doc->InsertText(num, nb);
		}
		break;
	case JS_TYPE_INTEGER:
		{
		char num[100];
		int nb = sprintf(num, "%ld", argv[0].u.i);
		doc->InsertText(num, nb);
		}
		break;
	default:
		strcpy (error_return, "illegal argument");
		return JS_ERROR;
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

struct JsData {
	JSInterpPtr interp;
};

void JsCtx::Init(DocFormater *document) {
	JSInterpOptions options;

	m_jsdata = new JsData;
	
	js_init_default_options (&options);
	options.verbose = 2;
	options.s_stderr = io_stderr;
	options.s_context = NULL;

	m_jsdata->interp = js_create_interp (&options);

	JSClassPtr cls;

	// document object
	cls = js_class_create (NULL, NULL, 0 /* no_auto_destroy */, NULL /*ctor*/);
	js_class_define_method (cls, "write", 0, nojs_document_write);
	js_class_define_method (cls, "writeln", 0, nojs_document_writeln);
	js_define_class (m_jsdata->interp, cls, "NetOptimistDocument");

	JSType * documentObject = new JSType;
	js_instantiate_class (m_jsdata->interp, cls, document, NULL, documentObject);

	js_set_var (m_jsdata->interp, "document", documentObject);
	
	// window object
	cls = js_class_create (NULL, NULL, 0 /* no_auto_destroy */, NULL /*ctor*/);
}

void JsCtx::Execute(const char *t) {
	if (!js_eval(m_jsdata->interp, (char*)t)) {
		fprintf (stderr, "JsCtx::Execute failed: %s\n", js_error_message (m_jsdata->interp));
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
