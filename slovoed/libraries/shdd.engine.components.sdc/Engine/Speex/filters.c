﻿/* Copyright (C) 2002 Jean-Marc Valin 
   File: filters.c
   Various analysis/synthesis filters

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

#include "filters.h"
#include "stack_alloc.h"
#include <math.h>
#include "misc.h"
#include "math_approx.h"
#include "ltp.h"

#ifdef FIXED_POINT
void bw_lpc(spx_word16_t gamma, spx_coef_t *lpc_in, spx_coef_t *lpc_out, int order)
{
   int i;
   spx_word16_t tmp=gamma;
   lpc_out[0] = lpc_in[0];
   for (i=1;i<order+1;i++)
   {
      lpc_out[i] = MULT16_16_P15(tmp,lpc_in[i]);
      tmp = MULT16_16_P15(tmp, gamma);
   }
}
#else
void bw_lpc(spx_word16_t gamma, spx_coef_t *lpc_in, spx_coef_t *lpc_out, int order)
{
   int i;
   float tmp=1;
   for (i=0;i<order+1;i++)
   {
      lpc_out[i] = tmp * lpc_in[i];
      tmp *= gamma;
   }
}

#endif


#ifdef FIXED_POINT

/* FIXME: These functions are ugly and probably introduce too much error */
void signal_mul(spx_sig_t *x, spx_sig_t *y, spx_word32_t scale, int len)
{
   int i;
   for (i=0;i<len;i++)
   {
      y[i] = SHL(MULT16_32_Q14(SHR(x[i],7),scale),7);
   }
}

void signal_div(spx_sig_t *x, spx_sig_t *y, spx_word32_t scale, int len)
{
   int i;
   spx_word16_t scale_1;

   scale = PSHR(scale, SIG_SHIFT);
   if (scale<2)
      scale_1 = 32767;
   else
      scale_1 = 32767/scale;
   for (i=0;i<len;i++)
   {
      y[i] = MULT16_32_Q15(scale_1,x[i]);
   }
}

#else

void signal_mul(spx_sig_t *x, spx_sig_t *y, spx_word32_t scale, int len)
{
   int i;
   for (i=0;i<len;i++)
      y[i] = scale*x[i];
}

void signal_div(spx_sig_t *x, spx_sig_t *y, spx_word32_t scale, int len)
{
   int i;
   float scale_1 = 1/scale;
   for (i=0;i<len;i++)
      y[i] = scale_1*x[i];
}
#endif



#ifdef FIXED_POINT

int normalize16(spx_sig_t *x, spx_word16_t *y, int max_scale, int len)
{
   int i;
   spx_sig_t max_val=1;
   int sig_shift;
   
   for (i=0;i<len;i++)
   {
      spx_sig_t tmp = x[i];
      if (tmp<0)
         tmp = -tmp;
      if (tmp >= max_val)
         max_val = tmp;
   }

   sig_shift=0;
   while (max_val>max_scale)
   {
      sig_shift++;
      max_val >>= 1;
   }

   for (i=0;i<len;i++)
      y[i] = SHR(x[i], sig_shift);
   
   return sig_shift;
}

spx_word16_t compute_rms(spx_sig_t *x, int len)
{
   int i;
   spx_word32_t sum=0;
   spx_sig_t max_val=1;
   int sig_shift;

   for (i=0;i<len;i++)
   {
      spx_sig_t tmp = x[i];
      if (tmp<0)
         tmp = -tmp;
      if (tmp > max_val)
         max_val = tmp;
   }

   sig_shift=0;
   while (max_val>16383)
   {
      sig_shift++;
      max_val >>= 1;
   }

   for (i=0;i<len;i+=4)
   {
      spx_word32_t sum2=0;
      spx_word16_t tmp;
      tmp = SHR(x[i],sig_shift);
      sum2 += MULT16_16(tmp,tmp);
      tmp = SHR(x[i+1],sig_shift);
      sum2 += MULT16_16(tmp,tmp);
      tmp = SHR(x[i+2],sig_shift);
      sum2 += MULT16_16(tmp,tmp);
      tmp = SHR(x[i+3],sig_shift);
      sum2 += MULT16_16(tmp,tmp);
      sum += SHR(sum2,6);
   }
   
   return SHR(SHL((spx_word32_t)spx_sqrt(1+DIV32(sum,len)),(sig_shift+3)),SIG_SHIFT);
}


