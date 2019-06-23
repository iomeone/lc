/*
  ==============================================================================

    Parser.h
    Created: 23 Jun 2019 9:47:33am
    Author:  朱飞

  ==============================================================================
*/

#pragma once


#include <list>

#include <stdexcept>
#include <stdio.h>
#include <exception>

#include <mapbox/variant.hpp>
#include "../JuceLibraryCode/JuceHeader.h"
//// The object type
//class Obj {
//public:
//    // The first word of the object represents the type of the object. Any code that handles object
//    // needs to check its type first, then access the following union members.
//
//    Obj(TYPE t)
//    {
//        _type = t;
//    }
//    ~Obj(){}
//    TYPE _type;
//
//    // Object values.
//    union {
//        // Int
//        int value;
//        // Cell
//        struct {
//            struct Obj *car;
//            struct Obj *cdr;
//        };
//        // Symbol
//        String name;
//        // Primitive
//        //        Primitive *fn;
//        // Function or Macro
//        struct {
//            struct Obj *params;
//            struct Obj *body;
//            struct Obj *env;
//        };
//        // Subtype for special type
//        int subtype;
//        // Environment frame. This is a linked list of association lists
//        // containing the mapping from symbols to their value.
//        struct {
//            struct Obj *vars;
//            struct Obj *up;
//        };
//        // Forwarding pointer
//        void *moved;
//    };
//} ;

inline void msg(String s)
{
    AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "msg", s, "ok");
}

struct TNil {int x;};
struct TInt
{
    TInt(int v)
    {
        _val = v;
    }
    int _val;
    
};

struct TSymbol
{
    TSymbol(String s)
    {
        _sym = s;
    }
    String _sym;
};
struct TCons;

using TObj = mapbox::util::variant<TNil*, TInt*, TSymbol*, mapbox::util::recursive_wrapper<TCons*>>;

struct TCons
{
    TCons(TObj h, TObj t)
    {
        _head = h;
        _tail = t;
    }
    TObj _head;
    TObj _tail;
};


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

    
    TObj  Nil()
    {
        return   (new TNil());
    }
    
    TObj  Cons(TObj  head, TObj  tail)
    {
        return   (new TCons(head, tail));
    }
    TObj ListReader( )
    {
        std::list<TObj> lst;
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
                    TObj acc = Nil();
                
                    for_each(lst.begin(), lst.end(), [this, &acc](TObj o)
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
    
    TObj readNumber (char firstChar)
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
    
    
    
    TObj  readSymbol(char firstChar)
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
        return (new TSymbol(acc));
    }
    
    std::function<TObj()> getHander(char c)
    {
       if( c == '(')
       {
           std::function<TObj()> p1 = [this]()
           {
               return this->ListReader();
           };
          
           return p1;
       }
       else if( c == ')')
       {
           char errorInfo[256];
           sprintf(errorInfo, "unexpected ')', line: %ld coloum: %ld", _line, _col);
           throw std::runtime_error(errorInfo);
       }
        else
            return nullptr;
    }
    
    
    
    TObj  readObj()
    {
        eat_whitespace();
        char c;
        if(read(c))
        {
            std::function<TObj()> fun = getHander(c);
            if(fun)
            {
//                msg("call fun");
                TObj r = fun();
//                msg("call fun end");
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
    
    TObj compile( )
    {
        TObj obj = readObj();
        
        return obj;
    }
    
    
    void getASTStr(TObj  obj, String& str)
    {
        obj.match( [&str]  (TNil* a)
                   {
                       str += " Nil";
                       
                   }
                   ,[&str]  (TInt* e)
                   {
                       str += " Int: " + String(e->_val);
                       
                   }
                   ,[&str]  (TSymbol* e)
                   {
                       str += "\nTSymbol: " + String(e->_sym);
                   
                       
                   }
                   ,[&str, this]  (TCons* e)
                   {
                       str += "( ";
                       getASTStr(e->_head, str);
                       getASTStr(e->_tail, str);
                       
                       str += " )";
                   }
                   
                   
                   
                   
                   );//str += "Nil";
                   
                   
    }
    
    
protected:
    
    char * _srcbuf{nullptr};
    size_t _srcbufLen {0};
    size_t _srcPos{0};
    
    size_t _line;
    size_t _col;
    
    
private:
    String _src;
    
};

struct calculator
{
public:
    int operator()(TNil value) const
    {
        return 1;
    }
    
 
};
