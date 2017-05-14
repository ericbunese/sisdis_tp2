/**********************************************************************/
/*                                                                    */
/*                       File "smpl.c"                                */
/*                  smpl Simulation Subsystem                         */
/*                                                                    */
/*  C Version 1.10                        (C) 1987  M. H. MacDougall  */
/*  Oct. 22, 1987                                All Rights Reserved  */
/*                                                                    */
/**********************************************************************/

#include "smpl.h"

#define nl 30000     /* element pool length - 30000         */
#define ns 27680     /* name space length - 27680           */
#define pl 58        /* printer page length   (lines used   */
#define sl 23        /* screen page length     by 'smpl'    */
#define FF 12        /* form feed                           */

static FILE
    *display, *opf;
/* A inicializacao das duas variaveis acima, feita quando da
   definicao, como abaixo, no texto original do programa, causa
   erro na compilacao: "Initializer element is not constant".
   A inicializacao passou a ser feita na funcao smpl(). */
/*  *display=stdout, */  /* screen display file                 */
/*  *opf=stdout; */      /* current output destination          */

static int
  event,             /* current simulation event            */
  token,             /* last token dispatched               */
  blk,               /* next available block index          */
  avl,               /* available element list header       */
  evl,               /* event list header                   */
  fchn,              /* facility descriptor chain header    */
  avn,               /* next available namespace position   */
  tr,                /* event trace flag                    */
  mr,                /* monitor activation flag             */
  lft=sl;            /* lines left on current page/screen   */


static real
  clock,             /* current simulation time             */
  start,             /* simulation interval start time      */
  tl;                /* last trace message issue time       */

static int
  l1[nl],
  l2[nl],            /*      facility descriptor,           */
  l3[nl];            /*           queue, &                  */
static real          /*          event list                 */
  l4[nl],            /*         element pool                */
  l5[nl];

static char
  name[ns];          /* model, facility, & table name space */

/*---------------  INITIALIZE SIMULATION SUBSYSTEM  ------------------*/
void smpl(int m, char *s)
    {
      int i; static int rns=1;
      display=opf=stdout;    /* inicializacao e feita aqui */
      blk=1; avl=-1; avn=0;       /* element pool & namespace headers */
      evl=fchn=0;           /* event list & descriptor chain headers  */
      clock=start=tl=0.0;   /* sim., interval start, last trace times */
      event=tr=0;                 /* current event no. & trace flags  */
      for (i=0; i<nl; i++)  {l1[i]=l2[i]=l3[i]=0; l4[i]=l5[i]=0.0;}
      i=save_name(s,50);                   /* model name -> namespace */
      rns=stream(rns); rns=++rns>15? 1:rns;  /* set random no. stream */
      mr=(m>0)? 1:0;                              /* set monitor flag */
     /*  if (mr) then {opf=display; init_mtr(1);} */
    }

/*-----------------------  RESET MEASUREMENTS  -----------------------*/
void reset()
  {
    resetf(); start=clock;
  }

/*---------------------------  SAVE NAME  ----------------------------*/
static int save_name(char *s, int m)
    {
      int i,n;
      n=strlen(s); if (n>m) then n=m;
      if (avn+n>ns) then error(2,0); /* namespace exhausted */
      i=avn; avn+=n+1; strncpy(&name[i],s,n);
      if (n==m) then name[avn++]='\0';
      return(i);
    }

/*-------------------------  GET MODEL NAME  -------------------------*/
char *mname()
  {
    return(name);
  }

/*------------------------  GET FACILITY NAME  -----------------------*/
char *fname(int f)
    {
      return(&name[l3[f+1]]);
    }

/*---------------------------  GET BLOCK  ----------------------------*/
static int get_blk(int n)
    {
      int i;
      if (blk==0) then error(3,0);    /* block request after schedule */
      i=blk; blk+=n;
      if (blk>=nl) then error(1,0);         /* element pool exhausted */
      return(i);
    }

/*--------------------------  GET ELEMENT  ---------------------------*/
static int get_elm()
  {
    int i;
    if (avl<=0) then
      {
        if (avl==0) then error(1,0);        /* empty element list */
      /*  if (mr && !tr) then init_mtr(2);*/
        /* build the free element list from the block of elements */
        /* remaining after all facilities have been defined       */
        for (i=blk; i<(nl-1); i++) l1[i]=i+1;
        avl=blk; blk=0;
      }
    i=avl; avl=l1[i];
    return(i);
  }

