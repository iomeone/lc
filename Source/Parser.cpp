/*
  ==============================================================================

    Parser.cpp
    Created: 23 Jun 2019 9:47:33am
    Author:  朱飞
 
 some of the code reference to https://github.com/rui314/minilisp/blob/nogc/minilisp.c

  ==============================================================================
*/


#include "Parser.h"



//
//static Obj *read(void) {
//    for (;;) {
//        int c = getchar();
//        if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
//            continue;
//        if (c == EOF)
//            return NULL;
//        if (c == ';') {
//            skip_line();
//            continue;
//        }
//        if (c == '(')
//            return read_list();
//        if (c == ')')
//            return Cparen;
//        if (c == '.')
//            return Dot;
//        if (c == '\'')
//            return read_quote();
//        if (isdigit(c))
//            return make_int(read_number(c - '0'));
//        if (c == '-')
//            return make_int(-read_number(0));
//        if (isalpha(c) || strchr("+=!@#$%^&*", c))
//            return read_symbol(c);
//        error("Don't know how to handle %c", c);
//    }
//}
