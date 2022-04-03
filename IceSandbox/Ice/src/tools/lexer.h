
#ifndef ICE_TOOLS_LEXER_H_
#define ICE_TOOLS_LEXER_H_

#include "defines.h"

#include <string>

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

    Token_NullTerminator,
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
    Token_Unknown,

    Token_End,
  };

  struct LexerToken
  {
    Ice::TokenTypes type{};
    std::string string{};
  };

  class Lexer
  {
  private:
    const char* charStream;
    const char* const streamEnd; // Used to avoid requiring \0 at the end of a stream

  public:
    Lexer(const char* _stream, u64 _size) : charStream(_stream), streamEnd(_stream + _size - 1)
    {}

    Lexer(const std::vector<char>& _stream) : charStream(_stream.data()),
                                              streamEnd(_stream.data() + _stream.size() - 1)
    {}

    Ice::LexerToken NextToken();
    b8 ExpectString(const char* _expected, Ice::LexerToken* _outToken = nullptr);
    b8 ExpectType(Ice::TokenTypes _expected, Ice::LexerToken* _outToken = nullptr);
    b8 CompletedStream();

    u32 GetUIntFromToken(const Ice::LexerToken* _token);

    u32 GetTokenSetIndex(const Ice::LexerToken& _token, const char* const* _stringArray, u32 _count);

  private:
    Ice::LexerToken GetSingleCharToken(Ice::TokenTypes _type);
    Ice::LexerToken GetStringToken();
    Ice::LexerToken GetNumberToken();
  };
} // namespace Ice

#endif // !define ICE_TOOLS_LEXER_H_
