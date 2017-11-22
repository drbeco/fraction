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
 *   $gcc frac.c -o frac.x -Wall
 *        -Wextra -ansi -pedantic-errors -g -O0 -DDEBUG=1
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

exp_t analyse(char *s); /* break string into tokens. error: return ex.nop=0 */
number_t calc(exp_t e); /* calculate the expression */
void printnum(number_t n); /* print the result */
kind_t kind(char *t, exp_t *e, int nop); /* return the token kind {none, integer, fraction, operator} and attribute it to exp_t e */
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

    e=analyse(sexp);
    if(!e.nop) /* nothing to operate */
    {
        printf("Error: nothing to do!\n");
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

/* return the token kind {none, integer, fraction, operator} and attribute it to exp_t e */
kind_t kind(char *t, exp_t *e, int nop)
{
    char *pm=NULL, *pe=NULL; /* middle and end pointers */
    int n, d;

    e->nop=nop;
    if(strlen(t)==1 && (t[0]==plus || t[0]==minus || t[0]==times || t[0]==over))
    {
        e->op=t[0];
        return operator;
    }

    n=strtol(t, &pm, 10);
    if(t==pm) /* no good digits at all */
        return none;

    if(*pm=='\0') /* all good */
    {
        if(nop==1)
            e->n1.i=n;
        else
            e->n2.i=n;
        return integer;
    }

    if(*pm=='/') /* maybe fraction */
    {
        pm++;
        d=strtol(pm, &pe, 10);
        if(pm==pe || *pe!='\0') /* no digits OR none good OR remainder left */
            return none;
        if(nop==1)
        {
            e->n1.n=n;
            e->n1.d=d;
        }
        else
        {
            e->n2.n=n;
            e->n2.d=d;
        }
        return fraction; /* pm!=pe && pe=='\0', all good again */
    }

    return none; /* not a '/' (over) separator */
}

/* break string into tokens. error: return ex.nop=0 */
exp_t analyse(char *s)
{
    char stk[SBUFF]; /* copy of string expression */
    char *t; /* tokenizing */
    kind_t k; /* token kind */
    int qs=0; /* state */
    exp_t ex;

    ex.n1.i = ex.n1.n = ex.n2.i = ex.n2.n = 0;
    ex.n1.d = ex.n2.d = 1;

    strncpy(stk, s, SBUFF); /* preserve original s */

    t=strtok(stk, " "); /* first token must be a num */

    while(t!=NULL)
    {
        k=kind(t, &ex, (qs>=3)+1);

        switch(qs)
        {
            case 0:
                switch(k)
                {
                    case integer:
                        qs=1;
                        break;
                    case fraction:
                        qs=2;
                        break;
                    default:
                        ex.nop=0;
                        return ex;
                }
                break;
            case 1:
                switch(k)
                {
                    case fraction:
                        qs=2;
                        break;
                    case operator:
                        qs=3;
                        break;
                    default:
                        ex.nop=0;
                        return ex;
                }
                break;
            case 2:
                switch(k)
                {
                    case operator:
                        qs=3;
                        break;
                    default:
                        ex.nop=0;
                        return ex;
                }
                break;
            case 3:
                switch(k)
                {
                    case integer:
                        qs=4;
                        break;
                    case fraction:
                        qs=5;
                        break;
                    default:
                        ex.nop=0;
                        return ex;
                }
                break;
            case 4:
                switch(k)
                {
                    case fraction:
                        qs=5;
                        break;
                    default:
                        ex.nop=0;
                        return ex;
                }
                break;
            case 5:
            default:
                ex.nop=0;
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

    if(qs==0 || qs==3)
    {
        printf("Syntax error\n");
        ex.nop=0;
        return ex;
    }

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

