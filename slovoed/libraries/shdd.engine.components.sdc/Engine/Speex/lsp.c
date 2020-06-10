﻿/*---------------------------------------------------------------------------*\
Original copyright
	FILE........: AKSLSPD.C
	TYPE........: Turbo C
	COMPANY.....: Voicetronix
	AUTHOR......: David Rowe
	DATE CREATED: 24/2/93

Modified by Jean-Marc Valin

   This file contains functions for converting Linear Prediction
   Coefficients (LPC) to Line Spectral Pair (LSP) and back. Note that the
   LSP coefficients are not in radians format but in the x domain of the
   unit circle.

   Speex License:

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <math.h>
#include "lsp.h"
#include "stack_alloc.h"
#include "math_approx.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef NULL
#define NULL 0
#endif

#ifdef FIXED_POINT

#define C1 8192
#define C2 -4096
#define C3 340
#define C4 -10

static spx_word16_t spx_cos(spx_word16_t x)
{
   spx_word16_t x2;

   if (x<12868)
   {
      x2 = MULT16_16_P13(x,x);
      return ADD32(C1, MULT16_16_P13(x2, ADD32(C2, MULT16_16_P13(x2, ADD32(C3, MULT16_16_P13(C4, x2))))));
   } else {
      x = 25736-x;
      x2 = MULT16_16_P13(x,x);
      return SUB32(-C1, MULT16_16_P13(x2, ADD32(C2, MULT16_16_P13(x2, ADD32(C3, MULT16_16_P13(C4, x2))))));
      //return SUB32(-C1, MULT16_16_Q13(x2, ADD32(C2, MULT16_16_Q13(C3, x2))));
   }
}


#define FREQ_SCALE 16384

/*#define ANGLE2X(a) (32768*cos(((a)/8192.)))*/
#define ANGLE2X(a) (SHL(spx_cos(a),2))

/*#define X2ANGLE(x) (acos(.00006103515625*(x))*LSP_SCALING)*/
#define X2ANGLE(x) (spx_acos(x))

#else

#define C1 0.99940307
#define C2 -0.49558072
#define C3 0.03679168

#define FREQ_SCALE 1.
#define ANGLE2X(a) (cos(a))
#define X2ANGLE(x) (acos(x))

#endif


/*---------------------------------------------------------------------------*\

	FUNCTION....: cheb_poly_eva()

	AUTHOR......: David Rowe
	DATE CREATED: 24/2/93

    This function evaluates a series of Chebyshev polynomials

\*---------------------------------------------------------------------------*/

#ifdef FIXED_POINT

static spx_word32_t cheb_poly_eva(spx_word32_t *coef,spx_word16_t x,int m,char *stack)
/*  float coef[]  	coefficients of the polynomial to be evaluated 	*/
/*  float x   		the point where polynomial is to be evaluated 	*/
/*  int m 		order of the polynomial 			*/
{
    int i;
    spx_word16_t *T;
    spx_word32_t sum;
    int m2=m>>1;
    spx_word16_t *coefn;

    /*Prevents overflows*/
    if (x>16383)
       x = 16383;
    if (x<-16383)
       x = -16383;

    /* Allocate memory for Chebyshev series formulation */
    T=PUSH(stack, m2+1, spx_word16_t);
    coefn=PUSH(stack, m2+1, spx_word16_t);

    for (i=0;i<m2+1;i++)
    {
       coefn[i] = coef[i];
       /*printf ("%f ", coef[i]);*/
    }
    /*printf ("\n");*/

    /* Initialise values */
    T[0]=16384;
    T[1]=x;

    /* Evaluate Chebyshev series formulation using iterative approach  */
    /* Evaluate polynomial and return value also free memory space */
    sum = ADD32(coefn[m2], MULT16_16_P14(coefn[m2-1],x));
    /*x *= 2;*/
    for(i=2;i<=m2;i++)
    {
       T[i] = MULT16_16_Q13(x,T[i-1]) - T[i-2];
       sum = ADD32(sum, MULT16_16_P14(coefn[m2-i],T[i]));
       /*printf ("%f ", sum);*/
    }
    
    /*printf ("\n");*/
    return sum;
}
#else
static float cheb_poly_eva(spx_word32_t *coef,float x,int m,char *stack)
/*  float coef[]  	coefficients of the polynomial to be evaluated 	*/
/*  float x   		the point where polynomial is to be evaluated 	*/
/*  int m 		order of the polynomial 			*/
{
    int i;
    float *T,sum;
    int m2=m>>1;

    /* Allocate memory for Chebyshev series formulation */
    T=PUSH(stack, m2+1, float);

    /* Initialise values */
    T[0]=1;
    T[1]=x;

    /* Evaluate Chebyshev series formulation using iterative approach  */
    /* Evaluate polynomial and return value also free memory space */
    sum = coef[m2] + coef[m2-1]*x;
    x *= 2;
    for(i=2;i<=m2;i++)
    {
       T[i] = x*T[i-1] - T[i-2];
       sum += coef[m2-i] * T[i];
    }
    
    return sum;
}
#endif

