/*
 * BitmapLoader.h
 *
 *  Created on: Nov 10, 2013
 *      Author: wolong
 */

#ifndef BITMAPLOADER_H_
#define BITMAPLOADER_H_

#include <FBase.h>
#include <FUi.h>

class BitmapLoader;

#include "VKU.h"
#include "BitmapCache.h"


class ImageLoadingOptions : public Tizen::Base::Object {
public:
	ImageLoadingOptions();
};

class BitmapLoader : public Tizen::Base::Runtime::EventDrivenThread {
public:
	BitmapLoader();
	virtual ~BitmapLoader();

	result Construct();
	result ImageLoaderRequest(ICacheEntry *cacheEntry);

	virtual bool OnStart(void);
	virtual void OnStop(void);
	virtual void OnUserEventReceivedN (RequestId requestId, Tizen::Base::Collection::IList *pArgs);
};

#endif /* BITMAPLOADER_H_ */
