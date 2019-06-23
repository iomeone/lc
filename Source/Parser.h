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

struct TNil {};
struct TInt
{
    TInt(int v)
    {
        val = v;
    }
    int val;
    
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

using TObj = mapbox::util::variant<TNil, TInt, mapbox::util::recursive_wrapper<TCons>>;

struct TCons
{
    TCons(TObj * h, TObj* t)
    {
        _head = h;
        _tail = t;
    }
    TObj* _head;
    TObj* _tail;
};


class Lan{
    
    
public:
    Lan(const String &src)
    {
        _src = src;
        _srcbuf = _src.getCharPointer().getAddress();
        _srcbufLen = strlen(_srcbuf);
        _srcbufLen = 0;
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
                    unread();
            }
            else
                break;
        }
    }

    
    TObj * Nil()
    {
        return (TObj*) (new TNil());
    }
    
    TObj * Cons(TObj * head, TObj * tail)
    {
        return   (TObj*)(new TCons(head, tail));
    }
    TObj* ListReader( )
    {
        while(true)
        {
            eat_whitespace();
            std::list<TObj*> lst;
            char c;
            if(read(c))
            {
                if(c == ')')
                {
                    TObj* acc = Nil();
                
                    for_each(lst.begin(), lst.end(), [this, &acc](TObj *o)
                    {
                        acc = this->Cons(o , acc);
                        return acc;
                    });
                }
                
                unread();
                lst.push_front(readObj());
            }
            else
            {
                char errorInfo[256];
                sprintf(errorInfo, "expected ')', line: %ld coloum: %ld", _line, _col);
                throw std::runtime_error(errorInfo);
            }
        }

        
    }
    
    TObj* readNumber (char firstChar)
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
        return (TObj* )(new TInt(n));
    }
    
    
    
    TObj * readSymbol(char firstChar)
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
        return (TObj* )(new TSymbol(acc));
    }
    
    std::function<TObj*(Lan&)> getHander(char c)
    {
       if( c == '(')
       {
           std::function<TObj*(Lan&)> p1 = &Lan::ListReader;
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
    
    
    
    TObj * readObj()
    {
        eat_whitespace();
        char c;
        if(read(c))
        {
            if(auto fun = getHander(c))
            {
                fun(*this);
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
        
        return nullptr;
    }
    
    bool compile( )
    {
        TObj * obj = readObj();
        
        return true;
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