void filter_mem2(spx_sig_t *x, spx_coef_t *num, spx_coef_t *den, spx_sig_t *y, int N, int ord, spx_mem_t *mem)
{
   int i,j;
   spx_sig_t xi,yi,nyi;

   for (i=0;i<N;i++)
   {
      int xh,xl,yh,yl;
      xi=SATURATE(x[i],805306368);
      yi = SATURATE(ADD32(xi, SHL(mem[0],2)),805306368);
      nyi = -yi;
      xh = xi>>15; xl=xi&0x00007fff; yh = yi>>15; yl=yi&0x00007fff; 
      for (j=0;j<ord-1;j++)
      {
         mem[j] = MAC16_32_Q15(MAC16_32_Q15(mem[j+1], num[j+1],xi), den[j+1],nyi);
      }
      mem[ord-1] = SUB32(MULT16_32_Q15(num[ord],xi), MULT16_32_Q15(den[ord],yi));
      y[i] = yi;
   }
}

void iir_mem2(spx_sig_t *x, spx_coef_t *den, spx_sig_t *y, int N, int ord, spx_mem_t *mem)
{
   int i,j;
   spx_word32_t xi,yi,nyi;

   for (i=0;i<N;i++)
   {
      int yh,yl;
      xi=SATURATE(x[i],805306368);
      yi = SATURATE(xi + (mem[0]<<2),805306368);
      nyi = -yi;
      yh = yi>>15; yl=yi&0x00007fff; 
      for (j=0;j<ord-1;j++)
      {
         mem[j] = MAC16_32_Q15(mem[j+1],den[j+1],nyi);
      }
      mem[ord-1] = - MULT16_32_Q15(den[ord],yi);
      y[i] = yi;
   }
}


void fir_mem2(spx_sig_t *x, spx_coef_t *num, spx_sig_t *y, int N, int ord, spx_mem_t *mem)
{
   int i,j;
   spx_word32_t xi,yi;

   for (i=0;i<N;i++)
   {
      int xh,xl;
      xi=SATURATE(x[i],805306368);
      yi = xi + (mem[0]<<2);
      xh = xi>>15; xl=xi&0x00007fff;
      for (j=0;j<ord-1;j++)
      {
         mem[j] = MAC16_32_Q15(mem[j+1], num[j+1],xi);
      }
      mem[ord-1] = MULT16_32_Q15(num[ord],xi);
      y[i] = SATURATE(yi,805306368);
   }

}

#else



spx_word16_t compute_rms(spx_sig_t *x, int len)
{
   int i;
   float sum=0;
   for (i=0;i<len;i++)
   {
      sum += x[i]*x[i];
   }
   return sqrt(.1+sum/len);
}

#ifdef _USE_SSE
#include "filters_sse.h"
#else


void filter_mem2(spx_sig_t *x, spx_coef_t *num, spx_coef_t *den, spx_sig_t *y, int N, int ord,  spx_mem_t *mem)
{
   int i,j;
   float xi,yi;
   for (i=0;i<N;i++)
   {
      xi=x[i];
      y[i] = num[0]*xi + mem[0];
      yi=y[i];
      for (j=0;j<ord-1;j++)
      {
         mem[j] = mem[j+1] + num[j+1]*xi - den[j+1]*yi;
      }
      mem[ord-1] = num[ord]*xi - den[ord]*yi;
   }
}


void iir_mem2(spx_sig_t *x, spx_coef_t *den, spx_sig_t *y, int N, int ord, spx_mem_t *mem)
{
   int i,j;
   for (i=0;i<N;i++)
   {
      y[i] = x[i] + mem[0];
      for (j=0;j<ord-1;j++)
      {
         mem[j] = mem[j+1] - den[j+1]*y[i];
      }
      mem[ord-1] = - den[ord]*y[i];
   }
}


#endif

void fir_mem2(spx_sig_t *x, spx_coef_t *num, spx_sig_t *y, int N, int ord, spx_mem_t *mem)
{
   int i,j;
   float xi;
   for (i=0;i<N;i++)
   {
      xi=x[i];
      y[i] = num[0]*xi + mem[0];
      for (j=0;j<ord-1;j++)
      {
         mem[j] = mem[j+1] + num[j+1]*xi;
      }
      mem[ord-1] = num[ord]*xi;
   }
}


#endif


