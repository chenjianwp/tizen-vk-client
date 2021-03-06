/*
 * VKUMessagesListItemProvider.cpp
 *
 *  Created on: Nov 5, 2013
 *      Author: igorglotov
 */

#include "VKUMessagesListItemProvider.h"
#include "JsonParseUtils.h"
#include "TimeUtils.h"
#include "ImageUtils.h"

#include "RoundedAvatar.h"

#include "MessageTextElement.h"

#include "MessagePhotoElement.h"
#include "MessageVideoElement.h"
#include "MessageWallElement.h"
#include "MessageAudioElement.h"
#include "MessageDocElement.h"
#include "MessageForwardedElement.h"
#include "MessageLocationElement.h"
#include "ObjectCounter.h"

using namespace Tizen::Ui;
using namespace Tizen::Ui::Scenes;
using namespace Tizen::Ui::Controls;
using namespace Tizen::Graphics;
using namespace Tizen::Web::Json;
using namespace Tizen::Base;
using namespace Tizen::Base::Collection;


static const int LIST_HEIGHT = 10000;
static const int TIMESTAMP_TEXT_COLOR = 0x6d7175;
static const int LIST_ITEM_UNREAD_COLOR = 0x191f25;
#define USE_CACHE 1

static const wchar_t *PRELOAD_MESSAGES = L"20";

VKUMessagesListItemProvider::VKUMessagesListItemProvider() {
	CONSTRUCT(L"VKUMessagesListItemProvider");
	_messagesJson = null;
}

VKUMessagesListItemProvider::~VKUMessagesListItemProvider() {
	DESTRUCT(L"VKUMessagesListItemProvider");

	VKUApp::GetInstance()->GetService()->UnsubscribeReadEvents();

	if (_messagesJson)
		delete _messagesJson;
}

result VKUMessagesListItemProvider::Construct(int peerId, JsonObject *chatJson, TableView *tableView) {
	_chatJson = chatJson;
	_tableView = tableView;
	_peerId = peerId;

	if (chatJson != null) {
		ProcessChatUsers(chatJson);
	}

	JsonArray *dialogData = static_cast<JsonArray *>(JsonParser::ParseN(VKUApp::GetInstance()->GetCacheDir() + "dialog" + Integer::ToString(_peerId) + ".json"));
	if(GetLastResult() == E_SUCCESS) {
		_messagesJson = dialogData;
	}

	VKUApp::GetInstance()->GetService()->SubscribeReadEvents(this);

	SetLastResult(E_SUCCESS);
	return E_SUCCESS;
}

result VKUMessagesListItemProvider::ProcessChatUsers(const JsonObject * chatJson) {
	AppLog("VKUMessagesListItemProvider::ProcessChatUsers");
	result r = E_SUCCESS;

	JsonArray *chatUsers;
	JsonParseUtils::GetArray(chatJson, L"users", chatUsers);

	_pUserIdAvatarMap = new HashMap(SingleObjectDeleter);
	_pUserIdAvatarMap->Construct(chatUsers->GetCount(), 1);

	for (int i=0; i<chatUsers->GetCount(); i++) {
		JsonObject *userJson;
		JsonParseUtils::GetObject(chatUsers, i, userJson);

		int userId;
		String avatarUrl;

		JsonParseUtils::GetInteger(*userJson, L"id", userId);
		JsonParseUtils::GetString(*userJson, L"photo_100", avatarUrl);

		AppLog("Adding avatar for %d : %ls", userId, avatarUrl.GetPointer());
		_pUserIdAvatarMap->Add(new Integer(userId), new String(avatarUrl));
	}

	return r;
}

// IListViewItemProvider
int VKUMessagesListItemProvider::GetItemCount() {
	AppLog("GetItemCount call");
	if (_messagesJson == null)
		return 0;

	return _messagesJson->GetCount();
}

