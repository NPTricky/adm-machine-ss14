/********************************************************/
/* Special Purpose Proxel-Based Solver                  */
/*                                                      */
/* Advanced Discrete Modeling 2006                      */
/*                                                      */
/* written/modified by                                  */
/* Graham Horton, Sanja Lazarova-Molnar,                */
/* Fabian Wickborn, Tim Benedict Jagla                  */
/********************************************************/

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>

#define MINPROB    1.0e-12
#define HPM        0
#define LPM        2
#define WP         0 /* (emission) working part */
#define DP         1 /* (emission) defective part */
#define DELTA      1
#define ENDTIME    50
#define PI         3.1415926
#define EMISSION   1 /* 1 = active */

typedef struct tproxel *pproxel;

typedef struct tproxel {
  int     id;                  /* unique proxel id for searching    */
  int     s;                   /* discrete state of SPN             */
  int     tau1k;               /* first supplementary variable      */
  int     tau2k;               /* second supplementary variable     */
  double  val;                 /* proxel probability                */
  pproxel left, right;         /* pointers to child proxels in tree */
} proxel;

double *y[3];                  /* vectors for storing solution      */
double  tmax;                  /* maximum simulation time           */
int     TAUMAX;                /* maximum simulation steps          */
int     totcnt;                /* counts total proxels processed    */
int     maxccp;                /* counts max # concurrent proxels   */
int     ccpcnt;                /* counts concurrent proxels         */
proxel *root[2];               /* trees for organising proxels      */
proxel *firstfree = NULL;      /* linked list of free proxels       */
double  eerror = 0;            /* accumulated error                 */
int     sw = 0;                /* switch for old and new time steps */
double  dt;                    /* delta time (DELTA)                */
int     e;                     /* flag to forward emissio           */
double *em[3];                 /* emission matrix                   */
double *emsum[2];              /* symbol emission sum per timestep  */
int    *emsequence;            /* symbol emission sequence          */
int    *empaths[5];            /* most likely generating paths      */

/********************************************************/
/* distribution functions                               */
/* instantaneous rate functions                         */
/********************************************************/

/*
hrf = hazard rate function
pdf = probability density function
cdf = cumulated distribution function
irf = instantaneous rate function
*/

/* returns weibull instantaneous rate function */
double weibullhrf(double x, double alpha, double beta, double x0) {
  double y;

  y = beta / alpha*pow((x - x0) / alpha, beta - 1);

  return y;
}

/* returns deterministic instantaneous rate function */
double dethrf(double x, double d) {
  double y;

  if (fabs(x - d) < dt / 2)
    y = 1.0 / dt;
  else
    y = 0.0;
  
  return y;
}

/* returns uniform instantaneous rate function */
double unihrf(double x, double a, double b) {
  double y;
  
  if ((x >= a) && (x < b))
    y = 1.0 / (b - x);
  else
    y = 0.0;

  return y;
}

/* returns exponential instantaneous rate function */
double exphrf(double x, double l) {
  return l;
}

/* returns normal probability density function */
double normalpdf(double x, double m, double s) {
  double z = (x - m) / s;
  return exp(-z * z / 2) / (sqrt(2 * PI)*s);
}

double logGamma(double x){
  double coef[] = { 76.18009173, -86.50532033, 24.01409822, -1.231739516, 0.00120858003, -0.00000536382 };
  double step = 2.50662827465, fpf = 5.5, t, tmp, ser;
  int i;

  t = x - 1;
  tmp = t + fpf;
  tmp = (t + 0.5) * log(tmp) - tmp;
  ser = 1;
  for (i = 1; i <= 6; i++) {
    t = t + 1;
    ser = ser + coef[i - 1] / t;
  }
  return tmp + log(step * ser);
}

double gammaSeries(double x, double a){
  int n, maxit = 100;
  double eps = 0.0000003;
  double sum = 1.0 / a, ap = a, gln = logGamma(a), del = sum;

  for (n = 1; n <= maxit; n++) {
    ap++;
    del = del * x / ap;
    sum = sum + del;
    if (fabs(del) < fabs(sum) * eps) break;
  }
  return sum * exp(-x + a * log(x) - gln);
}

