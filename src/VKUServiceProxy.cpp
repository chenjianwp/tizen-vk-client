/*
 * VKUServiceProxy.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: wolong
 */

#include "VKUServiceProxy.h"

using namespace Tizen::App;
using namespace Tizen::Base;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Base::Collection;
using namespace Tizen::Io;

static const int CHECK_INTERVAL = 1000; // Checking interval in sec.

static const wchar_t* LOCAL_MESSAGE_PORT_NAME = L"UI_PORT";

VKUServiceProxy::VKUServiceProxy(void) : pLocalMessagePort(null), pRemoteMessagePort(null) {}

VKUServiceProxy::~VKUServiceProxy(void) {}

result VKUServiceProxy::Construct(const AppId& appId, const String& remotePortName) {
	result r = E_FAILURE;

	this->appId = appId;

	AppManager* pAppManager = AppManager::GetInstance();
	TryReturn(pAppManager != null, E_FAILURE, "VKU : Fail to get AppManager instance.");

	AppLog("VKU : Checking service status for time out 5 sec...");

	for (int i = 0; i < 5; ++i) {
		if (pAppManager->IsRunning(appId)) {
			AppLog("VKU : Service is ready");
			break;
		} else {
			AppLog("VKU : Service is not ready. try to launch.");
			r = pAppManager->LaunchApplication(appId, null);
			TryReturn(!IsFailed(r), r, "VKU : [%s]", GetErrorMessage(r));
			Thread::Sleep(CHECK_INTERVAL);
		}
	}

	TryReturn(pAppManager->IsRunning(appId), E_FAILURE, "VKU : The service is not working.");

	pLocalMessagePort = MessagePortManager::RequestLocalMessagePort(LOCAL_MESSAGE_PORT_NAME);
	TryReturn(pLocalMessagePort != null, E_FAILURE, "[E_FAILURE] Failed to get LocalMessagePort instance.");

	pLocalMessagePort->AddMessagePortListener(*this);

	pRemoteMessagePort = MessagePortManager::RequestRemoteMessagePort(appId, remotePortName);
	TryReturn(pRemoteMessagePort != null, E_FAILURE, "[E_FAILURE] Failed to get LocalMessagePort instance.");

	AppLog("LocalMessagePort(\"%ls\") is ready", pLocalMessagePort->GetName().GetPointer());

	return E_SUCCESS;
}

result VKUServiceProxy::SendMessage(const IMap* pMessage) {
	result r = E_SUCCESS;
	AppLog("SendMessage is called.");

	if (pRemoteMessagePort != null) {
		r = pRemoteMessagePort->SendMessage(pLocalMessagePort, pMessage);
	} else {
		r = E_FAILURE;
	}

	return r;
}

void VKUServiceProxy::OnMessageReceivedN(RemoteMessagePort* pRemoteMessagePort, IMap* pMessage) {
	AppLog("VKU : A response message is Received.");

	/*
	String key(L"ServiceApp");
	String* pData = static_cast<String*>(pMessage->GetValue(key));

	App* pApp = App::GetInstance();

	if (pData != null && pApp != null) {
		AppLog("SampleUiApp : Received data : %ls", pData->GetPointer());

		if (pData->CompareTo(L"ready") == 0) {
			pApp->SendUserEvent(STATE_CONNECTED, null);
		} else if (pData->CompareTo(L"started") == 0) {
			pApp->SendUserEvent(STATE_TIMER_STARTED, null);
		} else if (pData->CompareTo(L"timer expired") == 0) {
			pApp->SendUserEvent(STATE_TIMER_EXPIRED, null);
		} else if (pData->CompareTo(L"stopped") == 0) {
			pApp->SendUserEvent(STATE_TIMER_STOPPED, null);
		} else if (pData->CompareTo(L"exit") == 0) {
			pApp->SendUserEvent(STATE_EXIT, null);
		}
	}
	*/

	delete pMessage;
}