/*-------------------------  RETURN ELEMENT  -------------------------*/
static void put_elm(int i)
    {
      l1[i]=avl; avl=i;
    }

/*-------------------------  SCHEDULE EVENT  -------------------------*/
void schedule(int ev, real te, int tkn)
    {
      int i;
      if (te<0.0) then error(4,0); /* negative event time */
      i=get_elm(); l2[i]=tkn; l3[i]=ev; l4[i]=0.0; l5[i]=clock+te;
      enlist(&evl,i);
      if (tr) then msg(1,tkn,"",ev,0);
    }

/*---------------------------  CAUSE EVENT  --------------------------*/
void cause(int *ev, int *tkn)
    {
      int i;
      if (evl==0) then error(5,0);          /* empty event list  */
      i=evl; *tkn=token=l2[i]; *ev=event=l3[i]; clock=l5[i];
      evl=l1[i]; put_elm(i);  /* delink element & return to pool */
      if (tr) then msg(2,*tkn,"",event,0);
   /*   if (mr && (tr!=3)) then mtr(tr,0);*/
    }

/*--------------------------  RETURN TIME  ---------------------------*/
double time()
  {
    return(clock);
  }

/*--------------------------  CANCEL EVENT  --------------------------*/
int cancel(int ev)
    {
      int pred,succ=evl,tkn;
      while((succ!=0) && (l3[succ]!=ev)) {pred=succ; succ=l1[pred];}
      if (succ==0) then return(-1);
      tkn=l2[succ]; if (tr) then msg(3,tkn,"",l3[succ],0);
      if (succ==evl)
        then evl=l1[succ];                 /* unlink  event */
        else l1[pred]=l1[succ];            /* list entry &  */
      put_elm(succ);                       /* deallocate it */
      return(tkn);
    }

/*-------------------------  SUSPEND EVENT  --------------------------*/
static int suspend(int tkn)
    {
      int pred,succ=evl;
      while((succ!=0) && (l2[succ]!=tkn)) {pred=succ; succ=l1[pred];}
      if (succ==0) then error(6,0);   /* no event scheduled for token */
      if (succ==evl)
        then evl=l1[succ];       /* unlink  event */
        else l1[pred]=l1[succ];  /* list entry    */
      if (tr) then msg(6,-1,"",l3[succ],0);
      return(succ);
    }

/*--------------  ENTER ELEMENT IN QUEUE OR EVENT LIST  --------------*/
static void enlist(int *head, int elm)
    { /* 'head' points to head of queue/event list */
      int pred,succ; real arg,v;
      arg=l5[elm]; succ=*head;
      while (1)
        { /* scan for position to insert entry:  event list is order- */
          /* ed in ascending 'arg' values, queues in descending order */
          if (succ==0)
            then break;  /* end of list */
            else
              {
                v=l5[succ];
                if (*head==evl)
                  then
                    { /* event list  */
                      if (v>arg) then break;
                    }
                  else
                    { /* queue: if entry is for a preempted token     */
                      /* (l4, the remaining event time, >0), insert   */
                      /* entry at beginning of its priority class;    */
                      /* otherwise, insert it at the end              */
                      if ((v<arg) || ((v==arg) && (l4[elm]>0.0)))
                        then break;
                    }
              }
          pred=succ; succ=l1[pred];
        }
      l1[elm]=succ; if (succ!=*head) then l1[pred]=elm; else *head=elm;
    }

/*-----------------------  DEFINE FACILITY  --------------------------*/
int facility(char *s, int n)
    {
      int f,i;
      f=get_blk(n+2); l1[f]=n; l3[f+1]=save_name(s,(n>1 ? 14:17));
      if (fchn==0)
        then fchn=f;
        else {i=fchn; while(l2[i+1]) i=l2[i+1]; l2[i+1]=f;}
      if (tr) then msg(13,-1,fname(f),f,0);
      return(f);
    }

/*---------------  RESET FACILITY & QUEUE MEASUREMENTS  --------------*/
static void resetf()
  {
    int i=fchn,j;
      while (i)
        {
          l4[i]=l4[i+1]=l5[i+1]=0.0;
          for (j=i+2; j<=(i+l1[i]+1); j++) {l3[j]=0; l4[j]=0.0;}
          i=l2[i+1];  /* advance to next facility */
        }
    start=clock;
  }

