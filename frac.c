/***************************************************************************
 *   frac.c                                   Version 20171114.163840      *
 *                                                                         *
 *   Calculadora de fracao                                                 *
 *   Copyright (C) 2017         by Ruben Carlo Benante                     *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *   To contact the author, please write to:                               *
 *   Ruben Carlo Benante                                                   *
 *   Email: rcb@beco.cc                                                    *
 *   Webpage: http://www.beco.cc                                           *
 *   Phone: +55 (81) 3184-7555                                             *
 ***************************************************************************/

/* ---------------------------------------------------------------------- */
/*
 * Instrucoes para compilar:
 *   $ gcc frac.c -o frac.x -Wall -Wextra -ansi -pedantic-errors 
 *   Para debug: -g -O0 -DDEBUG=1
 */

/* ---------------------------------------------------------------------- */
/* includes */

#include <stdio.h> /* Standard I/O functions */
#include <stdlib.h> /* Miscellaneous functions (rand, malloc, srand)*/
#include <string.h> /* Strings functions definitions */

/* ---------------------------------------------------------------------- */
/* definitions */

#ifndef VERSION /* gcc -DVERSION="0.1.160612.142306" */
#define VERSION "20171114.163840" /**< Version Number (string) */
#endif

/* Debug */
#ifndef DEBUG /* gcc -DDEBUG=1 */
#define DEBUG 0 /**< Activate/deactivate debug mode */
#endif

#if DEBUG==0
#define NDEBUG
#endif
/* #include <assert.h> */ /* Verify assumptions with assert. Turn off with #define NDEBUG */

/** @brief Debug message if DEBUG on */
#define IFDEBUG(M) if(DEBUG) fprintf(stderr, "[DEBUG file:%s line:%d]: " M "\n", __FILE__, __LINE__); else {;}

/* limits */
#define SBUFF 64 /**< string buffer */
#define BASE 10 /**< number base to operate */

/* ---------------------------------------------------------------------- */
/* data */

typedef struct number_s
{
    int i; /* integer part */
    int n; /* numerator */
    int d; /* denominator */
} number_t;

typedef enum kind_e {none, integer, fraction, operator} kind_t; /* kinds of tokens */
typedef enum operator_e {plus='+', minus='-', times='*', over='/'} ope_t; /* operators */

/* an expression is "n1" (with nop=1) or "n1 op n2" (with nop=2) */
typedef struct expression_s
{
    number_t n1, n2; /* operands initialized with 0 0/1 */
    ope_t op; /* operator: plus, minus, times or over */
    int nop; /* number of operands: 1 or 2 */
} exp_t;

/* ---------------------------------------------------------------------- */
/* prototypes */

/* syntatic analyse */
exp_t syntactic_analyse(char *s); /* syntactic analyse break string into tokens. error: return ex.nop=0 */

/* lexical analyse */
kind_t lexical_analyse(char *t); /* return the token kind {none, integer, fraction, operator} */
number_t token_breaker(char *t, kind_t k); /* get the token kind k and set it in the expression (ignore none and operator) */

/* evaluator */
number_t calc(exp_t e); /* calculate the expression */

/* support functions */
void printnum(number_t n); /* print the result */
int mdc(int x, int y); /* calculate the lcd (less common divisor) of 2 numbers */
void help(void); /* print help and exit */

/* ---------------------------------------------------------------------- */
int main(int ac, char *av[])
{
    char sexp[SBUFF]; /* a string to hold the expression */
    char *p; /* *p temp pointer */
    exp_t e; /* expression */
    number_t r; /* result */
    int i=1, c; /* index, counter */

    if(ac>1)
    {
        if(!strcmp(av[1], "-h"))
            help(); /* print help and exit */

        /* expression from command line */
        sexp[0]='\0';
        c=strlen(av[i])+1; /* add a space after token */
        while(c<SBUFF)
        {
            strcat(sexp, av[i]);
            strcat(sexp, " ");
            i++;
            if(i>=ac)
                break;
            c += strlen(av[i]) + 1; /* add a space after token */
        }
    }
    else /* expression from stdin (keyboard) */
    {
        printf("Expression (int? frac?) ( (+|-|*|/) (int? frac?) )?\n");
        printf("Where (int? frac?) is at least one integer or one fraction or both (mixed fraction).\n");
        printf("Maximum %d characters.\n", SBUFF-1);
        printf("?");

        fgets(sexp, SBUFF, stdin);
        if((p=strchr(sexp, '\n'))!=NULL)
            *p='\0';
    }

    e=syntactic_analyse(sexp);
    if(!e.nop) /* nothing to operate */
    {
        printf("Syntax error\n");
        exit(EXIT_FAILURE);
    }

    r=calc(e); /* calculate the result */

    printf("Result: ");
    printnum(r); /* print the result */
    printf("\n");

    if(r.d!=0.0)
        printf("Decimal: %.4f\n", r.n/(float)r.d);
    else
        printf("Decimal: division by zero\n");

    return EXIT_SUCCESS;
}

