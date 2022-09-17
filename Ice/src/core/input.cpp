
#include "defines.h"
#include "core/input.h"

#include "platform/platform.h"

Ice::IceInput Ice::Input;

void Ice::IceInput::Initialize()
{
  Ice::MemoryZero(&m_states, sizeof(InputStates));
}

void Ice::IceInput::Shutdown()
{
  
} 

void Ice::IceInput::Update()
{
  Ice::MemoryCopy(&m_states.keyboardCurrent, &m_states.keyboardPrevious, sizeof(keyboardState));
  Ice::MemoryCopy(&m_states.mouseCurrent, &m_states.mousePrevious, sizeof(mouseState));
}

void Ice::IceInput::ProcessKeyboardKey(IceKeyCodeFlag _key, b8 _pressed)
{
  m_states.keyboardCurrent.keys[_key] = _pressed;
}

b8 Ice::IceInput::IsKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key];
}

b8 Ice::IceInput::WasKeyDown(IceKeyCodeFlag _key)
{
  return m_states.keyboardPrevious.keys[_key];
}

b8 Ice::IceInput::OnKeyPressed(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && !m_states.keyboardPrevious.keys[_key];
}

b8 Ice::IceInput::OnKeyReleased(IceKeyCodeFlag _key)
{
  return !m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

b8 Ice::IceInput::OnKeyHold(IceKeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

void Ice::IceInput::ProcessMouseButton(IceMouseButtonFlag _button, b8 _pressed)
{
  m_states.mouseCurrent.buttons[_button] = _pressed;
}

void Ice::IceInput::ProcessMouseMove(i32 _deltaX, i32 _deltaY)
{
  //if (m_states.mouseCurrent.x != _x || m_states.mouseCurrent.y != _y)
  //{
  //  m_states.mouseCurrent.x = _x;
  //  m_states.mouseCurrent.y = _y;
  //}
  m_states.mouseCurrent.x += _deltaX;
  m_states.mouseCurrent.y += _deltaY;
}

void Ice::IceInput::ProcessMouseWheel(i32 _magnitude)
{
  m_states.mouseCurrent.wheel = _magnitude;
}

b8 Ice::IceInput::IsMouseButtonDown(IceMouseButtonFlag _button)
{
  return m_states.mouseCurrent.buttons[_button];
}

b8 Ice::IceInput::OnMouseButtonPressed(IceMouseButtonFlag _button)
{
  return m_states.mouseCurrent.buttons[Ice_Mouse_Left] && !m_states.mousePrevious.buttons[Ice_Mouse_Left];
}

b8 Ice::IceInput::OnMouseButtonReleased(IceMouseButtonFlag _button)
{
  return m_states.mousePrevious.buttons[Ice_Mouse_Left] && !m_states.mouseCurrent.buttons[Ice_Mouse_Left];
}

b8 Ice::IceInput::WasMouseButtonDown(IceMouseButtonFlag _button)
{
  return m_states.mousePrevious.buttons[_button];
}

void Ice::IceInput::GetMousePosition(i32* _x, i32* _y)
{
  *_x = m_states.mouseCurrent.x;
  *_y = m_states.mouseCurrent.y;
}

void Ice::IceInput::GetMousePosition(f32* _x, f32* _y)
{
  *_x = (f32)m_states.mouseCurrent.x;
  *_y = (f32)m_states.mouseCurrent.y;
}

void Ice::IceInput::GetMousePreviousPosition(i32* _x, i32* _y)
{
  *_x = m_states.mousePrevious.x;
  *_y = m_states.mousePrevious.y;
}

void Ice::IceInput::GetMousePreviousPosition(f32* _x, f32* _y)
{
  *_x = (f32)m_states.mousePrevious.x;
  *_y = (f32)m_states.mousePrevious.y;
}

void Ice::IceInput::GetMouseDelta(i32* _x, i32* _y)
{
  *_x = m_states.mouseCurrent.x - m_states.mousePrevious.x;
  *_y = m_states.mouseCurrent.y - m_states.mousePrevious.y;
}

void Ice::IceInput::GetMouseDelta(f32* _x, f32* _y)
{
  *_x = (f32)(m_states.mouseCurrent.x - m_states.mousePrevious.x);
  *_y = (f32)(m_states.mouseCurrent.y - m_states.mousePrevious.y);
}