/*---------------------------------------------------------------------------*\

	FUNCTION....: lpc_to_lsp()

	AUTHOR......: David Rowe
	DATE CREATED: 24/2/93

    This function converts LPC coefficients to LSP
    coefficients.

\*---------------------------------------------------------------------------*/

#ifdef FIXED_POINT
#define SIGN_CHANGE(a,b) (((a)&0x70000000)^((b)&0x70000000)||(b==0))
#else
#define SIGN_CHANGE(a,b) (((a)*(b))<0.0)
#endif


int lpc_to_lsp (spx_coef_t *a,int lpcrdr,spx_lsp_t *freq,int nb,spx_word16_t delta, char *stack)
/*  float *a 		     	lpc coefficients			*/
/*  int lpcrdr			order of LPC coefficients (10) 		*/
/*  float *freq 	      	LSP frequencies in the x domain       	*/
/*  int nb			number of sub-intervals (4) 		*/
/*  float delta			grid spacing interval (0.02) 		*/


{
    spx_word16_t temp_xr,xl,xr,xm=0;
    spx_word32_t psuml,psumr,psumm,temp_psumr/*,temp_qsumr*/;
    int i,j,m,flag,k;
    spx_word32_t *Q;                 	/* ptrs for memory allocation 		*/
    spx_word32_t *P;
    spx_word32_t *px;                	/* ptrs of respective P'(z) & Q'(z)	*/
    spx_word32_t *qx;
    spx_word32_t *p;
    spx_word32_t *q;
    spx_word32_t *pt;                	/* ptr used for cheb_poly_eval()
				whether P' or Q' 			*/
    int roots=0;              	/* DR 8/2/94: number of roots found 	*/
    flag = 1;                	/*  program is searching for a root when,
				1 else has found one 			*/
    m = lpcrdr/2;            	/* order of P'(z) & Q'(z) polynomials 	*/

    /* Allocate memory space for polynomials */
    Q = PUSH(stack, (m+1), spx_word32_t);
    P = PUSH(stack, (m+1), spx_word32_t);

    /* determine P'(z)'s and Q'(z)'s coefficients where
      P'(z) = P(z)/(1 + z^(-1)) and Q'(z) = Q(z)/(1-z^(-1)) */

    px = P;                      /* initialise ptrs 			*/
    qx = Q;
    p = px;
    q = qx;

#ifdef FIXED_POINT
    *px++ = LPC_SCALING;
    *qx++ = LPC_SCALING;
    for(i=1;i<=m;i++){
	*px++ = (a[i]+a[lpcrdr+1-i]) - *p++;
	*qx++ = (a[i]-a[lpcrdr+1-i]) + *q++;
    }
    px = P;
    qx = Q;
    for(i=0;i<m;i++)
    {
       /*if (fabs(*px)>=32768)
          speex_warning_int("px", *px);
       if (fabs(*qx)>=32768)
       speex_warning_int("qx", *qx);*/
       *px = (2+*px)>>2;
       *qx = (2+*qx)>>2;
       px++;
       qx++;
    }
    P[m] = PSHR(P[m],3);
    Q[m] = PSHR(Q[m],3);
#else
    *px++ = LPC_SCALING;
    *qx++ = LPC_SCALING;
    for(i=1;i<=m;i++){
	*px++ = (a[i]+a[lpcrdr+1-i]) - *p++;
	*qx++ = (a[i]-a[lpcrdr+1-i]) + *q++;
    }
    px = P;
    qx = Q;
    for(i=0;i<m;i++){
	*px = 2**px;
	*qx = 2**qx;
	 px++;
	 qx++;
    }
#endif

    px = P;             	/* re-initialise ptrs 			*/
    qx = Q;

    /* Search for a zero in P'(z) polynomial first and then alternate to Q'(z).
    Keep alternating between the two polynomials as each zero is found 	*/

    xr = 0;             	/* initialise xr to zero 		*/
    xl = FREQ_SCALE;               	/* start at point xl = 1 		*/


    for(j=0;j<lpcrdr;j++){
	if(j&1)            	/* determines whether P' or Q' is eval. */
	    pt = qx;
	else
	    pt = px;

	psuml = cheb_poly_eva(pt,xl,lpcrdr,stack);	/* evals poly. at xl 	*/
	flag = 1;
	while(flag && (xr >= -FREQ_SCALE)){
           spx_word16_t dd;
           /* Modified by JMV to provide smaller steps around x=+-1 */
#ifdef FIXED_POINT
           dd = MULT16_16_Q15(delta,(FREQ_SCALE - MULT16_16_Q14(MULT16_16_Q14(xl,xl),14000)));
           if (psuml<512 && psuml>-512)
              dd = PSHR(dd,1);
#else
           dd=delta*(1-.9*xl*xl);
           if (fabs(psuml)<.2)
              dd *= .5;
#endif
           xr = xl - dd;                        	/* interval spacing 	*/
	    psumr = cheb_poly_eva(pt,xr,lpcrdr,stack);/* poly(xl-delta_x) 	*/
	    temp_psumr = psumr;
	    temp_xr = xr;

    /* if no sign change increment xr and re-evaluate poly(xr). Repeat til
    sign change.
    if a sign change has occurred the interval is bisected and then
    checked again for a sign change which determines in which
    interval the zero lies in.
    If there is no sign change between poly(xm) and poly(xl) set interval
    between xm and xr else set interval between xl and xr and repeat till
    root is located within the specified limits 			*/

	    if(SIGN_CHANGE(psumr,psuml))
            {
		roots++;

		psumm=psuml;
		for(k=0;k<=nb;k++){
#ifdef FIXED_POINT
		    xm = ADD16(PSHR(xl,1),PSHR(xr,1));        	/* bisect the interval 	*/
#else
                    xm = .5*(xl+xr);        	/* bisect the interval 	*/
#endif
		    psumm=cheb_poly_eva(pt,xm,lpcrdr,stack);
		    /*if(psumm*psuml>0.)*/
		    if(!SIGN_CHANGE(psumm,psuml))
                    {
			psuml=psumm;
			xl=xm;
		    } else {
			psumr=psumm;
			xr=xm;
		    }
		}

	       /* once zero is found, reset initial interval to xr 	*/
	       freq[j] = X2ANGLE(xm);
	       xl = xm;
	       flag = 0;       		/* reset flag for next search 	*/
	    }
	    else{
		psuml=temp_psumr;
		xl=temp_xr;
	    }
	}
    }
    return(roots);
}