double gammaCF(double x, double a){
  int n, maxit = 100;
  double eps = 0.0000003;
  double gln = logGamma(a), g = 0, gold = 0, a0 = 1, a1 = x, b0 = 0, b1 = 1, fac = 1;
  double an, ana, anf;

  for (n = 1; n <= maxit; n++) {
    an = 1.0 * n;
    ana = an - a;
    a0 = (a1 + a0 * ana) * fac;
    b0 = (b1 + b0 * ana) * fac;
    anf = an * fac;
    a1 = x * a0 + anf * a1;
    b1 = x * b0 + anf * b1;
    if (a1 != 0) {
      fac = 1.0 / a1;
      g = b1 * fac;
      if (fabs((g - gold) / g) < eps)
        break;
      gold = g;
    }
  }
  return exp(-x + a * log(x) - gln) * g;
}

double gammacdf(double x, double a){
  if (x <= 0)
    return 0;
  else
    if (x < a + 1)
      return gammaSeries(x, a);
    else
      return 1 - gammaCF(x, a);
}

/* returns normal cumulated distribution function */
double normalcdf(double x, double m, double s){
  double z = (x - m) / s;

  if (z >= 0)
    return 0.5 + 0.5 * gammacdf(z * z / 2, 0.5);
  else
    return 0.5 - 0.5 * gammacdf(z * z / 2, 0.5);
}

/* returns normal instantaneous rate function */
double normalhrf(double x, double m, double s){
  return normalpdf(x, m, s) / (1 - normalcdf(x, m, s));
}

/* returns logarithmic normal probability density function */
double lognormalpdf(double x, double a, double b) {
  double z = (log(x) - a) / b;

  return exp(-z * z / 2) / (x * sqrt(2 * PI) * b);
}

/* returns logarithmic normal cumulated distribution function */
double lognormalcdf(double x, double a, double b) {
  double z = (log(x) - a) / b;

  if (x == 0)
    return 0;
  if (z >= 0)
    return 0.5 + 0.5 * gammacdf(z * z / 2, 0.5);
  else
    return 0.5 - 0.5 * gammacdf(z * z / 2, 0.5);
}

/* returns logarithmic normal instantaneous rate function using mu & sigma */
double lognormalhrf(double x, double a, double b){
  if ((x == 0.0) || (x > 70000))
    return 0.0;
  else
    return lognormalpdf(x, a, b) / (1.0 - lognormalcdf(x, a, b));
}

/********************************************************/
/* output functions                                     */
/********************************************************/

/* print a state in human readable form */
char* printstate(int s) {
  char* c;

  switch (s) {
  case HPM:
    c = "HPM"; break;
  case LPM:
    c = "LPM"; break;
  default:
    c = "NULL"; break;
  }

  return c;
}

/* print an emission in human readable form */
char* printemission(int e) {
  char* c;

  switch (e)
  {
  case WP:
    c = "WP"; break;
  case DP:
    c = "DP"; break;
  default:
    c = "NULL"; break;
  }

  return c;
}

/* print a proxel */
void printproxel(proxel *c) {
  int left = 0;
  int right = 0;
  
  if (c->left != NULL) {
    left = c->left->id;
  }

  if (c->right != NULL) {
    right = c->right->id;
  }
  
  int leaf = (c->left == NULL && c->right == NULL);
  
  printf("ID: %6i - %s - Age: %3d - Prob.: %7.5le - Leaf: %i (%i,%i)\n", c->id, printstate(c->s), c->tau1k, c->val, leaf, left, right);
}

/* print all proxels of a proxel tree */
void printtree(proxel *p) {
  if (p == NULL)
    return;
  printproxel(p);
  printtree(p->left);
  printtree(p->right);
}

