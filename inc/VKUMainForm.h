#ifndef _VKU_MAIN_FORM_H_
#define _VKU_MAIN_FORM_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FShell.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>
#include <FWebJson.h>

class VKUMainForm;

#include "IAPIRequestListener.h"

class VKUMainForm :
	public Tizen::Ui::Controls::Form,
	public Tizen::Ui::IActionEventListener,
	public Tizen::Ui::Controls::IFormBackEventListener,
 	public Tizen::Ui::Scenes::ISceneEventListener,
 	public IAPIRequestListener

{
public:
	VKUMainForm(void);
	virtual ~VKUMainForm(void);
	bool Initialize(void);

public:
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
	virtual void OnFormBackRequested(Tizen::Ui::Controls::Form& source);
	virtual void OnActionPerformed(const Tizen::Ui::Control& source, int actionId);

	void UpdateCounters();
protected:
	static const int ID_HEADER_MESSAGES = 101;
	static const int ID_HEADER_CONTACTS = 102;
	static const int ID_HEADER_SEARCH = 103;
	static const int ID_HEADER_SETTINGS = 104;

	virtual void OnSceneActivatedN(const Tizen::Ui::Scenes::SceneId& previousSceneId,
								   const Tizen::Ui::Scenes::SceneId& currentSceneId, Tizen::Base::Collection::IList* pArgs);
	virtual void OnSceneDeactivated(const Tizen::Ui::Scenes::SceneId& currentSceneId,
									const Tizen::Ui::Scenes::SceneId& nextSceneId);


	//void ClearContacts();


	virtual void OnResponseN(RequestId requestId, Tizen::Web::Json::JsonObject *object);
};

#endif	//_VKU_MAIN_FORM_H_