/*---------------------------------------------------------------------------*\

	FUNCTION....: lsp_to_lpc()

	AUTHOR......: David Rowe
	DATE CREATED: 24/2/93

    lsp_to_lpc: This function converts LSP coefficients to LPC
    coefficients.

\*---------------------------------------------------------------------------*/

#ifdef FIXED_POINT

void lsp_to_lpc(spx_lsp_t *freq,spx_coef_t *ak,int lpcrdr, char *stack)
/*  float *freq 	array of LSP frequencies in the x domain	*/
/*  float *ak 		array of LPC coefficients 			*/
/*  int lpcrdr  	order of LPC coefficients 			*/


{
    int i,j;
    spx_word32_t xout1,xout2,xin1,xin2;
    spx_word32_t *Wp;
    spx_word32_t *pw,*n1,*n2,*n3,*n4=NULL;
    spx_word16_t *freqn;
    int m = lpcrdr>>1;
    
    freqn = PUSH(stack, lpcrdr, spx_word16_t);
    for (i=0;i<lpcrdr;i++)
       freqn[i] = ANGLE2X(freq[i]);

    Wp = PUSH(stack, 4*m+2, spx_word32_t);
    pw = Wp;


    /* initialise contents of array */

    for(i=0;i<=4*m+1;i++){       	/* set contents of buffer to 0 */
	*pw++ = 0.0;
    }

    /* Set pointers up */

    pw = Wp;
    xin1 = 1048576;
    xin2 = 1048576;

    /* reconstruct P(z) and Q(z) by  cascading second order
      polynomials in form 1 - 2xz(-1) +z(-2), where x is the
      LSP coefficient */

    for(j=0;j<=lpcrdr;j++){
       int i2=0;
	for(i=0;i<m;i++,i2+=2){
	    n1 = pw+(i*4);
	    n2 = n1 + 1;
	    n3 = n2 + 1;
	    n4 = n3 + 1;
	    xout1 = ADD32(SUB32(xin1, MULT16_32_Q14(freqn[i2],*n1)), *n2);
            xout2 = ADD32(SUB32(xin2, MULT16_32_Q14(freqn[i2+1],*n3)), *n4);
	    *n2 = *n1;
	    *n4 = *n3;
	    *n1 = xin1;
	    *n3 = xin2;
	    xin1 = xout1;
	    xin2 = xout2;
	}
	xout1 = xin1 + *(n4+1);
	xout2 = xin2 - *(n4+2);
        /* FIXME: perhaps apply bandwidth expansion in case of overflow? */
        if (xout1 + xout2>256*32766)
           ak[j] = 32767;
        else if (xout1 + xout2 < -256*32767)
           ak[j] = -32768;
        else
           ak[j] = PSHR(ADD32(xout1,xout2),8);
	*(n4+1) = xin1;
	*(n4+2) = xin2;

	xin1 = 0.0;
	xin2 = 0.0;
    }
}
#else