/*------------------------  REQUEST FACILITY  ------------------------*/
int request(int f, int tkn, int pri)
    {
      int i,r;
      if (l2[f]<l1[f])
        then
          { /* facility nonbusy - reserve 1st-found nonbusy server    */
            for (i=f+2; l1[i]!=0; i++);
            l1[i]=tkn; l2[i]=pri; l5[i]=clock; l2[f]++; r=0;
          }
        else
          { /* facility busy - enqueue token marked w/event, priority */
            enqueue(f,tkn,pri,event,0.0); r=1;
          }
      if (tr) then msg(7,tkn,fname(f),r,l3[f]);
      return(r);
    }

/*-------------------------  ENQUEUE TOKEN  --------------------------*/
static void enqueue(int f, int j, int pri, int ev, real te)
    {
      int i;
      l5[f+1]+=l3[f]*(clock-l5[f]); l3[f]++; l5[f]=clock;
      i=get_elm(); l2[i]=j; l3[i]=ev; l4[i]=te; l5[i]=(real)pri;
      enlist(&l1[f+1],i);
    }

/*------------------------  PREEMPT FACILITY  ------------------------*/
int preempt(int f, int tkn, int pri)
    {
      int ev,i,j,k,r; real te;
      if (l2[f]<l1[f])
        then
          { /* facility nonbusy - locate 1st-found nonbusy server     */
            for (k=f+2; l1[k]!=0; k++)
            ; r=0;
            if (tr) then msg(8,tkn,fname(f),0,0);
          }
        else
          { /* facility busy - find server with lowest-priority user  */
            k=f+2; j=l1[f]+f+1;  /* indices of server elements 1 & n  */
            for (i=f+2; i<=j; i++) if (l2[i]<l2[k]) then k=i;
            if (pri<=l2[k])
              then
                { /* requesting token's priority is not higher than   */
                  /* that of any user: enqueue requestor & return r=1 */
                  enqueue(f,tkn,pri,event,0.0); r=1;
                  if (tr) then msg(7,tkn,fname(f),1,l3[f]);
                }
              else
                { /* preempt user of server k.  suspend event, save   */
                  /* event number & remaining event time, & enqueue   */
                  /* preempted token.  If remaining event time is 0   */
                  /* (preemption occurred at the instant release was  */
                  /* to occur, set 'te' > 0 for proper enqueueing     */
                  /* (see 'enlist').  Update facility & server stati- */
                  /* stics for the preempted token, and set r = 0 to  */
                  /* reserve the facility for the preempting token.   */
                  if (tr) then msg(8,tkn,fname(f),2,0);
                  j=l1[k]; i=suspend(j); ev=l3[i]; te=l5[i]-clock;
                  if (te==0.0) then te=1.0e-99; put_elm(i);
                  enqueue(f,j,l2[k],ev,te);
                  if (tr) then
                    {msg(10,-1,"",j,l3[f]); msg(12,-1,fname(f),tkn,0);}
                  l3[k]++; l4[k]+=clock-l5[k];
                  l2[f]--; l4[f+1]++; r=0;
                }
          }
      if (r==0) then
        { /* reserve server k of facility */
          l1[k]=tkn; l2[k]=pri; l5[k]=clock; l2[f]++;
        }
      return(r);
    }

/*------------------------  RELEASE FACILITY  ------------------------*/
void release(int f, int tkn)
    {
      int i,j=0,k,m; real te;
      /* locate server (j) reserved by releasing token */
      k=f+1+l1[f];     /* index of last server element */
      for (i=f+2; i<=k; i++) if (l1[i]==tkn) then {j=i; break;}
      if (j==0) then error(7,0); /* no server reserved */
      l1[j]=0; l3[j]++; l4[j]+=clock-l5[j]; l2[f]--;
      if (tr) then msg(9,tkn,fname(f),0,0);
      if (l3[f]>0) then
        { /* queue not empty:  dequeue request ('k' =  */
          /* index of element) & update queue measures */
          k=l1[f+1]; l1[f+1]=l1[k]; te=l4[k];
          l5[f+1]+=l3[f]*(clock-l5[f]); l3[f]--; l4[f]++; l5[f]=clock;
          if (tr) then msg(11,-1,"",l2[k],l3[f]);
          if (te==0.0) then
            then
              { /* blocked request:  place request at head of event   */
                /* list (so its facility request can be re-initiated  */
                /* before any other requests scheduled for this time) */
                l5[k]=clock; l1[k]=evl; evl=k; m=4;
              }
            else
              { /* return after preemption:  reserve facility for de- */
                /* queued request & reschedule remaining event time   */
                l1[j]=l2[k]; l2[j]=(int)l5[k]; l5[j]=clock; l2[f]++;
                if (tr) then msg(12,-1,fname(f),l2[k],0);
                l5[k]=clock+te; enlist(&evl,k); m=5;
              }
          if (tr) then msg(m,-1,"",l3[k],0);
        }
    }

