#if !defined(JSBRIDGE_CTX_H)
#define JSBRIDGE_CTX_H

struct JsData;
class DocFormater;
class HTMLWindow;

class JsCtx {
	JsData *m_jsdata;
public:
	JsCtx();
	~JsCtx();
	void Init(DocFormater *doc);
	void SetWindow(HTMLWindow *window);
	void Execute(const char *jscode);
	void ExecuteStip(const char *jscode);
	void SourceFile(const char *filename);
};

#endif
