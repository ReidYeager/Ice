
#include "defines.h"
#include "core/input.h"

#include "core/platform/platform.h"

Ice::Input Ice::input;

void Ice::Input::Initialize()
{
  Ice::MemoryZero(&m_states, sizeof(InputStates));
}

void Ice::Input::Shutdown()
{
  
} 

void Ice::Input::Update()
{
  Ice::MemoryCopy(&m_states.keyboardCurrent, &m_states.keyboardPrevious, sizeof(keyboardState));
  Ice::MemoryCopy(&m_states.mouseCurrent, &m_states.mousePrevious, sizeof(mouseState));
  m_states.mouseCurrent.rawDelta = { 0, 0 }; // Raw input does not return 0,0
}

void Ice::Input::ProcessKeyboardKey(KeyCodeFlag _key, b8 _pressed)
{
  m_states.keyboardCurrent.keys[_key] = _pressed;
}

b8 Ice::Input::IsKeyDown(KeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key];
}

b8 Ice::Input::WasKeyDown(KeyCodeFlag _key)
{
  return m_states.keyboardPrevious.keys[_key];
}

b8 Ice::Input::OnKeyPressed(KeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && !m_states.keyboardPrevious.keys[_key];
}

b8 Ice::Input::OnKeyReleased(KeyCodeFlag _key)
{
  return !m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

b8 Ice::Input::OnKeyHold(KeyCodeFlag _key)
{
  return m_states.keyboardCurrent.keys[_key] && m_states.keyboardPrevious.keys[_key];
}

void Ice::Input::ProcessMouseButton(MouseButtonFlag _button, b8 _pressed)
{
  m_states.mouseCurrent.buttons[_button] = _pressed;
}

void Ice::Input::ProcessMouseMove(i32 _deltaX, i32 _deltaY)
{
  //if (m_states.mouseCurrent.x != _x || m_states.mouseCurrent.y != _y)
  //{
  //  m_states.mouseCurrent.x = _x;
  //  m_states.mouseCurrent.y = _y;
  //}
  m_states.mouseCurrent.rawDelta = { _deltaX, _deltaY };
}

void Ice::Input::ProcessMouseWindowPos(i32 _x, i32 _y)
{
  m_states.mouseCurrent.windowPosition = { _x, _y };
}

void Ice::Input::ProcessMouseWheel(i32 _magnitude)
{
  m_states.mouseCurrent.wheel = _magnitude;
}

b8 Ice::Input::IsMouseButtonDown(MouseButtonFlag _button)
{
  return m_states.mouseCurrent.buttons[_button];
}

b8 Ice::Input::OnMouseButtonPressed(MouseButtonFlag _button)
{
  return m_states.mouseCurrent.buttons[Mouse_Left] && !m_states.mousePrevious.buttons[Mouse_Left];
}

b8 Ice::Input::OnMouseButtonReleased(MouseButtonFlag _button)
{
  return m_states.mousePrevious.buttons[Mouse_Left] && !m_states.mouseCurrent.buttons[Mouse_Left];
}

b8 Ice::Input::WasMouseButtonDown(MouseButtonFlag _button)
{
  return m_states.mousePrevious.buttons[_button];
}

Ice::vec2I Ice::Input::GetMousePosition()
{
  return m_states.mouseCurrent.windowPosition;
}

Ice::vec2I Ice::Input::GetMousePreviousPosition()
{
  return m_states.mousePrevious.windowPosition;
}

Ice::vec2I Ice::Input::GetMouseDelta()
{
  return m_states.mouseCurrent.rawDelta;
}

Ice::vec2I Ice::Input::GetMousePreviousDelta()
{
  return m_states.mousePrevious.rawDelta;
}