TableViewItem* VKUMessagesListItemProvider::CreateItem(int index, int itemWidth) {
	result r;
	AppLog("VKUMessagesListItemProvider::CreateItem");

	RoundedAvatar *pAvatar; // NOTE: used only if chat and message is out==0
	MessageBubble* pMessageBubble;
	RelativeLayout itemLayout;
	Color bgColor;

	JsonObject *itemObject;
	IJsonValue *itemValue;
	TableViewItem* pItem;
	JsonNumber outNumber;

	ArrayList *pMessageElements;

	Label *pTimeStamp;
	String timespampText;
	int timestampValue;

	String messageText(L"no text????");
	int out = 0, readState = 0;

	// reverse list
	int reversedIndex = _messagesJson->GetCount() - 1 - index;
	AppLog("Item %d of %d", reversedIndex, GetItemCount());

	// get message string
	r = _messagesJson->GetAt(reversedIndex, itemValue);
	TryCatch(r == E_SUCCESS, , "Failed GetAt");
	itemObject = static_cast<JsonObject *>(itemValue);

	JsonParseUtils::GetInteger(*itemObject, L"out", out);
	JsonParseUtils::GetInteger(*itemObject, L"date", timestampValue);
	JsonParseUtils::GetInteger(*itemObject, L"read_state", readState);

	TimeUtils::GetDialogsTime(timestampValue, timespampText);

	// create rich text panel
	AppLog("Message is %d == out", out);
	pMessageBubble = new MessageBubble();
	r = pMessageBubble->Construct(Dimension(itemWidth, LIST_HEIGHT));
	TryCatch(r == E_SUCCESS, , "Failed Construct RichTextPanel");

	pMessageBubble->SetOut(out);
	AppLog("RTPanel created and constructed");
	itemLayout.Construct();

	// get available elements
	pMessageElements = GetMessageElementsN(itemObject, itemWidth);

	// message text element
	for (int i=0; i<pMessageElements->GetCount(); i++) {
		AppLog("Adding element %d to pItem", i);
		MessageElement *pElement = static_cast<MessageElement *>(pMessageElements->GetAt(i));
		pMessageBubble->AddElement(pElement);
		AppLog("Added element %d to pItem with size of %dx%d", i, pElement->GetWidth(), pElement->GetHeight());
	}

	// timestamp label
	pTimeStamp = new Label();
	pTimeStamp->Construct(Rectangle(0, 0, 100, 28), timespampText);
	pTimeStamp->SetTextConfig(28, LABEL_TEXT_STYLE_NORMAL);
	pTimeStamp->SetTextColor(Color(TIMESTAMP_TEXT_COLOR, false));

	// create table item
	pItem = new TableViewItem();
	r = pItem->Construct(itemLayout, Dimension(itemWidth, pMessageBubble->GetHeight() + 2*BUBBLE_VERTICAL_MARGIN));
	TryCatch(r == E_SUCCESS, , "Failed GetAt");

	if (out == 0 && _peerId > 2000000000) {
		int fromId;
		JsonParseUtils::GetInteger(*itemObject, L"from_id", fromId);
		AppLog("Finding avatar for %d", fromId);

		pAvatar = new RoundedAvatar(AVATAR_NORMAL);
		String * avatarUrl = static_cast<String *>(_pUserIdAvatarMap->GetValue(Integer(fromId)));

		pAvatar->Construct(Rectangle(0, 0, 80, 80), *avatarUrl);
		r = pItem->AddControl(pAvatar);
		itemLayout.SetRelation(*pAvatar, pItem, RECT_EDGE_RELATION_LEFT_TO_LEFT);
		itemLayout.SetRelation(*pAvatar, pItem, RECT_EDGE_RELATION_TOP_TO_TOP);
		itemLayout.SetMargin(*pAvatar, 10, 0, 10, 0);
	}

	// add rich text panel to table item
	r = pItem->AddControl(pMessageBubble);
	TryCatch(r == E_SUCCESS, , "Failed AddControl");
	r = pItem->AddControl(pTimeStamp);

	itemLayout.SetCenterAligned(*pMessageBubble, CENTER_ALIGN_VERTICAL);
	itemLayout.SetHorizontalFitPolicy(*pTimeStamp, FIT_POLICY_CONTENT);

	if (out == 1) {
		itemLayout.SetRelation(*pMessageBubble, *pItem, RECT_EDGE_RELATION_RIGHT_TO_RIGHT);
		itemLayout.SetMargin(*pMessageBubble, 0, 10, 0, 0);

		itemLayout.SetRelation(*pTimeStamp, *pMessageBubble, RECT_EDGE_RELATION_RIGHT_TO_LEFT);
		itemLayout.SetRelation(*pTimeStamp, *pItem, RECT_EDGE_RELATION_BOTTOM_TO_BOTTOM);
		itemLayout.SetMargin(*pTimeStamp, 0, 10, 0, 30);
	} else {
		if (_peerId > 2000000000) {
			itemLayout.SetRelation(*pMessageBubble, pAvatar, RECT_EDGE_RELATION_LEFT_TO_RIGHT);
			itemLayout.SetMargin(*pMessageBubble, 10, 0, 0, 0);
		} else {
			itemLayout.SetRelation(*pMessageBubble, *pItem, RECT_EDGE_RELATION_LEFT_TO_LEFT);
			itemLayout.SetMargin(*pMessageBubble, 10, 0, 0, 0);
		}

		itemLayout.SetRelation(*pTimeStamp, *pMessageBubble, RECT_EDGE_RELATION_LEFT_TO_RIGHT);
		itemLayout.SetRelation(*pTimeStamp, *pItem, RECT_EDGE_RELATION_BOTTOM_TO_BOTTOM);
		itemLayout.SetMargin(*pTimeStamp, 10, 0, 0, 30);
	}

	// colors
	if (out == 1 && readState == 0) {
		bgColor = Color(LIST_ITEM_UNREAD_COLOR, false);
	} else {
		bgColor = Color::GetColor(COLOR_ID_BLACK);
	}

	pItem->SetBackgroundColor(bgColor, TABLE_VIEW_ITEM_DRAWING_STATUS_NORMAL);
	pItem->SetBackgroundColor(bgColor, TABLE_VIEW_ITEM_DRAWING_STATUS_PRESSED);
	pItem->SetBackgroundColor(bgColor, TABLE_VIEW_ITEM_DRAWING_STATUS_HIGHLIGHTED);

	pItem->RequestRedraw(true);

	AppLog("Returning item");
	return pItem;

CATCH:
	AppLogException("$${Function:CreateItem} is failed. %s", GetErrorMessage(r));
	return null;
}


