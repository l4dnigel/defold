#include <dlib/log.h>
#include "hid.h"
#include "hid_private.h"

#include <string.h>

namespace dmHID
{
    extern const char* KEY_NAMES[MAX_KEY_COUNT];
    extern const char* MOUSE_BUTTON_NAMES[MAX_MOUSE_BUTTON_COUNT];

    NewContextParams::NewContextParams()
    {
        memset(this, 0, sizeof(NewContextParams));
    }

    Context::Context()
    {
        memset(this, 0, sizeof(Context));
    }

    HContext NewContext(const NewContextParams& params)
    {
        HContext context = new Context();
        context->m_IgnoreMouse = params.m_IgnoreMouse;
        context->m_IgnoreKeyboard = params.m_IgnoreKeyboard;
        context->m_IgnoreGamepads = params.m_IgnoreGamepads;
        context->m_IgnoreTouchDevice = params.m_IgnoreTouchDevice;
        context->m_IgnoreAcceleration = params.m_IgnoreAcceleration;
        return context;
    }

    void DeleteContext(HContext context)
    {
        delete context;
    }

    HGamepad GetGamepad(HContext context, uint8_t index)
    {
        if (index < MAX_GAMEPAD_COUNT)
            return &context->m_Gamepads[index];
        else
            return INVALID_GAMEPAD_HANDLE;
    }

    uint32_t GetGamepadButtonCount(HGamepad gamepad)
    {
        return gamepad->m_ButtonCount;
    }

    uint32_t GetGamepadAxisCount(HGamepad gamepad)
    {
        return gamepad->m_AxisCount;
    }

    bool IsKeyboardConnected(HContext context)
    {
        return context->m_KeyboardConnected;
    }

    bool IsMouseConnected(HContext context)
    {
        return context->m_MouseConnected;
    }

    bool IsGamepadConnected(HGamepad gamepad)
    {
        if (gamepad != 0x0)
            return gamepad->m_Connected;
        else
            return false;
    }

    bool IsTouchDeviceConnected(HContext context)
    {
        return context->m_TouchDeviceConnected;
    }

    bool IsAccelerometerConnected(HContext context)
    {
        return context->m_AccelerometerConnected;
    }