/* print help and exit */
void help(void)
{
    printf("%s - %s - Version: %s\n", "frac", "Calculates and simplifies with mixed fractions", VERSION);
    printf("\nUsage: %s [-h] [(integer|mixed|fraction) ((+|-|*|/) (integer|mixed|fraction))?]\n", "frac");
    printf("\nOptions:\n");
    printf("\t-h,  --help\n\t\tShow this help.\n");
    printf("\t-v,  --verbose\n\t\tSet verbose level (cumulative). (not implemented)\n\n");
    printf("\tinteger:\n\t\ta number composed of digits 0..9 (e.g. 123)\n");
    printf("\tfraction:\n\t\ttwo integers separated by '/' with no spaces in between (e.g. 4/56)\n");
    printf("\tmixed:\n\t\ta mixed fraction is a integer followed by a fraction, separated by a space (e.g. 12 3/4)\n");

    printf("\nExit status:\n\t0 if ok.\n\ta error code otherwise.\n");
    printf("\nAuthor:\n\tWritten by %s <%s>\n\n", "Ruben Carlo Benante", "rcb@beco.cc");
    exit(EXIT_FAILURE);
}

/* break integer or fraction  */
number_t token_breaker(char *t, kind_t k) 
{
    number_t n={0};
    int in, d; /* numerator, denominator */
    char *pm=NULL, *pe=NULL; /* middle pointer and end pointer */

    /* this function do nothing with operators or unknow tokens */
    if(k==none || k==operator)
        return n; /* n={0}, nothing to do */

    in=strtol(t, &pm, BASE); /* convert a string t into a number n base 10, lefting the rest at pm */
    /* it's an integer */
    if(k==integer)
    {
        n.i=in; /* integer */
        n.n=0; /* numerator */
        n.d=1; /* denominator */
        return n;
    }

    /* it's a fraction */
    pm++; /* ignore '/' */
    d=strtol(pm, &pe, BASE); /* convert a string pm into a number d base 10, lefting the rest at pe */
    n.i=0; /* integer */
    n.n=in; /* numerator */
    n.d=d; /* denominator */
    return n;
}

/* return the token kind {none, integer, fraction, operator} */
kind_t lexical_analyse(char *t)
{
    char *pm=NULL, *pe=NULL; /* pm: middle pointer ; pe: end pointer */

    /* found an operator */
    if(strlen(t)==1 && (t[0]==plus || t[0]==minus || t[0]==times || t[0]==over))
        return operator;

    strtol(t, &pm, BASE); /* convert a string t into a number n base 10, lefting the rest at pm */
    if(t==pm) /* no good digits at all */
        return none; /* unrecognized token */

    if(*pm=='\0') /* all good digits */
        return integer; /* it is a simple integer */

    if(*pm=='/') /* maybe fraction */
    {
        pm++; /* skip '/' */
        strtol(pm, &pe, BASE); /* convert a string pm into a number d base 10, lefting the rest at pe */
        if(pm==pe || *pe!='\0') /* no digits OR none good OR remainder left */
            return none; /* denominator not an integer: token unrecognized */
        return fraction; /* otherwise, it is a fraction. pm!=pe && pe=='\0', all good again */
    }

    return none; /* not a '/' (over) separator */
}