bool VKUMessagesListItemProvider::DeleteItem(int index, TableViewItem* pItem) {
	delete pItem;
	return true;
}

void VKUMessagesListItemProvider::UpdateItem(int itemIndex, TableViewItem* pItem) {
	result r = E_SUCCESS;

//	RichTextPanel * pRtPanel = new RichTextPanel();
//
//	int reversedIndex = GetItemCount()-1 - itemIndex;
//	IJsonValue *itemValue;
//	r = messagesJson->GetAt(reversedIndex, itemValue);
//	TryCatch(r == E_SUCCESS, , "Failed GetAt");
//
//	JsonObject *itemObject = static_cast<JsonObject *>(itemValue);
//
//	String messageText(L"no text????");
//
//	IJsonValue *bodyValue;
//	static const String bodyConst(L"body");
//	if (itemObject->GetValue(&bodyConst, bodyValue) == E_SUCCESS) {
//		if (bodyValue->GetType() == JSON_TYPE_STRING) {
//			messageText = *static_cast<JsonString *>(bodyValue);
//		}
//	}
//
//	r = pRtPanel->Construct(Dimension(200, 200), Construct);
//	TryCatch(r == E_SUCCESS, , "Failed Construct RichTextPanel");
//
//	r = pItem->AddControl(pRtPanel);
//	TryCatch(r == E_SUCCESS, , "Failed AddControl");

	SetLastResult(r);
	return;

//CATCH:
//	AppLogException("$${Function:UpdateItem} is failed.", GetErrorMessage(r));
//	SetLastResult(r);
}

int VKUMessagesListItemProvider::GetDefaultItemHeight() {
	return LIST_HEIGHT;
}

void VKUMessagesListItemProvider::OnTableViewItemStateChanged(
		Tizen::Ui::Controls::TableView& tableView, int itemIndex,
		Tizen::Ui::Controls::TableViewItem* pItem,
		Tizen::Ui::Controls::TableViewItemStatus status) {

	switch(status) {
	case TABLE_VIEW_ITEM_STATUS_SELECTED:
		AppLog("TABLE_VIEW_ITEM_STATUS_HIGHLIGHTED");
		break;
	case TABLE_VIEW_ITEM_STATUS_HIGHLIGHTED:
		AppLog("TABLE_VIEW_ITEM_STATUS_HIGHLIGHTED");
		break;
	case TABLE_VIEW_ITEM_STATUS_CHECKED:
		AppLog("TABLE_VIEW_ITEM_STATUS_CHECKED");
		break;
	case TABLE_VIEW_ITEM_STATUS_UNCHECKED:
		AppLog("TABLE_VIEW_ITEM_STATUS_UNCHECKED");
		break;
	case TABLE_VIEW_ITEM_STATUS_MORE:
		AppLog("TABLE_VIEW_ITEM_STATUS_MORE");
		break;
	}
}