    bool GetKeyboardPacket(HContext context, KeyboardPacket* out_packet)
    {
        if (out_packet != 0x0 && context->m_KeyboardConnected)
        {
            *out_packet = context->m_KeyboardPacket;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool GetTextPacket(HContext context, TextPacket* out_packet)
    {
        if (out_packet != 0x0 && context->m_KeyboardConnected)
        {
            *out_packet = context->m_TextPacket;
            context->m_TextPacket.m_Size = 0;
            context->m_TextPacket.m_Text[0] = '\0';
            return true;
        }
        else
        {
            return false;
        }
    }

    bool GetMousePacket(HContext context, MousePacket* out_packet)
    {
        if (out_packet != 0x0 && context->m_MouseConnected)
        {
            *out_packet = context->m_MousePacket;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool GetGamepadPacket(HGamepad gamepad, GamepadPacket* out_packet)
    {
        if (gamepad != 0x0 && out_packet != 0x0 && gamepad->m_Connected)
        {
            *out_packet = gamepad->m_Packet;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool GetTouchDevicePacket(HContext context, TouchDevicePacket* out_packet)
    {
        if (out_packet != 0x0 && context->m_TouchDeviceConnected)
        {
            *out_packet = context->m_TouchDevicePacket;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool GetAccelerationPacket(HContext context, AccelerationPacket* out_packet)
    {
        if (out_packet != 0x0)
        {
            *out_packet = context->m_AccelerationPacket;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool GetKey(KeyboardPacket* packet, Key key)
    {
        if (packet != 0x0)
            return packet->m_Keys[key / 32] & (1 << (key % 32));
        else
            return false;
    }

    void SetKey(HContext context, Key key, bool value)
    {
        if (context != 0x0)
        {
            if (value)
                context->m_KeyboardPacket.m_Keys[key / 32] |= (1 << (key % 32));
            else
                context->m_KeyboardPacket.m_Keys[key / 32] &= ~(1 << (key % 32));
        }
    }

    bool GetMouseButton(MousePacket* packet, MouseButton button)
    {
        if (packet != 0x0)
            return packet->m_Buttons[button / 32] & (1 << (button % 32));
        else
            return false;
    }

    void SetMouseButton(HContext context, MouseButton button, bool value)
    {
        if (context != 0x0)
        {
            if (value)
                context->m_MousePacket.m_Buttons[button / 32] |= (1 << (button % 32));
            else
                context->m_MousePacket.m_Buttons[button / 32] &= ~(1 << (button % 32));
        }
    }

    void SetMousePosition(HContext context, int32_t x, int32_t y)
    {
        if (context != 0x0)
        {
            MousePacket& packet = context->m_MousePacket;
            packet.m_PositionX = x;
            packet.m_PositionY = y;
        }
    }

    void SetMouseWheel(HContext context, int32_t value)
    {
        if (context != 0x0)
        {
            context->m_MousePacket.m_Wheel = value;
        }
    }

    bool GetGamepadButton(GamepadPacket* packet, uint32_t button)
    {
        if (packet != 0x0)
            return packet->m_Buttons[button / 32] & (1 << (button % 32));
        else
            return false;
    }

    void SetGamepadButton(HGamepad gamepad, uint32_t button, bool value)
    {
        if (gamepad != 0x0)
        {
            if (value)
                gamepad->m_Packet.m_Buttons[button / 32] |= (1 << (button % 32));
            else
                gamepad->m_Packet.m_Buttons[button / 32] &= ~(1 << (button % 32));
        }
    }

    void SetGamepadAxis(HGamepad gamepad, uint32_t axis, float value)
    {
        if (gamepad != 0x0)
        {
            gamepad->m_Packet.m_Axis[axis] = value;
        }
    }

    // NOTE: A bit contrived function only used for unit-tests. See AddTouchPosition
    bool GetTouchPosition(TouchDevicePacket* packet, uint32_t touch_index, int32_t* x, int32_t* y)
    {
        if (packet != 0x0
                && x != 0x0
                && y != 0x0)
        {
            if (touch_index < packet->m_TouchCount)
            {
                const Touch& t = packet->m_Touches[touch_index];
                *x = t.m_X;
                *y = t.m_Y;
                return true;
            }
        }
        return false;
    }

    // NOTE: A bit contrived function only used for unit-tests
    // We should perhaps include additional relevant touch-arguments
    void AddTouchPosition(HContext context, int32_t x, int32_t y)
    {
        if (context->m_TouchDeviceConnected)
        {
            TouchDevicePacket& packet = context->m_TouchDevicePacket;
            if (packet.m_TouchCount < MAX_TOUCH_COUNT)
            {
                Touch& t = packet.m_Touches[packet.m_TouchCount++];
                t.m_X = x;
                t.m_Y = y;
            }
        }
    }

    void ClearTouchPositions(HContext context)
    {
        if (context->m_TouchDeviceConnected)
        {
            context->m_TouchDevicePacket.m_TouchCount = 0;
        }
    }

    const char* GetKeyName(Key key)
    {
        return KEY_NAMES[key];
    }

    const char* GetMouseButtonName(MouseButton input)
    {
        return MOUSE_BUTTON_NAMES[input];
    }

    const char* KEY_NAMES[MAX_KEY_COUNT] =
    {
        "KEY_SPACE",
        "KEY_EXCLAIM",
        "KEY_QUOTEDBL",
        "KEY_HASH",
        "KEY_DOLLAR",
        "KEY_AMPERSAND",
        "KEY_QUOTE",
        "KEY_LPAREN",
        "KEY_RPAREN",
        "KEY_ASTERISK",
        "KEY_PLUS",
        "KEY_COMMA",
        "KEY_MINUS",
        "KEY_PERIOD",
        "KEY_SLASH",
        "KEY_0",
        "KEY_1",
        "KEY_2",
        "KEY_3",
        "KEY_4",
        "KEY_5",
        "KEY_6",
        "KEY_7",
        "KEY_8",
        "KEY_9",
        "KEY_COLON",
        "KEY_SEMICOLON",
        "KEY_LESS",
        "KEY_EQUALS",
        "KEY_GREATER",
        "KEY_QUESTION",
        "KEY_AT",
        "KEY_A",
        "KEY_B",
        "KEY_C",
        "KEY_D",
        "KEY_E",
        "KEY_F",
        "KEY_G",
        "KEY_H",
        "KEY_I",
        "KEY_J",
        "KEY_K",
        "KEY_L",
        "KEY_M",
        "KEY_N",
        "KEY_O",
        "KEY_P",
        "KEY_Q",
        "KEY_R",
        "KEY_S",
        "KEY_T",
        "KEY_U",
        "KEY_V",
        "KEY_W",
        "KEY_X",
        "KEY_Y",
        "KEY_Z",
        "KEY_LBRACKET",
        "KEY_BACKSLASH",
        "KEY_RBRACKET",
        "KEY_CARET",
        "KEY_UNDERSCORE",
        "KEY_BACKQUOTE",
        "KEY_LBRACE",
        "KEY_PIPE",
        "KEY_RBRACE",
        "KEY_TILDE",
        "KEY_ESC",
        "KEY_F1",
        "KEY_F2",
        "KEY_F3",
        "KEY_F4",
        "KEY_F5",
        "KEY_F6",
        "KEY_F7",
        "KEY_F8",
        "KEY_F9",
        "KEY_F10",
        "KEY_F11",
        "KEY_F12",
        "KEY_UP",
        "KEY_DOWN",
        "KEY_LEFT",
        "KEY_RIGHT",
        "KEY_LSHIFT",
        "KEY_RSHIFT",
        "KEY_LCTRL",
        "KEY_RCTRL",
        "KEY_LALT",
        "KEY_RALT",
        "KEY_TAB",
        "KEY_ENTER",
        "KEY_BACKSPACE",
        "KEY_INSERT",
        "KEY_DEL",
        "KEY_PAGEUP",
        "KEY_PAGEDOWN",
        "KEY_HOME",
        "KEY_END",
        "KEY_KP_0",
        "KEY_KP_1",
        "KEY_KP_2",
        "KEY_KP_3",
        "KEY_KP_4",
        "KEY_KP_5",
        "KEY_KP_6",
        "KEY_KP_7",
        "KEY_KP_8",
        "KEY_KP_9",
        "KEY_KP_DIVIDE",
        "KEY_KP_MULTIPLY",
        "KEY_KP_SUBTRACT",
        "KEY_KP_ADD",
        "KEY_KP_DECIMAL",
        "KEY_KP_EQUAL",
        "KEY_KP_ENTER"
    };

    const char* MOUSE_BUTTON_NAMES[MAX_MOUSE_BUTTON_COUNT] =
    {
        "MOUSE_BUTTON_LEFT",
        "MOUSE_BUTTON_MIDDLE",
        "MOUSE_BUTTON_RIGHT"
    };
}