/*-----------------------  GET FACILITY STATUS  ----------------------*/
int status(int f)
    {
      return(l2[f]==l1[f]);
    }

/*--------------------  GET CURRENT QUEUE LENGTH  --------------------*/
int inq(int f)
    {
      return(l3[f]);
    }

/*--------------------  GET FACILITY UTILIZATION  --------------------*/
double U(int f)
    {
      int i; real b=0.0,t=clock-start;
      if (t>0.0) then
        {
          for (i=f+2; i<=f+l1[f]+1; i++) b+=l4[i];
          b/=t;
        }
      return(b);
    }

/*----------------------  GET MEAN BUSY PERIOD  ----------------------*/
double B(int f)
    {
      int i,n=0; real b=0.0;
      for (i=f+2; i<=f+l1[f]+1; i++) {b+=l4[i]; n+=l3[i];}
      return((n>0)? b/n:b);
    }

/*--------------------  GET AVERAGE QUEUE LENGTH  --------------------*/
double Lq(int f)
    {
      real t=clock-start;
      return((t>0.0)? (l5[f+1]/t):0.0);
    }

/*-----------------------  TURN TRACE ON/OFF  ------------------------*/
void trace(int n)
    {
      switch(n)
        {
          case 0: tr=0; break;
          case 1:
          case 2:
          case 3: tr=n; tl=-1.0; newpage(); break;
          case 4: end_line(); break;
         default: break;
        }
    }

/*--------------------  GENERATE TRACE MESSAGE  ----------------------*/
static void msg(int n, int i, char *s, int q1, int q2)
    {
      static char *m[14] = {"","SCHEDULE","CAUSE","CANCEL",
        "   RESCHEDULE","   RESUME","   SUSPEND","REQUEST","PREEMPT",
        "RELEASE","   QUEUE","   DEQUEUE","   RESERVE","FACILITY"};
      if (clock>tl)      /* print time stamp (if time has advanced) */
        then {tl=clock; fprintf(opf,"  time %-12.3f  ",clock);}
        else fprintf(opf,"%21s",m[0]);
      if (i>=0)          /* print token number if specified */
        then fprintf(opf,"--  token %-4d  -- ",i);
        else fprintf(opf,"--              -- ");
      fprintf(opf,"%s %s",m[n],s);   /* print basic message */
      switch(n)
        { /* append qualifier */
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:  fprintf(opf," EVENT %d",q1); break;
          case 7:
          case 8:  switch(q1)
                     {
                       case 0: fprintf(opf,":  RESERVED"); break;
                       case 1: fprintf(opf,":  QUEUED  (inq = %d)",q2);
                               break;
                       case 2: fprintf(opf,":  INTERRUPT"); break;
                      default: break;
                     }
                   break;
          case 9:  break;
          case 10:
          case 11: fprintf(opf," token %d  (inq = %d)",q1,q2); break;
          case 12: fprintf(opf," for token %d",q1); break;
          case 13: fprintf(opf,":  f = %d",q1); break;
          default: break;
        }
      fprintf(opf,"\n"); end_line();
    }

/*-------------------------  TRACE LINE END  -------------------------*/
static void end_line()
  {
    if ((--lft)==0) then
      { /* end of page/screen.  for trace 1, advance page if print- */
        /* er output;  screen output is free-running.  for trace 2, */
        /* pause on full screen;  for trace 3, pause after line.    */
        switch(tr)
          {
            case 1: if (opf==display)
                      then lft=sl;
                      else endpage();
                    break;
            case 2: if (mr)
                      then {putchar('\n'); lft=sl; pause();}
                      else endpage();
                    break;
            case 3: lft=sl; break;
          }
      }
    if (tr==3) then pause();
  }

