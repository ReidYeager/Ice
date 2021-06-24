
#include "core/input.h"
#include "core/memory_manager.h"
#include "logger.h"

IceInput Input;

void IceInput::Initialize()
{
  MemoryManager.Zero(&m_states, sizeof(InputStates));
  IcePrint("Initialized Input system");
}

void IceInput::Shutdown()
{
  IcePrint("Shutdown Input system");
}

void IceInput::Update()
{
  MemoryManager.Copy(&m_states.keyboardPrevious, &m_states.keyboardCurrent, sizeof(keyboardState));
  MemoryManager.Copy(&m_states.mousePrevious, &m_states.mouseCurrent, sizeof(mouseState));
}

// TODO : Add event system callbacks in Process* functions

void IceInput::ProcessKeyboardKey(IceKeyCodeFlag _key, b8 _pressed)
{
  m_states.keyboardCurrent.keys[_key] = _pressed;
  IcePrint("%c (%u) %s", (char)_key, _key, (_pressed? "pressed" : "released"));
}

b8 IceInput::IsKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key];
}

b8 IceInput::WasKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardPrevious.keys[_key];
}

void IceInput::ProcessMouseButton(IceMouseButtonFlag _button, b8 _pressed)
{
  m_states.mouseCurrent.buttons[_button] = _pressed;
  IcePrint("Mouse %u %s", _button, (_pressed ? "pressed" : "released"));
}

void IceInput::ProcessMouseMove(i32 _x, i32 _y)
{
  if (m_states.mouseCurrent.x != _x || m_states.mouseCurrent.y != _y)
  {
    m_states.mouseCurrent.x = _x;
    m_states.mouseCurrent.y = _y;

    i32 x, y;
    GetMouseDelta(&x, &y);
    IcePrint("Mouse (%i, %i) -- Δ(%i, %i)", m_states.mouseCurrent.x, m_states.mouseCurrent.y, x, y);
    
  }
}

b8 IceInput::IsMouseButtonDown(IceMouseButtonFlag _button)
{
  return m_states.mouseCurrent.buttons[_button];
}

b8 IceInput::WasMouseButtonDown(IceMouseButtonFlag _button)
{
  return m_states.mousePrevious.buttons[_button];
}

void IceInput::GetMousePosition(i32* _x, i32* _y)
{
  *_x = m_states.mouseCurrent.x;
  *_y = m_states.mouseCurrent.y;
}

void IceInput::GetMousePreviousPosition(i32* _x, i32* _y)
{
  *_x = m_states.mousePrevious.x;
  *_y = m_states.mousePrevious.y;
}

void IceInput::GetMouseDelta(i32* _x, i32* _y)
{
  *_x = m_states.mouseCurrent.x - m_states.mousePrevious.x;
  *_y = m_states.mouseCurrent.y - m_states.mousePrevious.y;
}

