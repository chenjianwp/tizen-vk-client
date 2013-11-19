/*
 * MessageWallElement.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: igorglotov
 */

#include "MessageWallElement.h"

using namespace Tizen::Graphics;

MessageWallElement::MessageWallElement() {
	// TODO Auto-generated constructor stub

}

MessageWallElement::~MessageWallElement() {
	// TODO Auto-generated destructor stub
}

result MessageWallElement::Construct(const Tizen::Graphics::Rectangle & rect) {
	result r = E_SUCCESS;
	r = Panel::Construct(rect);
	r = SetSize(Dimension(rect.width, rect.height));

	return r;
}

result MessageWallElement::OnDraw() {
	Canvas *pCanvas = GetCanvasN();

	if (pCanvas) {
		pCanvas->SetForegroundColor(Color::GetColor(COLOR_ID_RED));
		pCanvas->DrawRectangle(pCanvas->GetBounds());

		delete pCanvas;
	}

	return E_SUCCESS;
}