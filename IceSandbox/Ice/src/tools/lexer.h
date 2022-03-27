
#ifndef ICE_TOOLS_LEXER_H_
#define ICE_TOOLS_LEXER_H_

#include "defines.h"

#include <string>

#define IT(x) Ice_Token_##x

namespace Ice {
  enum TokenTypes
  {
    Token_String,
    Token_Int,
    Token_Float,

    Token_Hyphen,
    Token_Comma,
    Token_LeftBracket,
    Token_RightBracket,
    Token_LeftBrace,
    Token_RightBrace,
    Token_LeftParen,
    Token_RightParen,
    Token_FwdSlash,

    Token_End,
    Token_Unknown,
    Token_LessThan,
    Token_GreaterThan,
    Token_Equal,
    Token_Plus,
    Token_Star,
    Token_BackSlash,
    Token_Pound,
    Token_Period,
    Token_SemiColon,
    Token_Colon,
    Token_Apostrophe,
    Token_Quote,
    Token_Pipe
  };

  struct LexerToken
  {
    Ice::TokenTypes type{};
    std::string string{};
  };

  class Lexer
  {
  private:
    const char* charStream = nullptr;

  public:
    Lexer(const char* _stream) : charStream(_stream) {};
    Ice::LexerToken NextToken();
    b8 ExpectToken(const char* _expected);
    b8 CheckForExpectedToken(const char* _expected);

    u32 UintFromToken(const Ice::LexerToken* _token)
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
    Ice::LexerToken GetSingleCharToken(Ice::TokenTypes _type)
    {
      return { _type, std::string(std::string_view(charStream++, 1)) };
    }

    Ice::LexerToken GetStringToken();
    Ice::LexerToken GetNumberToken();
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
} // namespace Ice

Ice::LexerToken Ice::Lexer::NextToken()
{
  // Skip whitespace =====
  while(Ice::isWhiteSpace(*charStream))
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
  case '-': return GetSingleCharToken(Token_Hyphen);
  case ',': return GetSingleCharToken(Token_Comma);
  case '[': return GetSingleCharToken(Token_LeftBracket);
  case ']': return GetSingleCharToken(Token_RightBracket);
  case '{': return GetSingleCharToken(Token_LeftBrace);
  case '}': return GetSingleCharToken(Token_RightBrace);
  case '(': return GetSingleCharToken(Token_LeftParen);
  case ')': return GetSingleCharToken(Token_RightParen);
  case '/': return GetSingleCharToken(Token_FwdSlash);

  case '<': return GetSingleCharToken(Token_LessThan);
  case '>': return GetSingleCharToken(Token_GreaterThan);
  case '=': return GetSingleCharToken(Token_Equal);
  case '+': return GetSingleCharToken(Token_Plus);
  case '*': return GetSingleCharToken(Token_Star);
  case '\\': return GetSingleCharToken(Token_BackSlash);
  case '#': return GetSingleCharToken(Token_Pound);
  case '.': return GetSingleCharToken(Token_Period);
  case ';': return GetSingleCharToken(Token_SemiColon);
  case ':': return GetSingleCharToken(Token_Colon);
  case '\'': return GetSingleCharToken(Token_Apostrophe);
  case '"': return GetSingleCharToken(Token_Quote);
  case '|': return GetSingleCharToken(Token_Pipe);

  case '\0': return GetSingleCharToken(Token_End);
  default: return GetSingleCharToken(Token_Unknown);
  }
}

b8 Ice::Lexer::ExpectToken(const char* _expected)
{
  return NextToken().string.compare(_expected) == 0;
}

b8 Ice::Lexer::CheckForExpectedToken(const char* _expected)
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

Ice::LexerToken Ice::Lexer::GetStringToken()
{
  const char* stringBegining = charStream++;
  u32 stringLength = 1;

  while (Ice::isString(*charStream))
  {
    charStream++;
    stringLength++;
  }

  return { Token_String, std::string(std::string_view(stringBegining, stringLength)) };
}

Ice::LexerToken Ice::Lexer::GetNumberToken()
{
  const char* stringBegining = charStream++;
  u32 stringLength = 1;

  Ice::TokenTypes type = Ice::Token_Int;

  while (Ice::isNumber(*charStream))
  {
    if (*charStream == '.')
    {
      type = Token_Float;
    }

    charStream++;
    stringLength++;
  }

  return { type, std::string(std::string_view(stringBegining, stringLength)) };
}

#undef IT

#endif // !define ICE_TOOLS_LEXER_H_
