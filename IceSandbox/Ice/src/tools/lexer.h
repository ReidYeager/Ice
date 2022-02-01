
#ifndef ICE_TOOLS_LEXER_H_
#define ICE_TOOLS_LEXER_H_

#include "defines.h"

#include <string>

#define IT(x) Ice_Token_##x

enum IceTokenTypes
{
  Ice_Token_String,
  Ice_Token_Int,
  Ice_Token_Float,

  Ice_Token_Hyphen,
  Ice_Token_Comma,
  Ice_Token_LeftBracket,
  Ice_Token_RightBracket,
  Ice_Token_LeftBrace,
  Ice_Token_RightBrace,
  Ice_Token_LeftParen,
  Ice_Token_RightParen,
  Ice_Token_FwdSlash,

  Ice_Token_End,
  Ice_Token_Unknown,
  Ice_Token_LessThan,
  Ice_Token_GreaterThan,
  Ice_Token_Equal,
  Ice_Token_Plus,
  Ice_Token_Star,
  Ice_Token_BackSlash,
  Ice_Token_Pound,
  Ice_Token_Period,
  Ice_Token_SemiColon,
  Ice_Token_Colon,
  Ice_Token_Apostrophe,
  Ice_Token_Quote,
  Ice_Token_Pipe
};

struct IceLexerToken
{
  IceTokenTypes type{};
  std::string string{};
};

class IceLexer
{
private:
  const char* charStream = nullptr;

public:
  IceLexer(const char* _stream) : charStream(_stream) {};
  IceLexerToken NextToken();
  b8 ExpectToken(const char* _expected);
  b8 CheckForExpectedToken(const char* _expected);

  u32 UintFromToken(const IceLexerToken* _token)
  {
    u32 value = 0;

    for (char c : _token->string)
    {
      if (c == '.')
      {
        return value;
      }

      value = (value * 10) + (c - '0');
    }

    return value;
  }

private:
  IceLexerToken GetSingleCharToken(IceTokenTypes _type)
  {
    return {_type, std::string(std::string_view(charStream++, 1))};
  }

  IceLexerToken GetTextToken();
  IceLexerToken GetStringToken();
  IceLexerToken GetNumberToken();
};

bool isWhiteSpace(char _char)
{
  switch (_char)
  {
  case ' ':
  case '\n':
  case '\r':
  case '\t':
    return true;
  default:
    return false;
  }
}

bool isNumber(char _char)
{
  switch (_char)
  {
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '0':
  case '.':
    return true;
  default:
    return false;
  }
}

bool isString(char _char)
{
  switch (_char)
  {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
  case 'A': case 'B': case 'C': case 'D': case 'E':
  case 'F': case 'G': case 'H': case 'I': case 'J':
  case 'K': case 'L': case 'M': case 'N': case 'O':
  case 'P': case 'Q': case 'R': case 'S': case 'T':
  case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
  case '_':
  case 'a': case 'b': case 'c': case 'd': case 'e':
  case 'f': case 'g': case 'h': case 'i': case 'j':
  case 'k': case 'l': case 'm': case 'n': case 'o':
  case 'p': case 'q': case 'r': case 's': case 't':
  case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    return true;
  default:
    return false;
  }
}

IceLexerToken IceLexer::NextToken()
{
  // Skip whitespace =====
  while(isWhiteSpace(*charStream))
  {
    charStream++;
  };

  // Get token =====
  switch (*charStream)
  {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return GetNumberToken();
  case 'A': case 'B': case 'C': case 'D': case 'E':
  case 'F': case 'G': case 'H': case 'I': case 'J':
  case 'K': case 'L': case 'M': case 'N': case 'O':
  case 'P': case 'Q': case 'R': case 'S': case 'T':
  case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
  case '_':
  case 'a': case 'b': case 'c': case 'd': case 'e':
  case 'f': case 'g': case 'h': case 'i': case 'j':
  case 'k': case 'l': case 'm': case 'n': case 'o':
  case 'p': case 'q': case 'r': case 's': case 't':
  case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    return GetStringToken();
  case '-': return GetSingleCharToken(Ice_Token_Hyphen);
  case ',': return GetSingleCharToken(Ice_Token_Comma);
  case '[': return GetSingleCharToken(Ice_Token_LeftBracket);
  case ']': return GetSingleCharToken(Ice_Token_RightBracket);
  case '{': return GetSingleCharToken(Ice_Token_LeftBrace);
  case '}': return GetSingleCharToken(Ice_Token_RightBrace);
  case '(': return GetSingleCharToken(Ice_Token_LeftParen);
  case ')': return GetSingleCharToken(Ice_Token_RightParen);
  case '/': return GetSingleCharToken(Ice_Token_FwdSlash);

  case '<': return GetSingleCharToken(Ice_Token_LessThan);
  case '>': return GetSingleCharToken(Ice_Token_GreaterThan);
  case '=': return GetSingleCharToken(Ice_Token_Equal);
  case '+': return GetSingleCharToken(Ice_Token_Plus);
  case '*': return GetSingleCharToken(Ice_Token_Star);
  case '\\': return GetSingleCharToken(Ice_Token_BackSlash);
  case '#': return GetSingleCharToken(Ice_Token_Pound);
  case '.': return GetSingleCharToken(Ice_Token_Period);
  case ';': return GetSingleCharToken(Ice_Token_SemiColon);
  case ':': return GetSingleCharToken(Ice_Token_Colon);
  case '\'': return GetSingleCharToken(Ice_Token_Apostrophe);
  case '"': return GetSingleCharToken(Ice_Token_Quote);
  case '|': return GetSingleCharToken(Ice_Token_Pipe);

  case '\0': return GetSingleCharToken(Ice_Token_End);
  default: return GetSingleCharToken(Ice_Token_Unknown);
  }
}

b8 IceLexer::ExpectToken(const char* _expected)
{
  return NextToken().string.compare(_expected) == 0;
}

b8 IceLexer::CheckForExpectedToken(const char* _expected)
{
  const char* prevCharHead = charStream;

  if (ExpectToken(_expected))
  {
    return true;
  }

  // Undo token read
  charStream = prevCharHead;
  return false;
}

IceLexerToken IceLexer::GetStringToken()
{
  const char* stringBegining = charStream++;
  u32 stringLength = 1;

  while (isString(*charStream))
  {
    charStream++;
    stringLength++;
  }

  return { Ice_Token_String, std::string(std::string_view(stringBegining, stringLength)) };
}

IceLexerToken IceLexer::GetNumberToken()
{
  const char* stringBegining = charStream++;
  u32 stringLength = 1;

  IceTokenTypes type = Ice_Token_Int;

  while (isNumber(*charStream))
  {
    if (*charStream == '.')
    {
      type = Ice_Token_Float;
    }

    charStream++;
    stringLength++;
  }

  return { type, std::string(std::string_view(stringBegining, stringLength)) };
}

#endif // !define ICE_TOOLS_LEXER_H_