/*-----------------------------  PAUSE  ------------------------------*/
void pause()
  { /* pause execution via 'mtr' call (if active) */
  /*  if (mr) then mtr(tr,1); else */ getchar();
  }

/*------------------  DISPLAY ERROR MESSAGE & EXIT  ------------------*/
void error(int n, char *s)
    {
      FILE *dest;
      static char
          *m[8]={"Simulation Error at Time ",
                 "Empty Element Pool",
                 "Empty Name Space",
                 "Facility Defined After Queue/Schedule",
                 "Negative Event Time",
                 "Empty Event List",
                 "Preempted Token Not in Event List",
                 "Release of Idle/Unowned Facility"};
      dest=opf;
      while (1)
        { /* send messages to both printer and screen */
          fprintf(dest,"\n**** %s%.3f\n",m[0],clock);
          if (n) fprintf(dest,"     %s\n",m[n]);
          if (s) fprintf(dest,"     %s\n",s);
          if (dest==display) then break; else dest=display;
        }
      if (opf!=display) then report();
   /*   if (mr) then mtr(0,1); */
      exit(0);
    }

/*------------------------  GENERATE REPORT  -------------------------*/
void report()
  {

    newpage();
    reportf();
    endpage();
  }

/*--------------------  GENERATE FACILITY REPORT  --------------------*/
void reportf()
  {
    int f;
    if ((f=fchn)==0)
      then fprintf(opf,"\nno facilities defined:  report abandoned\n");
      else
        { /* f = 0 at end of facility chain */
          while(f) {f=rept_page(f); if (f>0) then endpage();}
        }
  }

/*----------------------  GENERATE REPORT PAGE  ----------------------*/
static int rept_page(int fnxt)
    {
      int f,i,n; char fn[19];
      static char *s[7]={
        "smpl SIMULATION REPORT", " MODEL: ", "TIME: ", "INTERVAL: ",
        "MEAN BUSY     MEAN QUEUE        OPERATION COUNTS",
        " FACILITY          UTIL.    ",
        " PERIOD        LENGTH     RELEASE   PREEMPT   QUEUE"};
      fprintf(opf,"\n%51s\n\n\n",s[0]);
      fprintf(opf,"%-s%-54s%-s%11.3f\n",s[1],mname(),s[2],clock);
      fprintf(opf,"%68s%11.3f\n\n",s[3],clock-start);
      fprintf(opf,"%75s\n",s[4]);
      fprintf(opf,"%s%s\n",s[5],s[6]);
      f=fnxt; lft-=8;
      while (f && lft--)
        {
          n=0; for (i=f+2; i<=f+l1[f]+1; i++) n+=l3[i];
          if (l1[f]==1)
            then sprintf(fn,"%s",fname(f));
            else sprintf(fn,"%s[%d]",fname(f),l1[f]);
          fprintf(opf," %-17s%6.4f %10.3f %13.3f %11d %9d %7d\n",
            fn,U(f),B(f),Lq(f),n,(int)l4[f+1],(int)l4[f]);
          f=l2[f+1];
        }
      return(f);
    }

/*---------------------------  COUNT LINES  --------------------------*/
int lns(int i)
    {
      lft-=i;  if (lft<=0) then endpage();
      return(lft);
    }

/*----------------------------  END PAGE  ----------------------------*/
void endpage()
  {
    /*int c,key;*/

    if (opf==display)
      then
        { /* screen output: push to top of screen & pause */
          while(lft>0) {putc('\n',opf); lft--;}
       /*   printf("\n[ENTER] to continue:");  getchar(); */
       /*   if (mr) then clr_scr(); else  */ printf("\n\n");
        }
      else if (lft<pl) then putc(FF,opf);
    newpage();

  }

/*----------------------------  NEW PAGE  ----------------------------*/
void newpage()
  { /* set line count to top of page/screen after page change/screen  */
    /* clear by 'smpl', another SMPL module, or simulation program    */
    lft=(opf==display)? sl:pl;
  }

/*------------------------  REDIRECT OUTPUT  -------------------------*/
FILE *sendto(FILE *dest)
    {
      if (dest) then opf=dest;
      return(opf);
    }