void lsp_to_lpc(spx_lsp_t *freq,spx_coef_t *ak,int lpcrdr, char *stack)
/*  float *freq 	array of LSP frequencies in the x domain	*/
/*  float *ak 		array of LPC coefficients 			*/
/*  int lpcrdr  	order of LPC coefficients 			*/


{
    int i,j;
    float xout1,xout2,xin1,xin2;
    float *Wp;
    float *pw,*n1,*n2,*n3,*n4=NULL;
    int m = lpcrdr/2;

    Wp = PUSH(stack, 4*m+2, float);
    pw = Wp;

    /* initialise contents of array */

    for(i=0;i<=4*m+1;i++){       	/* set contents of buffer to 0 */
	*pw++ = 0.0;
    }

    /* Set pointers up */

    pw = Wp;
    xin1 = 1.0;
    xin2 = 1.0;

    /* reconstruct P(z) and Q(z) by  cascading second order
      polynomials in form 1 - 2xz(-1) +z(-2), where x is the
      LSP coefficient */

    for(j=0;j<=lpcrdr;j++){
       int i2=0;
	for(i=0;i<m;i++,i2+=2){
	    n1 = pw+(i*4);
	    n2 = n1 + 1;
	    n3 = n2 + 1;
	    n4 = n3 + 1;
	    xout1 = xin1 - 2*(ANGLE2X(freq[i2])) * *n1 + *n2;
	    xout2 = xin2 - 2*(ANGLE2X(freq[i2+1])) * *n3 + *n4;
	    *n2 = *n1;
	    *n4 = *n3;
	    *n1 = xin1;
	    *n3 = xin2;
	    xin1 = xout1;
	    xin2 = xout2;
	}
	xout1 = xin1 + *(n4+1);
	xout2 = xin2 - *(n4+2);
	ak[j] = (xout1 + xout2)*0.5;
	*(n4+1) = xin1;
	*(n4+2) = xin2;

	xin1 = 0.0;
	xin2 = 0.0;
    }

}
#endif


#ifdef FIXED_POINT

/*Makes sure the LSPs are stable*/
void lsp_enforce_margin(spx_lsp_t *lsp, int len, spx_word16_t margin)
{
   int i;
   spx_word16_t m = margin;
   spx_word16_t m2 = 25736-margin;
  
   if (lsp[0]<m)
      lsp[0]=m;
   if (lsp[len-1]>m2)
      lsp[len-1]=m2;
   for (i=1;i<len-1;i++)
   {
      if (lsp[i]<lsp[i-1]+m)
         lsp[i]=lsp[i-1]+m;

      if (lsp[i]>lsp[i+1]-m)
         lsp[i]= SHR(lsp[i],1) + SHR(lsp[i+1]-m,1);
   }
}


void lsp_interpolate(spx_lsp_t *old_lsp, spx_lsp_t *new_lsp, spx_lsp_t *interp_lsp, int len, int subframe, int nb_subframes)
{
   int i;
   spx_word16_t tmp = DIV32_16(SHL(1 + subframe,14),nb_subframes);
   spx_word16_t tmp2 = 16384-tmp;
   for (i=0;i<len;i++)
   {
      interp_lsp[i] = MULT16_16_P14(tmp2,old_lsp[i]) + MULT16_16_P14(tmp,new_lsp[i]);
   }
}

#else

/*Makes sure the LSPs are stable*/
void lsp_enforce_margin(spx_lsp_t *lsp, int len, spx_word16_t margin)
{
   int i;
   if (lsp[0]<LSP_SCALING*margin)
      lsp[0]=LSP_SCALING*margin;
   if (lsp[len-1]>LSP_SCALING*(M_PI-margin))
      lsp[len-1]=LSP_SCALING*(M_PI-margin);
   for (i=1;i<len-1;i++)
   {
      if (lsp[i]<lsp[i-1]+LSP_SCALING*margin)
         lsp[i]=lsp[i-1]+LSP_SCALING*margin;

      if (lsp[i]>lsp[i+1]-LSP_SCALING*margin)
         lsp[i]= .5* (lsp[i] + lsp[i+1]-LSP_SCALING*margin);
   }
}


void lsp_interpolate(spx_lsp_t *old_lsp, spx_lsp_t *new_lsp, spx_lsp_t *interp_lsp, int len, int subframe, int nb_subframes)
{
   int i;
   float tmp = (1.0 + subframe)/nb_subframes;
   for (i=0;i<len;i++)
   {
      interp_lsp[i] = (1-tmp)*old_lsp[i] + tmp*new_lsp[i];
   }
}

#endif