ArrayList * VKUMessagesListItemProvider::GetMessageElementsN(const JsonObject *pMessageJson, int itemWidth) {
	AppLog("enter VKUMessagesListItemProvider::GetMessageElementsN");
	result r;

	// general
	ArrayList* pResultArray;

	// body stuff
	String messageText;
	MessageTextElement *pMessageTextElement;

	// attachs stuff
	IJsonValue *attachs;
	JsonArray * pAttachArray;
	int out;
	int emoji = 0;

	pResultArray = new ArrayList(SingleObjectDeleter);
	r = pResultArray->Construct(1);
	TryCatch(r == E_SUCCESS, , "pResultArray->Construct");
	JsonParseUtils::GetInteger(*pMessageJson, L"out", out);

	JsonObject * geoObject;
	r = JsonParseUtils::GetObject(pMessageJson, L"geo", geoObject);

	if (r == E_SUCCESS) {
		AppLog("Message has geo entry, receiving");
		MessageLocationElement * pLocationElement = new MessageLocationElement();
		pLocationElement->Construct(Rectangle(0, 0, 400, 400), geoObject);

		pResultArray->Add(pLocationElement);
	}

	r = JsonParseUtils::GetString(*pMessageJson, L"body", messageText);
	TryCatch(r == E_SUCCESS, , "JsonParseUtils::GetString body");

	JsonParseUtils::GetInteger(*pMessageJson, L"emoji", emoji);

	if (messageText.GetLength() != 0) {
		AppLog("Message has text entry, receiving");
		pMessageTextElement = new MessageTextElement();
		pMessageTextElement->Construct(Rectangle(0, 0, itemWidth-200, 10000));
		pMessageTextElement->SetText(messageText, emoji);

		pResultArray->Add(pMessageTextElement);
	}


	static const String attachConst(L"attachments");
	r = pMessageJson->GetValue(&attachConst, attachs);

	if (r == E_SUCCESS)
		pAttachArray = static_cast<JsonArray *>(attachs);

	for (int i=0; r == E_SUCCESS && i<pAttachArray->GetCount(); i++) {
		AppLog("Message has %d attachments, receiving %d", pAttachArray->GetCount(), i);

		IJsonValue *pAttachValue;
		JsonObject *pAttachObject;
		String attachType;

		MessageElement *pMessageElement;

		pAttachArray->GetAt(i, pAttachValue);

		pAttachObject = static_cast<JsonObject *>(pAttachValue);

		JsonParseUtils::GetString(*pAttachObject, L"type", attachType);

		if (attachType == L"photo") {
			AppLog("Message has photo, receiving");
			String imageUrl;

			IJsonValue *pPhotoValue;
			JsonObject *pPhotoObject;

			Rectangle thumbSize;

			int width = 0, height = 0;

			static const String photoConst(L"photo");

			pAttachObject->GetValue(&photoConst, pPhotoValue);
			pPhotoObject = static_cast<JsonObject *>(pPhotoValue);

			JsonParseUtils::GetString(*pPhotoObject, L"photo_604", imageUrl);
			JsonParseUtils::GetInteger(*pPhotoObject, L"width", width);
			JsonParseUtils::GetInteger(*pPhotoObject, L"height", height);

			if (width != 0 && height != 0) {
				thumbSize = ImageUtils::ScaleTo(320, Rectangle(0, 0, width, height));
			} else {
				thumbSize = Rectangle(0, 0, 320, 240);
			}

			MessagePhotoElement * pPhotoElement = new MessagePhotoElement();
			pPhotoElement->Construct(thumbSize, imageUrl);

			pMessageElement = static_cast<MessageElement *>(pPhotoElement);;

		} else if (attachType == L"video") {
			AppLog("Message has video, receiving");

//			IJsonValue *pVideoValue;
//			JsonObject *pVideoObject;
//
//			static const String videoConst(L"video");
//
//			pAttachObject->GetValue(&videoConst, pVideoValue);
//			pVideoObject = static_cast<JsonObject *>(pVideoValue);

			JsonObject *pVideoObject;
			JsonParseUtils::GetObject(pAttachObject, L"video", pVideoObject);

			MessageVideoElement *pVideoElement = new MessageVideoElement();
			pVideoElement->Construct(Rectangle(0, 0, 320, 240), pVideoObject);

			pMessageElement = static_cast<MessageElement *>(pVideoElement);

		} else if (attachType == L"audio") {
			AppLog("Message has audio, receiving");

			JsonObject *pAudioObject;
			JsonParseUtils::GetObject(pAttachObject, L"audio", pAudioObject);

			MessageAudioElement *pMessageAudioEleemnt = new MessageAudioElement();
			pMessageAudioEleemnt->Construct(Rectangle(0, 0, 520, 240), pAudioObject, out);

			pMessageElement = dynamic_cast<MessageElement *>(pMessageAudioEleemnt);

		} else if (attachType == L"doc") {
			AppLog("Message has doc, receiving");

			JsonObject *pDocObject;
			JsonParseUtils::GetObject(pAttachObject, L"doc", pDocObject);

			MessageDocElement * pDocElement = new MessageDocElement();
			pDocElement->Construct(Rectangle(0, 0, 520, 90), pDocObject, out);

			pMessageElement = static_cast<MessageElement * >(pDocElement);
		} else if (attachType == L"wall") {
			AppLog("Message has wall, receiving");

			JsonObject *pWallObject;
			JsonParseUtils::GetObject(pAttachObject, L"wall", pWallObject);

			MessageWallElement * pWallElement = new MessageWallElement();
			pWallElement->Construct(Rectangle(0, 0, 320, 240), pWallObject, out);

			pMessageElement = dynamic_cast<MessageElement *>(pWallElement);
		}

		pResultArray->Add(pMessageElement);
	}

	JsonArray *forwardedMessages;
	r = JsonParseUtils::GetArray(pMessageJson, L"fwd_messages", forwardedMessages);
	AppLog("Message has forwardedMessages?");
	for (int i=0; r == E_SUCCESS && i<forwardedMessages->GetCount(); i++) {
		AppLog("Message has forwardedMessages, receiving %d", i);

		JsonObject *fwdMessage;

		JsonParseUtils::GetObject(forwardedMessages, i, fwdMessage);

		MessageForwardedElement *pForwardedElement = new MessageForwardedElement();
		pForwardedElement->Construct(Rectangle(0, 0, 500, 40000), fwdMessage, out);

		MessageElement *pMessageElement;
		pMessageElement = static_cast<MessageForwardedElement *>(pForwardedElement);

		pResultArray->Add(pMessageElement);
	}

	AppLog("Message has done");

	return pResultArray;

CATCH:
	AppLogException("VKUMessagesListItemProvider::GetMessageElementsN is failed. %s", GetErrorMessage(r));
	return pResultArray;
}

