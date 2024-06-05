#include "Lexer.hpp"

/**
 * @brief Function to return the next token from standard input
 *
 * the variable 'm_IdentifierStr' is set there in case of an identifier,
 * the variable 'm_NumVal' is set there in case of a number.
 */
int Lexer::gettok()
{
    char c = std::cin.peek();
    std::string word = "";
Start:
    if(c == EOF)
    {
        return tok_eof;
    }
    if(std::isspace(c))
    {
        std::cin.get();
        goto Start;
    }
    if(std::isdigit(c))
    {
        goto Decimal;
    }
    if(std::isalpha(c))
    {
        word += std::cin.get();
        goto Word;
    }
    switch (c)
    {
        case '+':
            std::cin.get();
            return '+';
        case '-':
            std::cin.get();
            return '-';
        case '*':
            std::cin.get();
            return '*';
        case '(':
            std::cin.get();
            return '(';
        case '[':
            std::cin.get();
            return '[';
        case ']':
            std::cin.get();
            return ']';
        case '>':
            std::cin.get();
            goto Greater;
        case '<':
            std::cin.get();
            goto LessThan;
        case ':':
            std::cin.get();
            if(std::cin.peek()=='=')
            {
                std::cin.get();
                return tok_assign;
            }
            else
                return ':';
        case ';':
            std::cin.get();
            return ';';      
        case '$':
            std::cin.get();
            goto Octa;
        case '&':  
            std::cin.get();
            goto Hexa;

    }

Decimal:
    return readNumber(10);
Octa:
    return readNumber(8);
Hexa:
    return readNumber(16);
Greater:
    if(std::cin.peek() == '=')
    {
        std::cin.get();
        return tok_greaterequal;
    }
    return '>';
LessThan:
    if(std::cin.peek() == '=')
    {
        std::cin.get();
        return tok_lessequal;
    }
    else if(std::cin.peek() == '>')
    {
        std::cin.get();
        return tok_notequal;
    }
    return '<';
Word:
    if(isalpha(std::cin.peek()))
    {
        word += std::cin.get();
        goto Word;
    }

    if ( word == "array")
        return tok_array;
    else if (word == "downto")
        return tok_downto;
    else if (word == "to")
        return tok_to;
    else if (word == "and")
        return tok_and;
    else if (word == "not")
        return tok_not;
    else if (word == "div")
        return tok_div;
    else if (word == "mod")
        return tok_mod;
    else if (word == "or")
        return tok_or;
    else if (word == "begin")
        return tok_begin;
    else if (word == "end")
        return tok_end;
    else if (word == "const")
        return tok_const;
    else if (word == "procedure")
        return tok_procedure;
    else if (word == "forward")
        return tok_forward;
    else if( word == "function")
        return tok_function;
    else if (word == "if")
        return tok_if;
    else if (word == "then")
        return tok_then;
    else if(word == "else")
        return tok_else;
    else if (word == "program")
        return tok_program;
    else if (word == "while")
        return tok_while;
    else if ( word == "exit")
        return tok_exit;
    else if ( word == "var" )
        return tok_var;
    else if ( word == "integer" )
        return tok_integer;
    else if ( word == "for" )
        return tok_for;
    else if ( word == "do" )
        return tok_do;
    else
    {
        m_IdentifierStr = word;
        return tok_identifier;
    }

}

bool Lexer::isDigitCorrect(char num ,int base,int& value)
{
    if(isdigit(num))
        value = num - '0';
    else if(islower(num))
        value = num - 'a' + 10;
    else if (isupper(num))
        value = num - 'A' + 10;
    else 
        return 0;
    if(value >= base)
        return 0;
    return 1;
}

int Lexer::readNumber(int base)
{
    m_NumVal = 0;
    while(true)
    {
        int nextValue = 0;
        char next = std::cin.peek();
        if(!isalnum(next))
            break;
        if(!isDigitCorrect(next,base,nextValue))
        {
            throw std::runtime_error("Not Correct Digit for the base");
        }
        std::cin.get();
        m_NumVal = m_NumVal * base + nextValue;
    }
    return tok_number;
}
