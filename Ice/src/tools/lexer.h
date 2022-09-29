
#ifndef ICE_TOOLS_LEXER_H_
#define ICE_TOOLS_LEXER_H_

#include <string>
#include <vector>

namespace Ice {

enum TokenTypes
{
  Token_End = -1, // End of file
  Token_Unknown = 0,

  Token_String,
  Token_Float,
  Token_Decimal, // Decimal-base int
  Token_Hex,     // Hexadecimal-base int (Can use "0x" tag)

  Token_Hyphen,
  Token_Comma,
  Token_LeftBracket,
  Token_RightBracket,
  Token_LeftBrace,
  Token_RightBrace,
  Token_LeftParen,
  Token_RightParen,
  Token_FwdSlash,
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
  Token_Pipe,

  Token_NullTerminator,
  Token_Whitespace,
};

struct LexerToken
{
  Ice::TokenTypes type{};
  std::string string{};
  const char* start = 0;
};

class Lexer
{
private:
  const char* charStream; // The string to be read from 
  const char* const streamStart; // Used in GetProgress
  const char* const streamEnd; // Used to avoid requiring \0 at the end of the string
  const bool usesHex; // Defines how number identification handles a,b,c,d,e,f,A,B,C,D,E,F

public:
  Lexer(const char* _str, size_t _size, bool _useHex = false) : charStream(_str),
    streamStart(_str),
    streamEnd(_str + _size - 1),
    usesHex(_useHex)
  {}

  Lexer(const std::vector<char>& _str, bool _useHex = false) : charStream(_str.data()),
    streamStart(_str.data()),
    streamEnd(charStream + _str.size() - 1),
    usesHex(_useHex)
  {}

  //=========================
  // Token retrieval
  //=========================

  // Returns a token containing the string up to the next whitespace character
  // _expectHex (Optional) : overrides [a/A - f/F] as numeric values at the start of a token
  // _includeWhitespace (Optional) : Will treat whitespace as tokens when true
  Ice::LexerToken NextToken(bool _expectHex = false, bool _includeWhitespace = false)
  {
    if (!_includeWhitespace)
    {
      SkipWhitespace();
    }

    if (CompletedStream())
    {
      return { Token_End, "", charStream };
    }

    // Get token =====
    switch (*charStream)
    {
    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return GetNumberToken(usesHex);
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    {
      if (_expectHex)
      {
        return GetNumberToken(true);
      }
    }
    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z':
    case '_':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z':
      return GetStringToken();
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

    case '\0': return GetSingleCharToken(Token_NullTerminator);

      // The whitespace skip prevents this from being called when _includeWhitespace is false
    case ' ': case '\n': case '\r': case '\t':
      return GetWhitespaceToken();

    default: return GetSingleCharToken(Token_Unknown);
    }
  }

  // Reads the next token and compares it with the given string
  // Returns true and outputs the read token if they match
  // Returns false if they do not match, Does not move forward in the read string
  // _expected : The string to compare against
  // _outToken (Optional) : Output for the read expected string if found
  bool ExpectString(std::string _expected, Ice::LexerToken* _outToken = nullptr)
  {
    const char* prevCharHead = charStream;

    if (!IsWhiteSpace(_expected.c_str()[0]))
      SkipWhitespace();

    Ice::LexerToken next = Read(_expected.size());

    if (next.string.compare(_expected) == 0)
    {
      if (_outToken != nullptr)
        *_outToken = next;

      return true;
    }

    // Undo token read
    charStream = prevCharHead;
    return false;
  }

  // Reads the next token and compares its type with the given type
  // Returns true and outputs the token if they match
  // Returns false if they do not match, Does not move forward in the read string
  // _expected : The type to compare against
  // _outToken (Optional) : Output for the read string if the types match
  bool ExpectType(Ice::TokenTypes _expected, Ice::LexerToken* _outToken = nullptr)
  {
    const char* prevCharHead = charStream;

    if (_expected != Token_Whitespace)
      SkipWhitespace();

    Ice::LexerToken next = NextToken(_expected == Token_Hex);

    if (next.type == _expected)
    {
      if (_outToken != nullptr)
        *_outToken = next;

      return true;
    }

    // Undo token read
    charStream = prevCharHead;
    return false;
  }

  void SkipWhitespace()
  {
    while (!CompletedStream() && IsWhiteSpace(*charStream))
    {
      charStream++;
    };
  }

  // Creates a string token of a defined length, ignoring the characters' types
  // > Includes whitespace
  // _count : Grabs this many characters as a string regardless of type
  Ice::LexerToken Read(unsigned long long _count)
  {
    // Empty read
    if (_count == 0)
    {
      return { Token_String, std::string(""), charStream };
    }

    // Read =====
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    while (stringLength < _count && !CompletedStream())
    {
      charStream++;
      stringLength++;
    }

    return { Token_String, std::string(stringBeginning, stringLength), stringBeginning };
  }

