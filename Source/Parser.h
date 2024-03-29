/*
  ==============================================================================

    Parser.h
    Created: 23 Jun 2019 9:47:33am
    Author:  朱飞

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Type.h"
#include <list>
#include <stdexcept>
#include <stdio.h>
#include <exception>






class Lan{
public:
    Lan(const String &src)
    {
        _src = src;
        _srcbuf = _src.getCharPointer().getAddress();
        _srcbufLen = strlen(_srcbuf);
    }
    

    
    bool read(char & c)
    {
        if (_srcPos < _srcbufLen)
        {
            c = _srcbuf[_srcPos];
            ++_srcPos;
            return true;
        }
        else
            return false;
    }
    
    bool unread()
    {
        if(_srcPos > 0)
        {
            --_srcPos;
            return true;
        }
        else
            return false;
        
    }
    
    bool is_whitespace(char c)
    {
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c== ',')
        {
            return true;
        }
        else
            return false;
    }
    
    void eat_whitespace()
    {
        while(true)
        {
            char ch;
            if(read(ch))
            {
                if(is_whitespace(ch))
                    continue;
                else
                {
                    unread();
                    break;
                }
                
            }
            else
                break;
        }
    }

    
    TExpr  Nil()
    {
        return   (new TNil());
    }
    
    TExpr  Cons(TExpr  head, TExpr  tail)
    {
        return   (new TCons(head, tail));
    }
    TExpr ListReader( )
    {
        std::list<TExpr> lst;
        while(true)
        {
            eat_whitespace();
           
            char c;
            if(read(c))
            {
//                msg(String::charToString(c));
                if(c == ')')
                {
//                    msg("find )");
                    TExpr acc = Nil();
                
                    for_each(lst.begin(), lst.end(), [this, &acc](TExpr o)
                                                                {
                                                                    // msg("for_each ");
                                                                    acc = this->Cons(o , acc);
                                                                });
                    
                    return acc;
                }
                else
                {
                    unread();
                    lst.push_front(readObj());
                }
            }
            else
            {
//                 msg("expected ')'");
                char errorInfo[256];
                sprintf(errorInfo, "expected ')', line: %ld coloum: %ld", _line, _col);
                throw std::runtime_error(errorInfo);
            }
        }

        
    }
    
    TExpr readNumber (char firstChar)
    {
        std::string acc(1, firstChar);
        while (true)
        {
            char c;
            if(read(c))
            {
                if( isdigit(c) )
                    acc.push_back(c);
                else
                {
                    unread();
                    break;
                }
            }
            else
                break;
        }
        int n = std::stoi(acc);
        auto res = (new TInt(n));
        return res ;
    }
    
    
    
    TExpr  readSymbol(char firstChar)
    {
        std::string acc(1, firstChar);
        while (true)
        {
            char c;
            if(read(c))
            {
                if( is_whitespace(c) || getHander(c) )
                {
                    unread();
                    break;
                }
                else
                    acc.push_back(c);
            }
            else
                break;
        }
        if(acc == "true")
            return new TBool(true);
        else if(acc == "false")
            return new TBool(false);
        else if(acc == "nil")
            return new TNil;
        else
            return new TSymbol(acc);
    }
    
    std::function<TExpr()> getHander(char c)
    {
       if( c == '(')
       {
           std::function<TExpr()> h = [this]()
           {
               return this->ListReader();
           };
           return h;
       }
       else if( c == ')')
       {
		   std::function<TExpr()> h = [this]()
		   {
			   char errorInfo[256];
			   sprintf(errorInfo, "unexpected ')', line: %ld coloum: %ld", _line, _col);
			   throw std::runtime_error(errorInfo);
			   return (new TNil);
		   };
		   return h;
       }
        else
            return nullptr;
    }
    
    
    
    TExpr  readObj()
    {
        eat_whitespace();
        char c;
        if(read(c))
        {
            std::function<TExpr()> fun = getHander(c);
            if(fun)
            {
//              msg("call fun");
                TExpr r = fun();
//              msg("call fun end");
                return r;
            }
            else if(isdigit(c) || c == '-')
            {
                return readNumber(c);
            }
            else
            {
                return readSymbol(c);
            }
                
        }
        
        return (new TNil());
    }
    
    TExpr compile()
    {
        TExpr obj = readObj();
        
        return obj;
    }
    
    
    void getASTStr(TExpr  obj, String& str, int indent = 0)
    {
        obj.match( [&str, this, &indent]  (TNil* a)
                   {
						str +=   "   Nil";
                   }
                   ,[&str, this, &indent]  (TInt* e)
                   {
					   str +=  String(e->_val) + ":Int";
                   }
                   ,[&str, this, indent]  (TSymbol* e)
                   {
					   str +=   String(e->_sym);
                   }
                   ,[&str, this, &indent]  (TCons* e)
                   {
					   if (indent == 0)
						   str += "(";
					   else
						   str += "\n" + getSpace(indent) + "(";

					   ++indent;
                       getASTStr(e->_head, str, indent);
                       getASTStr(e->_tail, str, indent);
                       
                       str += ")";
                   }
                  ,[&str, this, indent]  (Code* e)
                  {
//                      str +=   String(e->_sym);
                  }
                  ,[&str, this, indent]  (TBool* e)
                  {
                      
                  }
				 , [&str, this, indent](SavedClosure* e)
				  {
					  jassertfalse;
				  }
                  );                   
    }
    
    
protected:
    
    char * _srcbuf{nullptr};
    size_t _srcbufLen {0};
    size_t _srcPos{0};
    
    size_t _line;
    size_t _col;
    
    
private:
    String _src;


private:
	String getSpace(int n)
	{
		return String::repeatedString("   ", n);
	}
    
};

struct calculator
{
public:
    int operator()(TNil value) const
    {
        return 1;
    }
    
 
};