/* syntaxe_analyse break string into tokens. error: return ex.nop=0 */
exp_t syntactic_analyse(char *s)
{
    char stk[SBUFF]; /* copy of string expression */
    char *t; /* tokenizing */
    kind_t k; /* token kind */
    int qs=0; /* state */
    exp_t ex; /* expression */
    number_t nb={0}; /* number during analyses */

    ex.n1.i = ex.n1.n = ex.n2.i = ex.n2.n = 0;
    ex.n1.d = ex.n2.d = 1;

    strncpy(stk, s, SBUFF); /* preserve original s */

    t=strtok(stk, " "); /* first token must be a num */

    while(t!=NULL)
    {
        k=lexical_analyse(t); /* what is this token? (none, integer, fraction or operator */
        nb=token_breaker(t, k); /* break token into number (if integer or fraction) */

        /* see automata.txt to understand the automata depicted bellow with qs as states */
        switch(qs) 
        {
            case 0:
                switch(k)
                {
                    case integer:
                        ex.n1.i = nb.i;
                        qs = 1;
                        ex.nop = 1; /* we have something to work */
                        break;
                    case fraction:
                        ex.n1.n = nb.n;
                        ex.n1.d = nb.d;
                        qs = 2;
                        ex.nop = 1; /* we have something to work */
                        break;
                    default:
                        ex.nop = 0; /* syntax error */
                        return ex;
                }
                break;
            case 1:
                switch(k)
                {
                    case fraction:
                        ex.n1.n = nb.n;
                        ex.n1.d = nb.d;
                        qs = 2;
                        ex.nop = 1; /* we have something to work */
                        break;
                    case operator:
                        ex.op = *t; /* the operator +, -, *, / */
                        qs = 3;
                        break;
                    default:
                        ex.nop = 0; /* syntax error */
                        return ex;
                }
                break;
            case 2:
                switch(k)
                {
                    case operator:
                        ex.op = *t; /* the operator +, -, *, / */
                        qs = 3;
                        break;
                    default:
                        ex.nop = 0; /* syntax error */
                        return ex;
                }
                break;
            case 3:
                switch(k)
                {
                    case integer:
                        ex.n2.i = nb.i;
                        ex.nop = 2; /* we have 2 operands */
                        qs = 4;
                        break;
                    case fraction:
                        ex.n2.n = nb.n;
                        ex.n2.d = nb.d;
                        ex.nop = 2; /* we have 2 operands */
                        qs = 5;
                        break;
                    default:
                        ex.nop = 0; /* syntax error */
                        return ex;
                }
                break;
            case 4:
                switch(k)
                {
                    case fraction:
                        ex.n2.n = nb.n;
                        ex.n2.d = nb.d;
                        ex.nop = 2; /* we have 2 operands */
                        qs = 5;
                        break;
                    default:
                        ex.nop = 0; /* syntax error */
                        return ex;
                }
                break;
            case 5:
            default:
                ex.nop = 0; /* syntax error */
                return ex;
        }
        t=strtok(NULL, " (),.");
    }

    printf("Expression: ");
    printnum(ex.n1);
    if(ex.nop==2)
    {
        printf(" %c ", ex.op);
        printnum(ex.n2);
    }
    printf("\n");

    /* cannot finish at states 0 or 3: syntaxe error */
    if(qs==0 || qs==3)
    {
        ex.nop=0; /* syntax error */
        return ex;
    }

    /* all good, return the expression assembled */
    return ex;
}

/* calculate the expression */
number_t calc(exp_t e)
{
    number_t r={0};
    int m;

    e.n1.n += e.n1.i*e.n1.d;
    e.n1.i = 0;

    printf("Fraction: ");
    printnum(e.n1);

    if(e.nop==2)
    {
        /* errno=ENOSYS; */
        e.n2.n += e.n2.i*e.n2.d;
        e.n2.i = 0;
        printf(" %c ", e.op);
        printnum(e.n2);

        switch(e.op)
        {
            case plus:
                r.n = e.n1.n*e.n2.d + e.n2.n*e.n1.d;
                r.d = e.n1.d * e.n2.d;
                break;
            case minus:
                r.n = e.n1.n*e.n2.d - e.n2.n*e.n1.d;
                r.d = e.n1.d * e.n2.d;
                break;
            case times:
                r.n = e.n1.n * e.n2.n;
                r.d = e.n1.d * e.n2.d;
                break;
            case over:
                r.n = e.n1.n * e.n2.d;
                r.d = e.n1.d * e.n2.n;
                break;
            default:
                printf("Operation not supported!\n"); /* this should never happen */
                exit(1); /* abort here */
        }
    }
    else /* only one number to simplify */
    {
        r.n = e.n1.n;
        r.d = e.n1.d;
    }

    printf("\n");

    if(e.nop==2)
    {
        printf("Intermediate: ");
        printnum(r);
        printf("\n");
    }

    /* avoid division by 0 */
    if(r.d==0) 
    {
        r.n = 1; /* cosmetic */
        return r; 
    }

    /* simplification */
    m = mdc(r.n, r.d);
    r.n /= m;
    r.d /= m;

    return r;
}

/* calculate the gcd (greatest common divisor / maximo divisor comum) of 2 numbers */
int mdc(int x, int y)
{
    if(!y)
        return x;
    else
        return(mdc(y, x%y));
}

/* print the result */
void printnum(number_t n)
{
    if(n.i==0) /* no integer, only the fraction part */
        printf("%d/%d", n.n, n.d);
    else /* there is an integer */
        if(n.n==0) /* and no fraction, only the integer */
            printf("%d", n.i);
        else /* and a fraction: mixed fraction */
            printf("%d %d/%d", n.i, n.n, n.d);
}

/* ---------------------------------------------------------------------- */
/* vi: set ai et ts=4 sw=4 tw=0 wm=0 fo=croql : C config for Vim modeline */
/* Template by Dr. Beco <rcb at beco dot cc> Version 20160612.142044      */