void syn_percep_zero(spx_sig_t *xx, spx_coef_t *ak, spx_coef_t *awk1, spx_coef_t *awk2, spx_sig_t *y, int N, int ord, char *stack)
{
   int i;
   spx_mem_t *mem = PUSH(stack,ord, spx_mem_t);
   for (i=0;i<ord;i++)
     mem[i]=0;
   iir_mem2(xx, ak, y, N, ord, mem);
   for (i=0;i<ord;i++)
      mem[i]=0;
   filter_mem2(y, awk1, awk2, y, N, ord, mem);
}

void residue_percep_zero(spx_sig_t *xx, spx_coef_t *ak, spx_coef_t *awk1, spx_coef_t *awk2, spx_sig_t *y, int N, int ord, char *stack)
{
   int i;
   spx_mem_t *mem = PUSH(stack,ord, spx_mem_t);
   for (i=0;i<ord;i++)
      mem[i]=0;
   filter_mem2(xx, ak, awk1, y, N, ord, mem);
   for (i=0;i<ord;i++)
     mem[i]=0;
   fir_mem2(y, awk2, y, N, ord, mem);
}

void qmf_decomp(short *xx, const spx_word16_t *aa, spx_sig_t *y1, spx_sig_t *y2, int N, int M, spx_word16_t *mem, char *stack)
{
   int i,j,k,M2;
   spx_word16_t *a;
   spx_word16_t *x;
   spx_word16_t *x2;
   
   a = PUSH(stack, M, spx_word16_t);
   x = PUSH(stack, N+M-1, spx_word16_t);
   x2=x+M-1;
   M2=M>>1;
   for (i=0;i<M;i++)
      a[M-i-1]= aa[i];

   for (i=0;i<M-1;i++)
      x[i]=mem[M-i-2];
   for (i=0;i<N;i++)
      x[i+M-1]=SATURATE(PSHR(xx[i],1),16383);
   for (i=0,k=0;i<N;i+=2,k++)
   {
      y1[k]=0;
      y2[k]=0;
      for (j=0;j<M2;j++)
      {
         y1[k]+=SHR(MULT16_16(a[j],ADD16(x[i+j],x2[i-j])),1);
         y2[k]-=SHR(MULT16_16(a[j],SUB16(x[i+j],x2[i-j])),1);
         j++;
         y1[k]+=SHR(MULT16_16(a[j],ADD16(x[i+j],x2[i-j])),1);
         y2[k]+=SHR(MULT16_16(a[j],SUB16(x[i+j],x2[i-j])),1);
      }
   }
   for (i=0;i<M-1;i++)
     mem[i]=SATURATE(PSHR(xx[N-i-1],1),16383);
}


/* By segher */
void fir_mem_up(spx_sig_t *x, const spx_word16_t *a, spx_sig_t *y, int N, int M, spx_word32_t *mem, char *stack)
   /* assumptions:
      all odd x[i] are zero -- well, actually they are left out of the array now
      N and M are multiples of 4 */
{
   int i, j;
   spx_word16_t *xx;
   
   xx= PUSH(stack, M+N-1, spx_word16_t);

   for (i = 0; i < N/2; i++)
      xx[2*i] = SHR(x[N/2-1-i],SIG_SHIFT+1);
   for (i = 0; i < M - 1; i += 2)
      xx[N+i] = mem[i+1];

   for (i = 0; i < N; i += 4) {
      spx_sig_t y0, y1, y2, y3;
      spx_word16_t x0;

      y0 = y1 = y2 = y3 = 0;
      x0 = xx[N-4-i];

      for (j = 0; j < M; j += 4) {
         spx_word16_t x1;
         spx_word16_t a0, a1;

         a0 = a[j];
         a1 = a[j+1];
         x1 = xx[N-2+j-i];

         y0 += SHR(MULT16_16(a0, x1),1);
         y1 += SHR(MULT16_16(a1, x1),1);
         y2 += SHR(MULT16_16(a0, x0),1);
         y3 += SHR(MULT16_16(a1, x0),1);

         a0 = a[j+2];
         a1 = a[j+3];
         x0 = xx[N+j-i];

         y0 += SHR(MULT16_16(a0, x0),1);
         y1 += SHR(MULT16_16(a1, x0),1);
         y2 += SHR(MULT16_16(a0, x1),1);
         y3 += SHR(MULT16_16(a1, x1),1);
      }
      y[i] = y0;
      y[i+1] = y1;
      y[i+2] = y2;
      y[i+3] = y3;
   }

   for (i = 0; i < M - 1; i += 2)
      mem[i+1] = xx[i];
}



