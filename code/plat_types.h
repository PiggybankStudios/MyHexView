/*
File:   plat_types.h
Author: Taylor Robbins
Date:   03\02\2018
*/

#ifndef _PLAT_TYPES_H
#define _PLAT_TYPES_H

// +--------------------------------------------------------------+
// |                        Generic Types                         |
// +--------------------------------------------------------------+
struct Version_t
{
	u32 major;
	u32 minor;
	u32 build;
};

struct FileInfo_t
{
	u32 size;
	void* content;
};

typedef enum
{
	Platform_Windows,
	Platform_Linux,
	Platform_OSX,
} PlatformType_t;

typedef enum : u8
{
	Modifier_Ctrl     = 0x01,
	Modifier_Alt      = 0x02,
	Modifier_Shift    = 0x04,
	Modifier_CapsLock = 0x08,
} ButtonModifier_t;

typedef enum
{
	Cursor_Default,
	Cursor_Text,
	Cursor_Pointer,
	Cursor_ResizeHorizontal,
	Cursor_ResizeVertical,
	
	NumCursorTypes,
} CursorType_t;

// +--------------------------------------------------------------+
// |                   Platform Specific Types                    |
// +--------------------------------------------------------------+
#ifdef WINDOWS_COMPILATION

struct OpenFile_t
{
	bool isOpen;
	HANDLE handle;
};

#elif OSX_COMPILATION

//TODO: Add platform specific types here

#elif LINUX_COMPILATION

//TODO: Add platform specific types here

#endif

// +--------------------------------------------------------------+
// |                      Button Enumeration                      |
// +--------------------------------------------------------------+
typedef enum
{
	//NOTE: We put the MouseButtons at the bottom so we can use them
	//		when referencing AppInput mouseStartPos and mouseMaxDist
	MouseButton_Left = 0,
	MouseButton_Right,
	MouseButton_Middle,
	
	Button_A,
	Button_B,
	Button_C,
	Button_D,
	Button_E,
	Button_F,
	Button_G,
	Button_H,
	Button_I,
	Button_J,
	Button_K,
	Button_L,
	Button_M,
	Button_N,
	Button_O,
	Button_P,
	Button_Q,
	Button_R,
	Button_S,
	Button_T,
	Button_U,
	Button_V,
	Button_W,
	Button_X,
	Button_Y,
	Button_Z,
	
	Button_0,
	Button_1,
	Button_2,
	Button_3,
	Button_4,
	Button_5,
	Button_6,
	Button_7,
	Button_8,
	Button_9,
	
	Button_F1,
	Button_F2,
	Button_F3,
	Button_F4,
	Button_F5,
	Button_F6,
	Button_F7,
	Button_F8,
	Button_F9,
	Button_F10,
	Button_F11,
	Button_F12,
	
	Button_Enter,
	Button_Backspace,
	Button_Escape,
	Button_Insert,
	Button_Delete,
	Button_Home,
	Button_End,
	Button_PageUp,
	Button_PageDown,
	Button_Tab,
	Button_CapsLock,
	
	Button_Control,
	Button_Alt,
	Button_Shift,
	
	Button_Right,
	Button_Left,
	Button_Up,
	Button_Down,
	
	Button_Plus,
	Button_Minus,
	Button_Pipe,
	Button_OpenBracket,
	Button_CloseBracket,
	Button_Colon,
	Button_Quote,
	Button_Comma,
	Button_Period,
	Button_QuestionMark,
	Button_Tilde,
	Button_Space,
	
	Buttons_NumButtons,
} Buttons_t;

inline const char* GetButtonName(Buttons_t button)
{
	switch (button)
	{
		case Button_A:            return "A";
		case Button_B:            return "B";
		case Button_C:            return "C";
		case Button_D:            return "D";
		case Button_E:            return "E";
		case Button_F:            return "F";
		case Button_G:            return "G";
		case Button_H:            return "H";
		case Button_I:            return "I";
		case Button_J:            return "J";
		case Button_K:            return "K";
		case Button_L:            return "L";
		case Button_M:            return "M";
		case Button_N:            return "N";
		case Button_O:            return "O";
		case Button_P:            return "P";
		case Button_Q:            return "Q";
		case Button_R:            return "R";
		case Button_S:            return "S";
		case Button_T:            return "T";
		case Button_U:            return "U";
		case Button_V:            return "V";
		case Button_W:            return "W";
		case Button_X:            return "X";
		case Button_Y:            return "Y";
		case Button_Z:            return "Z";
		case Button_0:            return "0";
		case Button_1:            return "1";
		case Button_2:            return "2";
		case Button_3:            return "3";
		case Button_4:            return "4";
		case Button_5:            return "5";
		case Button_6:            return "6";
		case Button_7:            return "7";
		case Button_8:            return "8";
		case Button_9:            return "9";
		case Button_F1:           return "F1";
		case Button_F2:           return "F2";
		case Button_F3:           return "F3";
		case Button_F4:           return "F4";
		case Button_F5:           return "F5";
		case Button_F6:           return "F6";
		case Button_F7:           return "F7";
		case Button_F8:           return "F8";
		case Button_F9:           return "F9";
		case Button_F10:          return "F10";
		case Button_F11:          return "F11";
		case Button_F12:          return "F12";
		case Button_Enter:        return "Enter";
		case Button_Backspace:    return "Backspace";
		case Button_Escape:       return "Escape";
		case Button_Insert:       return "Insert";
		case Button_Delete:       return "Delete";
		case Button_Home:         return "Home";
		case Button_End:          return "End";
		case Button_PageUp:       return "PageUp";
		case Button_PageDown:     return "PageDown";
		case Button_Tab:          return "Tab";
		case Button_CapsLock:     return "Caps Lock";
		case Button_Control:      return "Control";
		case Button_Alt:          return "Alt";
		case Button_Shift:        return "Shift";
		case Button_Right:        return "Right";
		case Button_Left:         return "Left";
		case Button_Up:           return "Up";
		case Button_Down:         return "Down";
		case Button_Plus:         return "Plus";
		case Button_Minus:        return "Minus";
		case Button_Pipe:         return "Pipe";
		case Button_OpenBracket:  return "Open Bracket";
		case Button_CloseBracket: return "Close Bracket";
		case Button_Colon:        return "Colon";
		case Button_Quote:        return "Quote";
		case Button_Comma:        return "Comma";
		case Button_Period:       return "Period";
		case Button_QuestionMark: return "Question Mark";
		case Button_Tilde:        return "Tilde";
		case Button_Space:        return "Space";
		case MouseButton_Left:    return "Mouse Left";
		case MouseButton_Right:   return "Mouse Right";
		case MouseButton_Middle:  return "Mouse Middle";
		default:                  return "Unknown";
	};
}

bool ButtonIsPolling(Buttons_t button)
{
	switch (button)
	{
		case Button_Shift: return true;
		case Button_Control: return true;
		case Button_Alt: return true;
	};
	return false;
}


#endif //  _PLAT_TYPES_H