  // Creates a string token of all characters up to the first instance of the key character
  // > Does not include the key character in the token string
  // > Includes whitespace
  // _key : The character to stop at
  Ice::LexerToken ReadTo(char _key)
  {
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    while (*charStream != _key && !CompletedStream())
    {
      charStream++;
      stringLength++;
    }

    return { Token_String, std::string(stringBeginning, stringLength), stringBeginning };
  }

  // Creates a string token of all characters including the first instance of the key character
  // > Includes whitespace
  // _key : The character to stop at
  Ice::LexerToken ReadThrough(char _key)
  {
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    while (*charStream != _key && !CompletedStream())
    {
      charStream++;
      stringLength++;
    }

    // Include the key character
    charStream++;
    stringLength++;

    return { Token_String, std::string(stringBeginning, stringLength), stringBeginning };
  }

  // Creates a string token of all characters through the first instance of the key string
  // > Includes whitespace
  // _key : The character to stop at
  /*
  Ice::LexerToken ReadThrough(const char* _key)
  {
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    Ice::LexerToken tmpToken;

    while (!CompletedStream())
    {
      ReadTo(_key[0]);

      if (ExpectString(_key, &tmpToken))
      {
        stringLength = (tmpToken.start + tmpToken.string.size()) - stringBeginning;
        break;
      }
      else
      {
        Read(1);
      }

      //charStream++;
      //stringLength++;
    }

    return { Token_String, std::string(stringBeginning, stringLength), stringBeginning };
  }
  */

  // Creates a string token of all characters up to the first instance of any of the key characters
  // > Does not include the key character in the token string
  // > Includes whitespace
  // Returns the index of the found key character, or -1 if none is found
  // _keys : The characters to stop at
  // _outToken (Optional) : The output string token
  unsigned int ReadToFirst(const std::string _keys, Ice::LexerToken* _outToken = nullptr)
  {
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    unsigned int keyFound = 0;
    while (!keyFound && !CompletedStream())
    {
      for (int i = 1; i <= _keys.size() && !keyFound; i++)
      {
        keyFound = i * (*charStream == _keys.c_str()[i - 1]);
      }

      charStream++;
      stringLength++;
    }

    // Prevent an infinite loop when hitting the end of the string
    if (keyFound) // != 0
    {
      // Undo the key read
      charStream--;
      stringLength--;
    }

    *_outToken = { Token_String, std::string(stringBeginning, stringLength), stringBeginning };

    return keyFound - 1;
  }

  // Creates a string token of all characters including the first instance of any of the key characters
  // > Does not include the key character in the token string
  // > Includes whitespace
  // Returns the index of the found key character, or -1 if none is found
  // _keys : The characters to stop at
  // _outToken (Optional) : The output string token
  unsigned int ReadThroughFirst(const std::string _keys, Ice::LexerToken* _outToken = nullptr)
  {
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    unsigned int keyFound = 0;
    while (!keyFound && !CompletedStream())
    {
      for (int i = 1; i <= _keys.size() && !keyFound; i++)
      {
        keyFound = i * (*charStream == _keys.c_str()[i - 1]);
      }

      charStream++;
      stringLength++;
    }

    *_outToken = { Token_String, std::string(stringBeginning, stringLength), stringBeginning };

    return keyFound - 1;
  }

  //=========================
  // Numbers
  //=========================

  // Decimal =====

  // Returns an unsigned int
  unsigned int GetUIntFromToken(const Ice::LexerToken* _token)
  {
    int offset = _token->string[0] == '-'; // Ignores negative sign if present
    return (unsigned int)strtoul(_token->string.c_str() + offset, nullptr, 10);
  }

  // Returns a signed int
  int GetIntFromToken(const Ice::LexerToken* _token)
  {
    return (int)strtol(_token->string.c_str(), nullptr, 10);
  }

  // Returns an unsigned long
  unsigned long GetULongFromToken(const Ice::LexerToken* _token)
  {
    int offset = _token->string[0] == '-'; // Ignores negative sign if present
    return strtoul(_token->string.c_str() + offset, nullptr, 10);
  }

  // Returns a signed long
  long GetLongFromToken(const Ice::LexerToken* _token)
  {
    return strtol(_token->string.c_str(), nullptr, 10);
  }

  // Hex =====