void comb_filter_mem_init (CombFilterMem *mem)
{
   mem->last_pitch=0;
   mem->last_pitch_gain[0]=mem->last_pitch_gain[1]=mem->last_pitch_gain[2]=0;
   mem->smooth_gain=1;
}

#ifdef FIXED_POINT
#define COMB_STEP 32767
#else
#define COMB_STEP 1.0
#endif

void comb_filter(
spx_sig_t *exc,          /*decoded excitation*/
spx_sig_t *new_exc,      /*enhanced excitation*/
spx_coef_t *ak,           /*LPC filter coefs*/
int p,               /*LPC order*/
int nsf,             /*sub-frame size*/
int pitch,           /*pitch period*/
spx_word16_t *pitch_gain,   /*pitch gain (3-tap)*/
float  comb_gain,    /*gain of comb filter*/
CombFilterMem *mem
)
{
   int i;
   spx_word16_t exc_energy=0, new_exc_energy=0;
   float gain;
   spx_word16_t step;
   spx_word16_t fact;
   spx_word16_t cgain;

   /*Compute excitation amplitude prior to enhancement*/
   exc_energy = compute_rms(exc, nsf);
   /*for (i=0;i<nsf;i++)
     exc_energy+=((float)exc[i])*exc[i];*/

   /*Some gain adjustment if pitch is too high or if unvoiced*/
   {
      float g=0;
      g = GAIN_SCALING_1*.5*(gain_3tap_to_1tap(pitch_gain)+gain_3tap_to_1tap(mem->last_pitch_gain));
      if (g>1.3)
         comb_gain*=1.3/g;
      if (g<.5)
         comb_gain*=2*g;
   }
   step = DIV32(COMB_STEP, nsf);
   fact=0;

#ifdef FIXED_POINT
   cgain = comb_gain*32768;
#else
   cgain = comb_gain;
#endif

   /*Apply pitch comb-filter (filter out noise between pitch harmonics)*/
   for (i=0;i<nsf;i++)
   {
      spx_word32_t exc1, exc2;

      fact += step;
      
      exc1 = SHL(MULT16_32_Q15(SHL(pitch_gain[0],7),exc[i-pitch+1]) +
                 MULT16_32_Q15(SHL(pitch_gain[1],7),exc[i-pitch]) +
                 MULT16_32_Q15(SHL(pitch_gain[2],7),exc[i-pitch-1]) , 2);
      exc2 = SHL(MULT16_32_Q15(SHL(mem->last_pitch_gain[0],7),exc[i-mem->last_pitch+1]) +
                 MULT16_32_Q15(SHL(mem->last_pitch_gain[1],7),exc[i-mem->last_pitch]) +
                 MULT16_32_Q15(SHL(mem->last_pitch_gain[2],7),exc[i-mem->last_pitch-1]),2);

      new_exc[i] = exc[i] + MULT16_32_Q15(cgain,MULT16_32_Q15(fact,exc1)  + MULT16_32_Q15(SUB16(COMB_STEP,fact), exc2));
   }

   mem->last_pitch_gain[0] = pitch_gain[0];
   mem->last_pitch_gain[1] = pitch_gain[1];
   mem->last_pitch_gain[2] = pitch_gain[2];
   mem->last_pitch = pitch;

   /*Amplitude after enhancement*/
   new_exc_energy = compute_rms(new_exc, nsf);

   /*Compute scaling factor and normalize energy*/
   gain = (exc_energy)/(.1+new_exc_energy);
   if (gain < .5)
      gain=.5;
   if (gain>.9999)
      gain=.9999;

#ifdef FIXED_POINT
   {
      spx_word16_t gg = gain*32768;
      for (i=0;i<nsf;i++)
   {
      mem->smooth_gain = MULT16_16_Q15(31457,mem->smooth_gain) + MULT16_16_Q15(1311,gg);
      new_exc[i] = MULT16_32_Q15(mem->smooth_gain, new_exc[i]);
   }
   }
#else
   for (i=0;i<nsf;i++)
   {
      mem->smooth_gain = .96*mem->smooth_gain + .04*gain;
      new_exc[i] *= mem->smooth_gain;
   }
#endif
}