void printemissionsequence(int kmax) {
  int k;
  printf("Emission Sequence:\n{ ");
  for (k = 1; k < kmax + 2; ++k) {
    printf("%i ", emsequence[k]);
  }
  printf("}\nLegend: WP = 0 | DP = 1\n\n");
}

/* print complete solution */
void plotsolution(int kmax) {
  int k;
  char* one = (e) ? printemission(WP) : printstate(HPM);
  char* two = (e) ? printemission(DP) : printstate(LPM);

  printf("%s/%s Probabilities:\n",one,two);
  for (k = 1; k <= kmax; k++) {
    printf("Time: %6.2f - %s-Prob.: %7.5le - %s-Prob.: %7.5le\n", k*dt, one, y[0][k], two, y[2][k]);
  }
  printf("\n");

  if (e) {
    printf("Sequence Probabilities:\n");
    for (k = 1; k <= kmax; k++) {
      printf("Time: %6.2f - %s-Prob.: %7.5le\n", k*dt, printemission(emsequence[k]), emsum[emsequence[k]][k]);
    }
    printf("\n");
  }
}

/* count leafs of a proxel tree */
int countleafs(proxel *p) {
  if (p == NULL) {
    return 0;
  }

  if (p->left == NULL && p->right == NULL) {
    return 1;
  } else {
    return countleafs(p->left) + countleafs(p->right);
  }
}

/* compute size of tree */
int size(proxel *p) {
  if (p == NULL) {
    return 0;
  }

  return size(p->left) + size(p->right) + 1;
}

/********************************************************/
/* proxel manipulation functions			                  */
/********************************************************/

/* compute unique id from proxel state */
int state2id(int s, int t1k, int t2k) {
  return(TAUMAX*(TAUMAX*s + t1k) + t2k);
}

/* returns a proxel from the tree */
proxel *getproxel()
{
  proxel *temp;
  proxel *old;
  int LEFT = 0, RIGHT = 1;
  int dir, cont = 1;

  if (root[1 - sw] == NULL)
    return(NULL);
  temp = root[1 - sw];
  old = temp;

  /* move down the tree to a leaf */
  while (cont == 1)
  {
    /* go right */
    if ((temp->right != NULL) && (temp->left == NULL))
    {
      old = temp;
      temp = temp->right;
      dir = RIGHT;
    }
    /* go left */
    else if ((temp->right == NULL) && (temp->left != NULL))
    {
      old = temp;
      temp = temp->left;
      dir = LEFT;
    }
    /* choose right/left at random */
    else if ((temp->right != NULL) && (temp->left != NULL))
    {
      if (rand() > RAND_MAX / 2)
      {
        old = temp;
        temp = temp->left;
        dir = LEFT;
      }
      else
      {
        old = temp;
        temp = temp->right;
        dir = RIGHT;
      }
    }
    else
      cont = 0;
  }
  if (temp == root[1 - sw])
    root[1 - sw] = NULL;
  else
  {
    if (dir == RIGHT)
      old->right = NULL;
    else
      old->left = NULL;
  }
  old = firstfree;
  firstfree = temp;
  temp->right = old;
  ccpcnt -= 1;
  return(temp);
}

/* get a fresh proxel and copy data into it */
proxel *insertproxel(int s, int tau1k, int tau2k, double val) {
  proxel *temp;

  /* create new proxel or grab one from free list */
  if (firstfree == NULL)
    temp = malloc(sizeof(proxel));
  else {
    temp = firstfree;
    firstfree = firstfree->right;
  }
  /* copy values */
  temp->id = state2id(s, tau1k, tau2k);
  temp->s = s;
  temp->tau1k = tau1k;
  temp->tau2k = tau2k;
  temp->val = val;
  ccpcnt += 1;
  if (maxccp < ccpcnt) {
    maxccp = ccpcnt;
    //printf("\n ccpcnt=%d",ccpcnt);
  }

  return(temp);
}

