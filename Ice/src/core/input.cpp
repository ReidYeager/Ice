
#include "defines.h"
#include "core/input.h"

#include "platform/platform.h"

IceInput Input;

void IceInput::Initialize()
{
  Ice::MemoryZero(&m_states, sizeof(InputStates));
}

void IceInput::Shutdown()
{
  
}

void IceInput::Update()
{
  Ice::MemoryCopy(&m_states.keyboardCurrent, &m_states.keyboardPrevious, sizeof(keyboardState));
  Ice::MemoryCopy(&m_states.mouseCurrent, &m_states.mousePrevious, sizeof(mouseState));
}

void IceInput::ProcessKeyboardKey(IceKeyCodeFlag _key, b8 _pressed)
{
  m_states.keyboardCurrent.keys[_key] = _pressed;
}

b8 IceInput::IsKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key];
}

b8 IceInput::WasKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardPrevious.keys[_key];
}

b8 IceInput::OnKeyPressed(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && !m_states.keyboardPrevious.keys[_key];
}

b8 IceInput::OnKeyReleased(IceKeyCodeFlag _key)
{
  return !m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

b8 IceInput::OnKeyHold(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

void IceInput::ProcessMouseButton(IceMouseButtonFlag _button, b8 _pressed)
{
  m_states.mouseCurrent.buttons[_button] = _pressed;
}

void IceInput::ProcessMouseMove(i32 _x, i32 _y)
{
  if (m_states.mouseCurrent.x != _x || m_states.mouseCurrent.y != _y)
  {
    m_states.mouseCurrent.x = _x;
    m_states.mouseCurrent.y = _y;
  }
}

void IceInput::ProcessMouseWheel(i32 _magnitude)
{
  m_states.mouseCurrent.wheel = _magnitude;
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