void VKUMessagesListItemProvider::RequestLoadMore(int count) {
	int firstMessageId;
	JsonObject *firstMessage;

	JsonParseUtils::GetObject(_messagesJson, _messagesJson->GetCount() - 1, firstMessage);
	JsonParseUtils::GetInteger(*firstMessage, L"id", firstMessageId);

	VKUApi::GetInstance().CreateRequest("execute.getHistoryAndMarkRead", this)
		->Put(L"count", Integer::ToString(count))
		->Put(L"user_id", Integer::ToString(_peerId))
		->Put(L"start_message_id", Integer::ToString(firstMessageId - 1))
		->Put(L"rev", L"1")
		->Submit(REQUEST_LOAD_MORE);
}

void VKUMessagesListItemProvider::RequestNewMessage(int messageId) {
	VKUApi::GetInstance().CreateRequest("execute.getMessagesAndMarkRead", this)
		->Put(L"message_ids", Integer::ToString(messageId))
		->Submit(REQUEST_GET_NEW_MESSAGE);
	//RequestNewMessages();
}

void VKUMessagesListItemProvider::RequestUpdateHistory() {
	AppLog("request new messages json");
	VKUApi::GetInstance().CreateRequest("execute.getHistoryAndMarkRead", this)
		->Put(L"count", PRELOAD_MESSAGES)
		->Put(L"user_id", Integer::ToString(_peerId))
		->Submit(REQUEST_UPDATE_HISTORY);
}


