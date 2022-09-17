
#ifndef ICE_CORE_INPUT_H_
#define ICE_CORE_INPUT_H_

#include "defines.h"

namespace Ice {

#define NewKey(name, code) Ice_Key_##name = code
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
typedef Ice::Flag IceKeyCodeFlag;

enum IceMouseButtonFlagBits
{
  Ice_Mouse_Left,
  Ice_Mouse_Right,
  Ice_Mouse_Middle,
  Ice_Mouse_Forward,
  Ice_Mouse_Back,
  Ice_Mouse_Extra,
  Ice_Mouse_Max
};
typedef Ice::Flag IceMouseButtonFlag;
#undef NewButton

// Handles signals from the platform's physical input events
extern class IceInput
{
private:
  struct keyboardState
  {
    b8 keys[256];
  };
  struct mouseState
  {
    i32 x;
    i32 y;
    i32 wheel; // Positive is up-scroll
    b8 buttons[Ice_Mouse_Max];
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
  void ProcessKeyboardKey(IceKeyCodeFlag _key, b8 _pressed);
  b8 IsKeyDown(IceKeyCodeFlag _key);
  b8 WasKeyDown(IceKeyCodeFlag _key);
  b8 OnKeyPressed(IceKeyCodeFlag _key);
  b8 OnKeyReleased(IceKeyCodeFlag _key);
  b8 OnKeyHold(IceKeyCodeFlag _key);

  // Updates the mouse state regarding the inputs
  void ProcessMouseButton(IceMouseButtonFlag _button, b8 _pressed);
  void ProcessMouseMove(i32 _deltaX, i32 _deltaY);
  void ProcessMouseWheel(i32 _magnitude);
  b8 IsMouseButtonDown(IceMouseButtonFlag _button);
  b8 OnMouseButtonPressed(IceMouseButtonFlag _button);
  b8 OnMouseButtonReleased(IceMouseButtonFlag _button);
  b8 WasMouseButtonDown(IceMouseButtonFlag _button);
  void GetMousePosition(i32* _x, i32* _y);
  void GetMousePosition(f32* _x, f32* _y);
  void GetMousePreviousPosition(i32* _x, i32* _y);
  void GetMousePreviousPosition(f32* _x, f32* _y);
  void GetMouseDelta(i32* _x, i32* _y);
  void GetMouseDelta(f32* _x, f32* _y);
} Input;

} // namespace Ice

#endif // !CORE_INPUT_H
