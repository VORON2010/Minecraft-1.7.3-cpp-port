#pragma once

#include <string>
#include <array>

#include "java/Type.h"
#include "java/File.h"

#include "lwjgl/Keyboard.h"

class Minecraft;

class KeyMapping
{
public:
	jstring name;
	int_t key;

	KeyMapping(const jstring &name, int_t key)
	{
		this->name = name;
		this->key = key;
	}
};

class Options
{
public:
	struct Option
	{
	private:
		Option() = delete;

	public:
		struct Element
		{
			bool isProgress;
			bool isBoolean;
			const char16_t *captionId;
		};

		friend class Options;

	public:
		static Element MUSIC;
		static Element SOUND;
		static Element INVERT_MOUSE;
		static Element SENSITIVITY;
		static Element RENDER_DISTANCE;
		static Element VIEW_BOBBING;
		static Element ANAGLYPH;
		static Element LIMIT_FRAMERATE;
		static Element GUI_SCALE;
		static Element AMBIENT_OCCLUSION;
		static Element ADVANCED_OPENGL;
		static Element DIFFICULTY;
		static Element GRAPHICS;

		static constexpr Element *values[13] = {
			&MUSIC, &SOUND, &INVERT_MOUSE, &SENSITIVITY, &RENDER_DISTANCE, &VIEW_BOBBING, &ANAGLYPH, &LIMIT_FRAMERATE, &GUI_SCALE, &AMBIENT_OCCLUSION, &ADVANCED_OPENGL, &DIFFICULTY, &GRAPHICS
		};
	};

private:
	static const char16_t *RENDER_DISTANCE_NAMES[4];
	static const char16_t *LIMIT_FRAMERATE_NAMES[3];
	static const char16_t *GUI_SCALE_NAMES[4];
	static const char16_t *DIFFICULTY_NAMES[4];

public:
	float music = 1.0f;
	float sound = 1.0f;

	float mouseSensitivity = 0.5f;
	bool invertYMouse = false;

	int_t viewDistance = 0;
	bool bobView = true;
	bool anaglyph3d = false;
	int_t limitFramerate = 1;
	int_t guiScale = 0;
	bool ambientOcclusion = true;
	bool advancedOpengl = false;
	bool fancyGraphics = true;

	jstring skin = u"Default";

	KeyMapping keyUp = KeyMapping(u"key.forward", lwjgl::Keyboard::KEY_W);
	KeyMapping keyLeft = KeyMapping(u"key.left", lwjgl::Keyboard::KEY_A);
	KeyMapping keyDown = KeyMapping(u"key.back", lwjgl::Keyboard::KEY_S);
	KeyMapping keyRight = KeyMapping(u"key.right", lwjgl::Keyboard::KEY_D);
	KeyMapping keyJump = KeyMapping(u"key.jump", lwjgl::Keyboard::KEY_SPACE);
	KeyMapping keyFog = KeyMapping(u"key.fog", lwjgl::Keyboard::KEY_F);
	KeyMapping keySneak = KeyMapping(u"key.sneak", lwjgl::Keyboard::KEY_LSHIFT);
	KeyMapping keySprint = KeyMapping(u"key.sprint", lwjgl::Keyboard::KEY_LCONTROL);
	KeyMapping keyDrop = KeyMapping(u"key.drop", lwjgl::Keyboard::KEY_Q);
	KeyMapping keyInventory = KeyMapping(u"key.inventory", lwjgl::Keyboard::KEY_E);
	KeyMapping keyChat = KeyMapping(u"key.chat", lwjgl::Keyboard::KEY_T);

	std::array<KeyMapping*, 11> keyMappings = { &keyUp, &keyLeft, &keyDown, &keyRight, &keyJump, &keySneak, &keySprint, &keyDrop, &keyInventory, &keyChat, &keyFog };

protected:
	Minecraft &minecraft;

private:
	std::unique_ptr<File> optionsFile;

public:
	int_t difficulty = 2;

	bool hideGui = false;
	bool thirdPersonView = false;
	bool showDebugInfo = false;
	bool smoothCamera = false;

	jstring username = u"";
	jstring lastMpIp = u"";

	Options(Minecraft &minecraft);
	void open(File *optionsFile);
	void setUsername(const jstring &username);

	jstring getKeyDescription(int_t i);
	jstring getKeyMessage(int_t i);

	void setKey(int_t i, int_t key);
	void set(Option::Element &option, float value);
	void toggle(Option::Element &option, int_t add);

	float getProgressValue(Option::Element &option);
	bool getBooleanValue(Option::Element &option);

	jstring getMessage(Option::Element &option);

	void load();

private:
	float readFloat(const std::string &s);

public:
	void save();
};
