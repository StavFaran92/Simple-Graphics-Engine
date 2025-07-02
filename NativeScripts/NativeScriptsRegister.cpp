#include "NativeScriptsRegister.h"

#include "PlayerController_Deprecated.h"
#include "CameraScript.h"
#include "EnemyController.h"
#include "CursorScript.h"

#include "NativeScriptsLoader.h"

NativeScriptRegisterer::NativeScriptRegisterer()
{
	NativeScriptsLoader::instance->registerScript("PlayerController_Deprecated", []() { return new PlayerController_Deprecated(); });
	NativeScriptsLoader::instance->registerScript("CameraScript", []() { return new CameraScript(); });
	NativeScriptsLoader::instance->registerScript("EnemyController", []() { return new EnemyController(); });
	NativeScriptsLoader::instance->registerScript("CursorScript", []() { return new CursorScript(); });
}