  // Returns an unsigned int
  unsigned int GetUIntFromHexToken(const Ice::LexerToken* _token)
  {
    const std::string& str = _token->string;

    // Ignores negative sign if present
    int offset = str[0] == '-';
    // Ignores "0x" if present
    offset += 2 * (str.size() > (2 + offset) && str[0 + offset] == '0' && str[1 + offset] == 'x');

    return (unsigned int)strtoul(_token->string.c_str() + offset, nullptr, 16);
  }

  // Returns a signed int
  int GetIntFromHexToken(const Ice::LexerToken* _token)
  {
    const std::string& str = _token->string;

    // Ignores negative sign if present
    int offset = str[0] == '-';
    // Ignores "0x" if present
    offset += 2 * (str.size() > (2 + offset) && str[0 + offset] == '0' && str[1 + offset] == 'x');

    return (int)strtol(_token->string.c_str() + offset, nullptr, 16);
  }

  // Returns an unsigned long
  unsigned long GetULongFromHexToken(const Ice::LexerToken* _token)
  {
    const std::string& str = _token->string;

    // Ignores negative sign if present
    int offset = str[0] == '-';
    // Ignores "0x" if present
    offset += 2 * (str.size() > (2 + offset) && str[0 + offset] == '0' && str[1 + offset] == 'x');

    return strtoul(_token->string.c_str() + offset, nullptr, 16);
  }

  // Returns a signed long
  long GetLongFromHexToken(const Ice::LexerToken* _token)
  {
    const std::string& str = _token->string;

    // Ignores negative sign if present
    int offset = str[0] == '-';
    // Ignores "0x" if present
    offset += 2 * (str.size() > (2 + offset) && str[0 + offset] == '0' && str[1 + offset] == 'x');

    return strtol(_token->string.c_str() + offset, nullptr, 16);
  }

  // Binary =====

  // Returns an unsigned int
  unsigned int GetUIntFromBinaryToken(const Ice::LexerToken* _token)
  {
    int offset = _token->string[0] == '-'; // Ignores negative sign if present
    return (unsigned int)strtoul(_token->string.c_str() + offset, nullptr, 2);
  }

  // Returns a signed int
  int GetIntFromBinaryToken(const Ice::LexerToken* _token)
  {
    return (int)strtol(_token->string.c_str(), nullptr, 2);
  }

  // Returns an unsigned long
  unsigned long GetULongFromBinaryToken(const Ice::LexerToken* _token)
  {
    int offset = _token->string[0] == '-'; // Ignores negative sign if present
    return strtoul(_token->string.c_str() + offset, nullptr, 2);
  }

  // Returns a signed long
  long GetLongFromBinaryToken(const Ice::LexerToken* _token)
  {
    return strtol(_token->string.c_str(), nullptr, 2);
  }

  // Float =====

  // Returns a float
  float GetFloatFromToken(const Ice::LexerToken* _token)
  {
    return strtof(_token->string.c_str(), nullptr);
  }

  // Returns a double
  double GetDoubleFromToken(const Ice::LexerToken* _token)
  {
    return strtod(_token->string.c_str(), nullptr);
  }

  //=========================
  // Additional tools
  //=========================

  // Compares the token string with the array of strings
  // Returns [0, _count) as the index of the matching string, _count if no match was found
  unsigned int GetTokenSetIndex(const Ice::LexerToken& _token,
                                const char* const* _stringArray,
                                unsigned int _count,
                                b8 _matchCase = true)
  {
    unsigned int elementIndex, charIndex;
    char tokenChar, testChar;
    b8 charTestResult;

    for (elementIndex = 0; elementIndex < _count; elementIndex++)
    {
      charIndex = 0;
      do
      {
        tokenChar = _token.string[charIndex];
        testChar = _stringArray[elementIndex][charIndex];
        charTestResult = (tokenChar | (!_matchCase * 0x20)) == (testChar | (!_matchCase * 0x20));
        charIndex++;
      } while (charTestResult && tokenChar != '\0' && testChar != '\0');

      // If both are '\0'
      if (tokenChar == testChar)
        return elementIndex;
    }

    return elementIndex;
  }

  // Peek at the next token's string in the stream
  // _count (Optional) : The number of characters to look at (including whitespace)
  // > 0 looks at the next token, skipping whitespace
  std::string Peek(unsigned long long _count = 0)
  {
    if (_count == 0)
    {
      const char* head = charStream;
      Ice::LexerToken token = NextToken();

      charStream = head;
      return token.string;
    }

    // Skip whitespace =====
    // Done to preserve the token start position standard
    while (!CompletedStream() && IsWhiteSpace(*charStream))
    {
      charStream++;
    };

    // Read =====
    const char* stringBeginning = charStream;
    unsigned int stringLength = 0;

    while (stringLength < _count && !CompletedStream())
    {
      charStream++;
      stringLength++;
    }

    return std::string(stringBeginning, stringLength);
  }

