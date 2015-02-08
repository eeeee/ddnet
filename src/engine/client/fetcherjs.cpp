#if defined(EMSCRIPTEN)
#include <base/system.h>
#include <engine/storage.h>
#include "fetcher.h"
#include "emscripten.h"

CFetchTask::CFetchTask()
{
	m_pNext = NULL;
}

void CFetcher::EmLoad(unsigned u, void *user, const char *url) {
	CFetchTask *pTask = (CFetchTask *)user;
	pTask->m_State = CFetchTask::STATE_DONE;
	if(pTask->m_pfnCompCallback)
		pTask->m_pfnCompCallback(pTask, pTask->m_pUser);
}

void CFetcher::EmError(unsigned u, void *user, int errorCode) {
	CFetchTask *pTask = (CFetchTask *)user;
	pTask->m_State = CFetchTask::STATE_ERROR;
	if(pTask->m_pfnCompCallback)
		pTask->m_pfnCompCallback(pTask, pTask->m_pUser);
}

void CFetcher::EmProgress(unsigned u, void *user, int someNumber) {
	CFetchTask *pTask = (CFetchTask *)user;
	pTask->m_Progress = someNumber;
	if(pTask->m_pfnProgressCallback)
		pTask->m_pfnProgressCallback(pTask, pTask->m_pUser);
}

CFetcher::CFetcher()
{
	m_pStorage = NULL;
}

bool CFetcher::Init()
{
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	return true;
}

CFetcher::~CFetcher()
{
}

void CFetcher::Escape(char *pBuf, size_t size, const char *pStr)
{
	str_copy(pBuf, pStr, size);
}

void CFetcher::QueueAdd(CFetchTask *pTask, const char *pUrl, const char *pDest, int StorageType, void *pUser, COMPFUNC pfnCompCb, PROGFUNC pfnProgCb)
{
	str_copy(pTask->m_pUrl, pUrl, sizeof(pTask->m_pUrl));
	str_copy(pTask->m_pDest, pDest, sizeof(pTask->m_pDest));
	pTask->m_StorageType = StorageType;
	pTask->m_pfnProgressCallback = pfnProgCb;
	pTask->m_pfnCompCallback = pfnCompCb;
	pTask->m_pUser = pUser;
	pTask->m_Size = pTask->m_Progress = 0;
	pTask->m_Abort = false;

	for(int i = 0; pTask->m_pDest[i] != '\0'; i++)
	{
		if(pTask->m_pDest[i] == '/')
		{
			pTask->m_pDest[i] = '\0';
			m_pStorage->CreateFolder(pTask->m_pDest, StorageType);
			pTask->m_pDest[i] = '/';
		}
	}
	char buf[1024];
	m_pStorage->GetCompletePath(StorageType, pTask->m_pDest, buf, sizeof(buf));
	pTask->m_State = CFetchTask::STATE_QUEUED;
	emscripten_async_wget2(pUrl, buf, "GET", "", (void *)pTask, &EmLoad, &EmError, &EmProgress);
}
#endif
