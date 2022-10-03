
#ifndef ICE_CORE_INPUT_H_
#define ICE_CORE_INPUT_H_

#include "defines.h"

#include "math/linear.h"
#include "tools/flag_array.h"

namespace Ice {

#define NewKey(name, code) Key_##name = code
enum IceKeyCodeFlagBits
{
  // Top row numbers
  NewKey(0, 0x30),
  NewKey(1, 0x31),
  NewKey(2, 0x32),
  NewKey(3, 0x33),
  NewKey(4, 0x34),
  NewKey(5, 0x35),
  NewKey(6, 0x36),
  NewKey(7, 0x37),
  NewKey(8, 0x38),
  NewKey(9, 0x39),

  // Numpad numbers
  NewKey(Numpad0, 0x60),
  NewKey(Numpad1, 0x61),
  NewKey(Numpad2, 0x62),
  NewKey(Numpad3, 0x63),
  NewKey(Numpad4, 0x64),
  NewKey(Numpad5, 0x65),
  NewKey(Numpad6, 0x66),
  NewKey(Numpad7, 0x67),
  NewKey(Numpad8, 0x68),
  NewKey(Numpad9, 0x69),

  // Letters
  NewKey(A, 0x41),
  NewKey(B, 0x42),
  NewKey(C, 0x43),
  NewKey(D, 0x44),
  NewKey(E, 0x45),
  NewKey(F, 0x46),
  NewKey(G, 0x47),
  NewKey(H, 0x48),
  NewKey(I, 0x49),
  NewKey(J, 0x4A),
  NewKey(K, 0x4B),
  NewKey(L, 0x4C),
  NewKey(M, 0x4D),
  NewKey(N, 0x4E),
  NewKey(O, 0x4F),
  NewKey(P, 0x50),
  NewKey(Q, 0x51),
  NewKey(R, 0x52),
  NewKey(S, 0x53),
  NewKey(T, 0x54),
  NewKey(U, 0x55),
  NewKey(V, 0x56),
  NewKey(W, 0x57),
  NewKey(X, 0x58),
  NewKey(Y, 0x59),
  NewKey(Z, 0x5A),

  NewKey(Escape, 0x1B)
};
typedef Ice::Flag KeyCodeFlag;

enum IceMouseButtonFlagBits
{
  Mouse_Left,
  Mouse_Right,
  Mouse_Middle,
  Mouse_Back,
  Mouse_Forward,
  Mouse_Extra,
  Mouse_Max
};
typedef Ice::Flag MouseButtonFlag;
#undef NewButton

// Handles signals from the platform's physical input events
extern class Input
{
private:
  struct keyboardState
  {
    b8 keys[256]; // Not a FlagArray for faster reads & easier copy in input.Update
  };
  struct mouseState
  {
    Ice::vec2I windowPosition;
    Ice::vec2I rawDelta;
    i32 wheel; // Positive is up-scroll
    b8 buttons[Mouse_Max];
  };

  struct InputStates
  {
    keyboardState keyboardCurrent;
    keyboardState keyboardPrevious; // Rework to be a signal bit within the current state?
    mouseState mouseCurrent;
    mouseState mousePrevious;
  } m_states;

public:
  // Clears all input data
  void Initialize();
  // Does nothing
  void Shutdown();
  // Moves the current state into the previous state
  void Update();

  // Updates the keyboard state regarding the input keycode
  void ProcessKeyboardKey(KeyCodeFlag _key, b8 _pressed);
  b8 IsKeyDown(KeyCodeFlag _key);
  b8 WasKeyDown(KeyCodeFlag _key);
  b8 OnKeyPressed(KeyCodeFlag _key);
  b8 OnKeyReleased(KeyCodeFlag _key);
  b8 OnKeyHold(KeyCodeFlag _key);

  // Updates the mouse state regarding the inputs
  void ProcessMouseButton(MouseButtonFlag _button, b8 _pressed);
  void ProcessMouseMove(i32 _deltaX, i32 _deltaY);
  void ProcessMouseWindowPos(i32 _x, i32 _y);
  void ProcessMouseWheel(i32 _magnitude);
  b8 IsMouseButtonDown(MouseButtonFlag _button);
  b8 OnMouseButtonPressed(MouseButtonFlag _button);
  b8 OnMouseButtonReleased(MouseButtonFlag _button);
  b8 WasMouseButtonDown(MouseButtonFlag _button);
  Ice::vec2I GetMousePosition();
  //Ice::vec2 GetMousePosition();
  Ice::vec2I GetMousePreviousPosition();
  //Ice::vec2 GetMousePreviousPosition();
  Ice::vec2I GetMouseDelta();
  //Ice::vec2 GetMouseDelta();
  Ice::vec2I GetMousePreviousDelta();
  //Ice::vec2 GetMousePreviousDelta();
} input;

} // namespace Ice

#endif // !CORE_INPUT_H
