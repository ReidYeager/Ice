
#ifndef CORE_INPUT_H
#define CORE_INPUT_H 1

#include "defines.h"

#define NewKey(name, code) ICE_INPUT_KEY##name = code
// TODO : Implement the rest of the input codes (Use Windows' codes)
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
  NewKey(NUMPAD0, 0x60),
  NewKey(NUMPAD1, 0x61),
  NewKey(NUMPAD2, 0x62),
  NewKey(NUMPAD3, 0x63),
  NewKey(NUMPAD4, 0x64),
  NewKey(NUMPAD5, 0x65),
  NewKey(NUMPAD6, 0x66),
  NewKey(NUMPAD7, 0x67),
  NewKey(NUMPAD8, 0x68),
  NewKey(NUMPAD9, 0x69),

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
  NewKey(Z, 0x5A)
};
typedef IceFlag IceKeyCodeFlag;

enum IceMouseButtonFlagBits
{
  ICE_INPUT_MOUSE_LEFT,
  ICE_INPUT_MOUSE_RIGHT,
  ICE_INPUT_MOUSE_MIDDLE,
  ICE_INPUT_MOUSE_FORWARD,
  ICE_INPUT_MOUSE_BACK,
  ICE_INPUT_MOUSE_EXTRA,
  ICE_INPUT_MOUSE_MAX
};
typedef IceFlag IceMouseButtonFlag;
#undef NewButton

class IceInput
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
    u8 buttons[ICE_INPUT_MOUSE_MAX];
  };

  struct InputStates
  {
    keyboardState keyboardCurrent;
    keyboardState keyboardPrevious;
    mouseState mouseCurrent;
    mouseState mousePrevious;
  };

  InputStates m_states;

public:
  void Initialize();
  void Shutdown();
  void Update();

  void ProcessKeyboardKey(IceKeyCodeFlag _key, b8 _pressed);
  b8 IsKeyDown(IceKeyCodeFlag _key);
  b8 WasKeyDown(IceKeyCodeFlag _key);

  void ProcessMouseButton(IceMouseButtonFlag _button, b8 _pressed);
  void ProcessMouseMove(i32 _x, i32 _y);
  // Mouse wheel
  b8 IsMouseButtonDown(IceMouseButtonFlag _button);
  b8 WasMouseButtonDown(IceMouseButtonFlag _button);
  void GetMousePosition(i32* _x, i32* _y);
  void GetMousePreviousPosition(i32* _x, i32* _y);
  void GetMouseDelta(i32* _x, i32* _y);

};

extern IceInput Input;

#endif // !CORE_INPUT_H
