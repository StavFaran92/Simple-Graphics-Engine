#include "NativeScriptsRegister.h"

#include "PlayerControllerScript.h"
#include "CameraScript.h"
#include "EnemyController.h"
#include "CursorScript.h"

#include "NativeScriptsLoader.h"

NativeScriptRegisterer::NativeScriptRegisterer()
{
	NativeScriptsLoader::instance->registerScript("PlayerControllerScript", []() { return new PlayerControllerScript(); });
	NativeScriptsLoader::instance->registerScript("CameraScript", []() { return new CameraScript(); });
	NativeScriptsLoader::instance->registerScript("EnemyController", []() { return new EnemyController(); });
	NativeScriptsLoader::instance->registerScript("CursorScript", []() { return new CursorScript(); });
}