  // Returns the percentage (0-1) within the string at which the read head is positioned
  float GetProgress()
  {
    long long length = streamEnd - streamStart;
    long long head = charStream - streamStart;

    return (float)((double)head / (double)length);
  }

  // Returns true if the read head has reached the end of the read string
  bool CompletedStream()
  {
    return charStream > streamEnd;
  }

  //=========================
  // Token generation
  //=========================
private:

  Ice::LexerToken GetSingleCharToken(Ice::TokenTypes _type)
  {
    return { _type, std::string(charStream, 1), charStream++ };
  }

  Ice::LexerToken GetStringToken()
  {
    const char* stringBegining = charStream;
    unsigned int stringLength = 0;

    while (IsString(*charStream) && !CompletedStream())
    {
      charStream++;
      stringLength++;
    }

    return { Token_String, std::string(stringBegining, stringLength), stringBegining };
  }

  Ice::LexerToken GetNumberToken(bool _isHex = false)
  {
    const char* stringBegining = charStream;
    unsigned int stringLength = 0;

    int offset = *stringBegining == '-';

    // Handle hyphen-only
    if (offset && !IsNumber(*charStream))
    {
      charStream--;
      return GetSingleCharToken(Token_Hyphen);
    }

    // Hex test =====

    Ice::TokenTypes type = Ice::Token_Decimal;

    // If hex is guaranteed or the '0x' tag is found
    if (_isHex)
    {
      type = Ice::Token_Hex;
    }
    else if (*(stringBegining + offset) == '0' && *(stringBegining + offset + 1) == 'x')
    {
      type = Ice::Token_Hex;

      // Skip the '0x' tag
      charStream = (stringBegining + offset + 2);
      stringLength += 2;
    }

    // Read number =====

    while (IsNumber(*charStream, type) && !CompletedStream())
    {
      if (*charStream == '.')
      {
        if (type == Ice::Token_Hex)
        {
          break;
        }

        type = Token_Float;
      }

      charStream++;
      stringLength++;
    }

    return { type, std::string(stringBegining, stringLength), stringBegining };
  }

  Ice::LexerToken GetWhitespaceToken()
  {
    const char* stringBeginning = charStream++;
    unsigned int stringLength = 1;

    while (!CompletedStream() && IsWhiteSpace(*charStream))
    {
      charStream++;
      stringLength++;
    }

    return { Token_Whitespace, std::string(stringBeginning, stringLength), stringBeginning };
  }

  Ice::TokenTypes GetCharacterType(char _char, bool _expectHex = false)
  {
    if (CompletedStream())
      return Token_End;

    // Get token =====
    switch (_char)
    {
    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return Token_Decimal;
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    {
      if (_expectHex)
      {
        return Token_Hex;
      }
    }
    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z':
    case '_':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z':
      return Token_String;
    case ',': return Token_Comma;
    case '[': return Token_LeftBracket;
    case ']': return Token_RightBracket;
    case '{': return Token_LeftBrace;
    case '}': return Token_RightBrace;
    case '(': return Token_LeftParen;
    case ')': return Token_RightParen;
    case '/': return Token_FwdSlash;
    case '<': return Token_LessThan;
    case '>': return Token_GreaterThan;
    case '=': return Token_Equal;
    case '+': return Token_Plus;
    case '*': return Token_Star;
    case '\\': return Token_BackSlash;
    case '#': return Token_Pound;
    case '.': return Token_Period;
    case ';': return Token_SemiColon;
    case ':': return Token_Colon;
    case '\'': return Token_Apostrophe;
    case '"': return Token_Quote;
    case '|': return Token_Pipe;

    case '\0': return Token_NullTerminator;

    case ' ': case '\n': case '\r': case '\t':
      return Token_Whitespace;

    default: return Token_Unknown;
    }
  }

  bool IsWhiteSpace(char _char)
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

  bool IsNumber(char _char, Ice::TokenTypes _type = Ice::Token_Decimal)
  {
    if (_type == Ice::Token_Hex)
    {
      switch (_char)
      {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case 'a': case 'A':
      case 'b': case 'B':
      case 'c': case 'C':
      case 'd': case 'D':
      case 'e': case 'E':
      case 'f': case 'F':
        return true;
      default:
        return false;
      }
    }
    else
    {
      switch (_char)
      {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case '.':
        return true;
      default:
        return false;
      }
    }
  }

  bool IsString(char _char)
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
    case '_': case '-':
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


}; // class Lexer

} // namespace Ice

#endif // !define Ice_LEXER_H_
