/*
 * MessagePhotoElement.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: igorglotov
 */

#include "MessagePhotoElement.h"
#include "SceneRegister.h"
#include "VKU.h"

using namespace Tizen::Graphics;
using namespace Tizen::Ui::Scenes;
using namespace Tizen::Base::Collection;
using namespace Tizen::Base;

MessagePhotoElement::MessagePhotoElement() {
	pImageView = null;
}

MessagePhotoElement::~MessagePhotoElement() {

}

result MessagePhotoElement::Construct(const Tizen::Graphics::Rectangle & rect, Tizen::Base::String & imageUrl) {
	result r = E_SUCCESS;
	r = Panel::Construct(rect);
	r = SetSize(Dimension(rect.width, rect.height));

	url = imageUrl;
	pImageView = new WebImageView();
	r = pImageView->Construct(rect, imageUrl);

	r = AddControl(pImageView);
	SetPropagatedTouchEventListener(this);

	return r;
}

void MessagePhotoElement::SetUrl(Tizen::Base::String & aurl) {
	url = aurl;
}

result MessagePhotoElement::OnDraw() {
	result r = Panel::OnDraw();

	Canvas *pCanvas = GetCanvasN();

	if (pCanvas) {
		pCanvas->SetForegroundColor(Color::GetColor(COLOR_ID_GREEN));
		pCanvas->DrawRectangle(pCanvas->GetBounds());

		delete pCanvas;
	}

	return r;
}

bool MessagePhotoElement::OnTouchPressed(Tizen::Ui::Control& source, const Tizen::Ui::TouchEventInfo& touchEventInfo) {
	return true;
}

bool MessagePhotoElement::OnTouchReleased(Tizen::Ui::Control& source, const Tizen::Ui::TouchEventInfo& touchEventInfo) {
	SceneManager* pSceneManager = SceneManager::GetInstance();

	ArrayList* pList = new (std::nothrow) ArrayList(SingleObjectDeleter);
	String *pUrl = new String(url);

	pList->Construct(1);
	pList->Add(pUrl);

	pSceneManager->GoForward(ForwardSceneTransition(SCENE_GALLERY), pList);

	return true;
}