void VKUMessagesListItemProvider::OnResponseN(RequestId requestId, JsonObject *object) {
	result r = E_SUCCESS;
	String cacheFile(VKUApp::GetInstance()->GetCacheDir() + "dialog" + Integer::ToString(_peerId) + ".json");
	JsonObject *response;
	JsonArray *items;

	AppLog("processing messages json");

	r = JsonParseUtils::GetObject(object, L"response", response);
	TryCatch(r == E_SUCCESS, , "failed get response from object");

	r = JsonParseUtils::GetArray(response, L"items", items);
	TryCatch(r == E_SUCCESS, , "failed get items from response");


	if(requestId == REQUEST_GET_NEW_MESSAGE) {

		if(_messagesJson == null) {
			AppLog("Assigned %d items", items->GetCount());
			_messagesJson = items->CloneN();
		} else {
			AppLog("added %d items", items->GetCount());
			for(int i = 0; i < items->GetCount(); i++) {
				IJsonValue *value;
				items->GetAt(i, value);
				_messagesJson->InsertAt(static_cast<JsonObject *>(value)->CloneN(), 0);
			}
		}

		_tableView->UpdateTableView();
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pTableView->UpdateTableView");

		_tableView->RequestRedraw(true);
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pTableView->RequestRedraw");

		r = _tableView->ScrollToItem(_tableView->GetItemCount() - 1);
		TryCatch(r == E_SUCCESS, , "Failed pTableView->ScrollToItem");

		JsonWriter::Compose(_messagesJson, cacheFile);
	} else if(requestId == REQUEST_UPDATE_HISTORY) {
		_messagesJson = items->CloneN();

		_tableView->UpdateTableView();
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pTableView->UpdateTableView");

		_tableView->RequestRedraw(true);
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pTableView->RequestRedraw");

		r = _tableView->ScrollToItem(_tableView->GetItemCount() - 1);
		TryCatch(r == E_SUCCESS, , "Failed pTableView->ScrollToItem");

		JsonWriter::Compose(_messagesJson, cacheFile);
	} else if(requestId == REQUEST_LOAD_MORE) {
		if(_messagesJson != null) {
			AppLog("added %d items", items->GetCount());
			//_messagesJson->Add(items);

			for(int i = items->GetCount() - 1; i >= 0; i--) {
				IJsonValue *value;
				items->GetAt(i, value);

				_messagesJson->Add(static_cast<JsonObject *>(value)->CloneN());
			}
		}

		_tableView->UpdateTableView();
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pTableView->UpdateTableView");

		_tableView->RequestRedraw(true);
		TryCatch(GetLastResult() == E_SUCCESS, r = GetLastResult() , "Failed pTableView->RequestRedraw");

		r = _tableView->ScrollToItem(items->GetCount());
		TryCatch(r == E_SUCCESS, , "Failed pTableView->ScrollToItem");

		//JsonWriter::Compose(_messagesJson, cacheFile);
	}

	delete object;
	SetLastResult(r);
	return;
CATCH:
	AppLogException("DialogHistoryListener::OnResponseN is failed.", GetErrorMessage(r));
	delete object;
	SetLastResult(r);
	return;
}

void VKUMessagesListItemProvider::OnScrollEndReached(Control& source, ScrollEndEvent type) {
	if(type == SCROLL_END_EVENT_END_TOP) {
		RequestLoadMore(30);
	}
}

void VKUMessagesListItemProvider::OnReadEvent(int messageId) {
	if(_messagesJson != null) {
		for(int i = 0; i < _messagesJson->GetCount(); i++) {
			JsonObject *messageObject;
			int currentMessageId;
			JsonParseUtils::GetObject(_messagesJson, i, messageObject);
			JsonParseUtils::GetInteger(*messageObject, L"id", currentMessageId);
			if(currentMessageId == messageId) {
				messageObject->SetValue(new String(L"read_state"), static_cast<IJsonValue*>(new JsonNumber(1)), true);
				_tableView->UpdateTableView();
				_tableView->RequestRedraw(true);
				break;
			}
		}
	}
}