/* adds a new proxel to the tree */
void addproxel(int s, int tau1k, int tau2k, double val) {
  proxel *temp, *temp2;
  int cont = 1, id;

  /* Alarm! TAUMAX overstepped! */
  if (tau1k >= TAUMAX) {
    //  printf(">>> %3d %3d %3d %7.5le \n", s, tau1k, val, TAUMAX);
    tau1k = TAUMAX - 1;
  }

  /* New tree, add root */
  if (root[sw] == NULL) {
    root[sw] = insertproxel(s, tau1k, tau2k, val);
    root[sw]->left = NULL;
    root[sw]->right = NULL;
    return;
  }

  /* compute id of new proxel */
  id = state2id(s, tau1k, tau2k);

  /* Locate insertion point in tree */
  temp = root[sw];
  while (cont == 1) {
    if ((temp->left != NULL) && (id < temp->id))
      temp = temp->left;
    else
      if ((temp->right != NULL) && (id > temp->id))
        temp = temp->right;
      else
        cont = 0;
  }

  /* Insert left leaf into tree */
  if ((temp->left == NULL) && (id < temp->id)) {
    temp2 = insertproxel(s, tau1k, tau2k, val);
    temp->left = temp2;
    temp2->left = NULL;
    temp2->right = NULL;
    return;
  }

  /* Insert right leaf into tree */
  if ((temp->right == NULL) && (id > temp->id)) {
    temp2 = insertproxel(s, tau1k, tau2k, val);
    temp->right = temp2;
    temp2->left = NULL;
    temp2->right = NULL;
    return;
  }

  /* Proxels have the same id, just add their vals */
  if (id == temp->id) {
    temp->val += val;
    return;
  }
  printf("\n\n\n!!!!!! addproxel failed !!!!!\n\n\n");
}

/********************************************************/
/*	model specific distributions	                      */
/********************************************************/

/* INSTANTANEOUS RATE FUNCTION 1 */
double overheat(double age) {
  // x0 (fourth parameter) is probably zero (start/warmup time)
  return weibullhrf(age, 55, 4, 0);
}

double produce(double age) {
  return dethrf(age, 1);
}

/* INSTANTANEOUS RATE FUNCTION 2 */
double cooldown(double age) {
  return unihrf(age, 9, 11);
}

// t = time step
// from = state (HPM,LPM)
// to = state (HPM,LPM)
// p = probability of the current state
// o = desired output (WP,LP)
double emission(int t, int from, int to, double p, int o) {
  double result = p * em[to][o];
  emsum[o][t] += result;
  return result;
}

/********************************************************/
/*  main processing loop                                */
/********************************************************/

