#ifndef ENGINE_CLIENT_FETCHER_H
#define ENGINE_CLIENT_FETCHER_H

#define WIN32_LEAN_AND_MEAN
#if defined(EMSCRIPTEN)
	#define __i386__
#endif
#include "curl/curl.h"
#if defined(EMSCRIPTEN)
	#undef __i386__
#endif
#include "curl/easy.h"
#include <engine/fetcher.h>

class CFetcher : public IFetcher
{
private:
	CURL *m_pHandle;

	LOCK m_Lock;
	CFetchTask *m_pFirst;
	CFetchTask *m_pLast;
	class IStorage *m_pStorage;
public:
	CFetcher();
	virtual bool Init();
	~CFetcher();

	virtual void QueueAdd(CFetchTask *pTask, const char *pUrl, const char *pDest, int StorageType = 2, void *pUser = 0, COMPFUNC pfnCompCb = 0, PROGFUNC pfnProgCb = 0);
	static void FetcherThread(void *pUser);
	bool FetchFile(CFetchTask *pTask);
	static void WriteToFile(char *pData, size_t size, size_t nmemb, void *pFile);
	static int ProgressCallback(void *pUser, double DlTotal, double DlCurr, double UlTotal, double UlCurr);

#if defined(EMSCRIPTEN)
	static void EmLoad(unsigned u, void *user, const char *url);
	static void EmError(unsigned u, void *user, int errorCode);
	static void EmProgress(unsigned u, void *user, int someNumber);
#endif
};

#endif
