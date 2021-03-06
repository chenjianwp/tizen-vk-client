/*
 * ContactsRetrieveListener.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: igorglotov
 */

#include "ContactsRetrieveListener.h"
#include "ObjectCounter.h"

using namespace Tizen::Ui::Controls;
using namespace Tizen::Web::Json;

ContactsRetrieveListener::ContactsRetrieveListener(GroupedTableView * apTableView, ContactsTableProvider * apProvider) {
	CONSTRUCT(L"ContactsRetrieveListener");
	pGroupedTableView = apTableView;
	pProvider = apProvider;
}

ContactsRetrieveListener::~ContactsRetrieveListener() {
	DESTRUCT(L"ContactsRetrieveListener");
}

void ContactsRetrieveListener::OnResponseN(RequestId requestId, JsonObject *object) {
	result r = E_SUCCESS;
	if(requestId == REQUEST_GET_CONTACTS) {
		AppLog("ContactsRetrieveListener::OnResponseN");
		pProvider->SetUsersJson(object);

		pGroupedTableView->UpdateTableView();
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pGroupedTableView->UpdateTableView");

		pGroupedTableView->RequestRedraw(true);
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pGroupedTableView->RequestRedraw");
	}
	delete object;
	SetLastResult(r);
	return;

CATCH:
	AppLogException("$${Function:OnResponseN} is failed.", GetErrorMessage(r));
	delete object;
	SetLastResult(r);
}
