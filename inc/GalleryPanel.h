/*
 * GalleryPanel.h
 *
 *  Created on: Nov 18, 2013
 *      Author: igorglotov
 */

#ifndef GALLERYPANEL_H_
#define GALLERYPANEL_H_

#include <FUi.h>

class GalleryPanel:
	public Tizen::Ui::Controls::Panel,
	public Tizen::Ui::IActionEventListener {
public:
	GalleryPanel();
	virtual ~GalleryPanel();

	bool Initialize();

	// IActionEventListener
	void OnActionPerformed(const Tizen::Ui::Control& source, int actionId);
};

#endif /* GALLERYPANEL_H_ */