int main(int argc, char **argv) {
  int     k, j, kmax;
  proxel *currproxel;
  double  val, z, valhpm, vallpm;
  int     s, tau1k, tau2k;

  /* initialise the simulation */
  root[0] = NULL;
  root[1] = NULL;
  eerror = 0.0;
  totcnt = 0;
  maxccp = 0;
  double tmax = ENDTIME;
  dt = DELTA;
  e = EMISSION;

  if (e) {
    printf("Using Symbol Emission...\n\n");
  }
  else {
    printf("No Symbol Emission...\n\n");
  }

  kmax = (int)floor(tmax / dt + 0.5);
  TAUMAX = kmax;

  /* initialize the solution vector for each time step */
  for (k = 0; k < 3; k++) {
    y[k] = malloc(sizeof(double) * (kmax + 2));
    for (j = 0; j < kmax + 2; j++)
      y[k][j] = 0.0;
  }

  if (e) {
    for (k = 0; k < 3; k++) { /* rows */
      em[k] = malloc(sizeof(double) * 2);
      for (j = 0; j < 2; j++) { /* cols */
        em[k][j] = 0.0;
      }
    }
    em[HPM][WP] = 0.95;
    em[HPM][DP] = 0.05;
    em[LPM][WP] = 0.8;
    em[LPM][DP] = 0.2;
    printf("Emission Matrix:\nHPM->WP: %11.10f\nHPM->DP: %11.10f\nLPM->WP: %11.10f\nLPM->DP: %11.10f\n\n", em[HPM][WP], em[HPM][DP], em[LPM][WP], em[LPM][DP]);

    /* initialize the emission sum vector */
    for (k = 0; k < 2; k++) { /* cols */
      emsum[k] = malloc(sizeof(double) * (kmax + 2));
      for (j = 0; j < kmax + 2; j++) {
        emsum[k][j] = 0.0;
      }
    }

    /* initialize the emission sequence */
    emsequence = malloc(sizeof(int) * (kmax + 2));
    for (k = 1; k < kmax + 2; ++k) {
      if (k % 2 == 0) {
        emsequence[k] = WP;
      }
      else {
        emsequence[k] = DP;
      }
      emsequence[k] = WP;
    }
    printemissionsequence(kmax);

    /* initialize the most likely paths */
    for (k = 0; k < 5; k++) {
      empaths[k] = malloc(sizeof(int) * (kmax + 2));
      for (j = 0; j < kmax + 2; j++)
        empaths[k][j] = 0;
    }
  }

  /* set initial proxel */
  addproxel(HPM, 0, 0, 1.0);

  /* first loop: iteration over all time steps*/
  /* current model time is k*dt */
  for (k = 1; k < kmax + 2; k++) {

    /* print progress information
    if (k % 100 == 0)  {
      printf("Step %d\n", k);
      printf("Size of tree %d\n", size(root[sw]));
    } */
    
    sw = 1 - sw;

    /* second loop: iterating over all proxels of a time step */
    while (root[1 - sw] != NULL)
    {
      totcnt++;
      currproxel = getproxel();
      while ((currproxel->val < MINPROB) && (root[1 - sw] != NULL)) {
        val = currproxel->val;
        eerror += val;
        currproxel = getproxel();
      }
      val = currproxel->val;
      tau1k = currproxel->tau1k;
      tau2k = currproxel->tau2k;
      s = currproxel->s;
      y[s][k - 1] += val;

      /* create child proxels */
      switch (s) {
      case HPM:
        /* probability to overheat the machine */
        z = dt * overheat(tau1k*dt);
        if (z < 1.0) {
          if (e) {
            vallpm = emission(k, s, LPM, val*z, emsequence[k]);
            valhpm = emission(k, s, HPM, val*(1 - z), emsequence[k]);
          }
          else {
            vallpm = val*z;
            valhpm = val*(1 - z);
          }
          addproxel(LPM, 0, 0, vallpm);
          addproxel(HPM, tau1k + 1, 0, valhpm);
        }
        else {
          if (e) {
            vallpm = emission(k, s, LPM, val, emsequence[k]);
          }
          else {
            vallpm = val;
          }
          addproxel(LPM, 0, 0, vallpm);
        }
        break;
      case LPM:
        /* probability to cooldown the machine */
        z = dt * cooldown(tau1k*dt);
        if (z < 1.0) {
          if (e) {
            valhpm = emission(k, s, HPM, val*z, emsequence[k]);
            vallpm = emission(k, s, LPM, val*(1 - z), emsequence[k]);
          }
          else {
            valhpm = val*z;
            vallpm = val*(1 - z);
          }
          addproxel(HPM, 0, 0, valhpm);
          addproxel(LPM, tau1k + 1, 0, vallpm);
        }
        else {
          if (e) {
            valhpm = emission(k, s, HPM, val, emsequence[k]);
          }
          else {
            valhpm = val;
          } 
          addproxel(HPM, 0, 0, valhpm);
        }
        break;
      default:
        printf("something went wrong!");
        break;
      }
    }
  }

  /*
  printf("\n");
  printtree(root[sw]);
  printf("\n");
  */

  plotsolution(kmax);

  printf("Tree Size = %d\n", size(root[sw]));
  printf("Proxels (Max Concurrent) = %d\n", maxccp);
  printf("Proxels (Total) = %d\n", totcnt);
  printf("Leafs (Total) = %i\n", countleafs(root[sw]));
  printf("Accumulated Error = %7.5le\n", eerror);

  printf("\n"); // last carriage return before exit

  return(0);
}
