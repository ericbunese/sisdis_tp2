/**********************************************************************/
/*                                                                    */
/*                       File "smpl.h"                                */
/*                  smpl Simulation Subsystem                         */
/*                                                                    */
/*  C Version 1.10                        (C) 1987  M. H. MacDougall  */
/*  Oct. 22, 1987                                All Rights Reserved  */
/*                                                                    */
/**********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>     /* should include a declaration for 'atof' */

typedef double real;
#define then    

/* ---------------------- rand names --------------------------------*/
extern double ranf();
extern int stream(int n);
extern long seed(long Ik,int n);
extern double uniform(double a,double b);
extern int randomic(int i, int n);
extern double expntl(double x);
extern double erlang(double x,double s);
extern double hyperx(double x,double s);
extern double normal(double x,double s);
 
/* ---------------------- smpl names --------------------------------*/
extern double time();             
extern double U(int f);
extern double B(int f);
extern double Lq(int f);
extern char *mname();
extern char *fname(int f);
extern FILE *sendto(FILE *dest);   
extern void smpl(int m, char *s);
extern void reset();
static int save_name(char *s, int m);
static int get_blk(int n);
static int get_elm(); 
static void put_elm(int i);
extern void schedule(int ev, real te, int tkn);
extern void cause(int *ev, int *tkn);
extern int cancel(int ev);  
static int suspend(int tkn); 
static void enlist(int *head, int elm);  
extern int facility(char *s, int n);
static void resetf();
extern int request(int f, int tkn, int pri);
static void enqueue(int f, int j, int pri, int ev, real te);
extern int preempt(int f,int tkn, int pri);
extern void release(int f, int tkn);
extern int status(int f); 
extern int inq(int f);  
extern void trace(int n); 
static void msg(int n, int i, char *s, int q1, int q2);
static void end_line();
extern void pause();
extern void error(int n, char *s);
extern void report(); 
extern void reportf();
static int rept_page(int fnxt);
extern int lns(int i); 
extern void endpage();
extern void newpage(); 

/* ------------------------------------------------------------------*/



