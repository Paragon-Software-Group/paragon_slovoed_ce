﻿/* Copyright (C) 2002 Jean-Marc Valin 
   File: sb_celp.c

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
#include "sb_celp.h"
#include "stdlib.h"
#include "filters.h"
#include "lpc.h"
#include "lsp.h"
#include "stack_alloc.h"
#include "cb_search.h"
#include "quant_lsp.h"
#include "vq.h"
#include "ltp.h"
#include "misc.h"

#include <stdio.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#define sqr(x) ((x)*(x))

#define SUBMODE(x) st->submodes[st->submodeID]->x

#ifdef FIXED_POINT
const spx_word16_t gc_quant_bound[16] = {125, 164, 215, 282, 370, 484, 635, 832, 1090, 1428, 1871, 2452, 3213, 4210, 5516, 7228};
#define LSP_MARGIN 410
#define LSP_DELTA1 6553
#define LSP_DELTA2 1638

#else

#define LSP_MARGIN .05
#define LSP_DELTA1 .2
#define LSP_DELTA2 .05

#endif

#define QMF_ORDER 64

#ifdef FIXED_POINT
const static spx_word16_t h0[64] = {2, -7, -7, 18, 15, -39, -25, 75, 35, -130, -41, 212, 38, -327, -17, 483, -32, -689, 124, 956, -283, -1307, 543, 1780, -973, -2467, 1733, 3633, -3339, -6409, 9059, 30153, 30153, 9059, -6409, -3339, 3633, 1733, -2467, -973, 1780, 543, -1307, -283, 956, 124, -689, -32, 483, -17, -327, 38, 212, -41, -130, 35, 75, -25, -39, 15, 18, -7, -7, 2};

const static spx_word16_t h1[64] = {2, 7, -7, -18, 15, 39, -25, -75, 35, 130, -41, -212, 38, 327, -17, -483, -32, 689, 124, -956, -283, 1307, 543, -1780, -973, 2467, 1733, -3633, -3339, 6409, 9059, -30153, 30153, -9059, -6409, 3339, 3633, -1733, -2467, 973, 1780, -543, -1307, 283, 956, -124, -689, 32, 483, 17, -327, -38, 212, 41, -130, -35, 75, 25, -39, -15, 18, 7, -7, -2};


#else
const static float h0[64] = {
   3.596189e-05, -0.0001123515,
   -0.0001104587, 0.0002790277,
   0.0002298438, -0.0005953563,
   -0.0003823631, 0.00113826,
   0.0005308539, -0.001986177,
   -0.0006243724, 0.003235877,
   0.0005743159, -0.004989147,
   -0.0002584767, 0.007367171,
   -0.0004857935, -0.01050689,
   0.001894714, 0.01459396,
   -0.004313674, -0.01994365,
   0.00828756, 0.02716055,
   -0.01485397, -0.03764973,
   0.026447, 0.05543245,
   -0.05095487, -0.09779096,
   0.1382363, 0.4600981,
   0.4600981, 0.1382363,
   -0.09779096, -0.05095487,
   0.05543245, 0.026447,
   -0.03764973, -0.01485397,
   0.02716055, 0.00828756,
   -0.01994365, -0.004313674,
   0.01459396, 0.001894714,
   -0.01050689, -0.0004857935,
   0.007367171, -0.0002584767,
   -0.004989147, 0.0005743159,
   0.003235877, -0.0006243724,
   -0.001986177, 0.0005308539,
   0.00113826, -0.0003823631,
   -0.0005953563, 0.0002298438,
   0.0002790277, -0.0001104587,
   -0.0001123515, 3.596189e-05
};

const static float h1[64] = {
   3.596189e-05, 0.0001123515,
   -0.0001104587, -0.0002790277,
   0.0002298438, 0.0005953563,
   -0.0003823631, -0.00113826,
   0.0005308539, 0.001986177,
   -0.0006243724, -0.003235877,
   0.0005743159, 0.004989147,
   -0.0002584767, -0.007367171,
   -0.0004857935, 0.01050689,
   0.001894714, -0.01459396,
   -0.004313674, 0.01994365,
   0.00828756, -0.02716055,
   -0.01485397, 0.03764973,
   0.026447, -0.05543245,
   -0.05095487, 0.09779096,
   0.1382363, -0.4600981,
   0.4600981, -0.1382363,
   -0.09779096, 0.05095487,
   0.05543245, -0.026447,
   -0.03764973, 0.01485397,
   0.02716055, -0.00828756,
   -0.01994365, 0.004313674,
   0.01459396, -0.001894714,
   -0.01050689, 0.0004857935,
   0.007367171, 0.0002584767,
   -0.004989147, -0.0005743159,
   0.003235877, 0.0006243724,
   -0.001986177, -0.0005308539,
   0.00113826, 0.0003823631,
   -0.0005953563, -0.0002298438,
   0.0002790277, 0.0001104587,
   -0.0001123515, -3.596189e-05
};
#endif

static void mix_and_saturate(spx_word32_t *y0, spx_word32_t *y1, short *out, int len)
{
   int i;
   for (i=0;i<len;i++)
   {
      spx_word32_t tmp;
#ifdef FIXED_POINT
      tmp=PSHR(y0[i]-y1[i],SIG_SHIFT-1);
#else
      tmp=2*(y0[i]-y1[i]);
#endif
      if (tmp>32767)
         out[i] = 32767;
      else if (tmp<-32767)
         out[i] = -32767;
      else
         out[i] = tmp;
   }
}

#ifndef TOMATO_DECODE_ONLY
static int k = 0;
void *sb_encoder_init(const SpeexMode *m)
{
   int i;
   SBEncState *st;
   SpeexSBMode *mode;

   st = (SBEncState*)speex_alloc(sizeof(SBEncState)+10000*sizeof(spx_sig_t));
   st->mode = m;
   mode = (SpeexSBMode*)m->mode;

   st->stack = ((char*)st) + sizeof(SBEncState);

   st->st_low = speex_encoder_init(mode->nb_mode);
   st->full_frame_size = 2*mode->frameSize;
   st->frame_size = mode->frameSize;
   st->subframeSize = mode->subframeSize;
   st->nbSubframes = mode->frameSize/mode->subframeSize;
   st->windowSize = st->frame_size*3/2;
   st->lpcSize=mode->lpcSize;
   st->bufSize=mode->bufSize;

   st->encode_submode = 1;
   st->submodes=mode->submodes;
   st->submodeSelect = st->submodeID=mode->defaultSubmode;
   
   i=9;
   speex_encoder_ctl(st->st_low, SPEEX_SET_QUALITY, &i);

   st->lag_factor = mode->lag_factor;
   st->lpc_floor = mode->lpc_floor;
   st->gamma1=GAMMA_SCALING*mode->gamma1;
   st->gamma2=GAMMA_SCALING*mode->gamma2;
   st->first=1;

   st->x0d=PUSH(st->stack, st->frame_size, spx_sig_t);
   st->x1d=PUSH(st->stack, st->frame_size, spx_sig_t);
   st->high=PUSH(st->stack, st->full_frame_size, spx_sig_t);
   st->y0=PUSH(st->stack, st->full_frame_size, spx_sig_t);
   st->y1=PUSH(st->stack, st->full_frame_size, spx_sig_t);

   st->h0_mem=PUSH(st->stack, QMF_ORDER, spx_word16_t);
   st->h1_mem=PUSH(st->stack, QMF_ORDER, spx_word16_t);
   st->g0_mem=PUSH(st->stack, QMF_ORDER, spx_word32_t);
   st->g1_mem=PUSH(st->stack, QMF_ORDER, spx_word32_t);

   st->buf=PUSH(st->stack, st->windowSize, spx_sig_t);
   st->excBuf=PUSH(st->stack, st->bufSize, spx_sig_t);
   st->exc = st->excBuf + st->bufSize - st->windowSize;

   st->res=PUSH(st->stack, st->frame_size, spx_sig_t);
   st->sw=PUSH(st->stack, st->frame_size, spx_sig_t);
   st->target=PUSH(st->stack, st->frame_size, spx_sig_t);
   /*Asymmetric "pseudo-Hamming" window*/
   {
      int part1, part2;
      part1 = st->subframeSize*7/2;
      part2 = st->subframeSize*5/2;
      st->window = PUSH(st->stack, st->windowSize, spx_word16_t);
      for (i=0;i<part1;i++)
         st->window[i]=(spx_word16_t)(SIG_SCALING*(.54-.46*cos(M_PI*i/part1)));
      for (i=0;i<part2;i++)
         st->window[part1+i]=(spx_word16_t)(SIG_SCALING*(.54+.46*cos(M_PI*i/part2)));
   }

   st->lagWindow = PUSH(st->stack, st->lpcSize+1, spx_word16_t);
   for (i=0;i<st->lpcSize+1;i++)
      st->lagWindow[i]=16384*exp(-.5*sqr(2*M_PI*st->lag_factor*i));

   st->autocorr = PUSH(st->stack, st->lpcSize+1, spx_word16_t);
   st->lpc = PUSH(st->stack, st->lpcSize+1, spx_coef_t);
   st->bw_lpc1 = PUSH(st->stack, st->lpcSize+1, spx_coef_t);
   st->bw_lpc2 = PUSH(st->stack, st->lpcSize+1, spx_coef_t);
   st->lsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->qlsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->old_lsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->old_qlsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->interp_lsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->interp_qlsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->interp_lpc = PUSH(st->stack, st->lpcSize+1, spx_coef_t);
   st->interp_qlpc = PUSH(st->stack, st->lpcSize+1, spx_coef_t);
   st->pi_gain = PUSH(st->stack, st->nbSubframes, spx_word32_t);

   st->mem_sp = PUSH(st->stack, st->lpcSize, spx_mem_t);
   st->mem_sp2 = PUSH(st->stack, st->lpcSize, spx_mem_t);
   st->mem_sw = PUSH(st->stack, st->lpcSize, spx_mem_t);

   st->vbr_quality = 8;
   st->vbr_enabled = 0;
   st->vad_enabled = 0;
   st->abr_enabled = 0;
   st->relative_quality=0;

   st->complexity=2;
   speex_decoder_ctl(st->st_low, SPEEX_GET_SAMPLING_RATE, &st->sampling_rate);
   st->sampling_rate*=2;

   return st;
}

void sb_encoder_destroy(void *state)
{
   SBEncState *st=(SBEncState*)state;

   speex_encoder_destroy(st->st_low);

   speex_free(st);
}


int sb_encode(void *state, short *in, SpeexBits *bits)
{
   SBEncState *st;
   int i, roots, sub;
   char *stack;
   spx_mem_t *mem;
   spx_sig_t *innov, *syn_resp;
   spx_word32_t *low_pi_gain;
   spx_sig_t *low_exc, *low_innov;
   SpeexSBMode *mode;
   int dtx;

   st = (SBEncState*)state;
   stack=st->stack;
   mode = (SpeexSBMode*)(st->mode->mode);

   {
      short *low = PUSH(stack, st->frame_size, short);

      /* Compute the two sub-bands by filtering with h0 and h1*/
      qmf_decomp(in, h0, st->x0d, st->x1d, st->full_frame_size, QMF_ORDER, st->h0_mem, stack);
      
      for (i=0;i<st->frame_size;i++)
         low[i] = PSHR(st->x0d[i],SIG_SHIFT);
      
      /* Encode the narrowband part*/
      speex_encode(st->st_low, low, bits);

      for (i=0;i<st->frame_size;i++)
         st->x0d[i] = SHL(low[i],SIG_SHIFT);
   }
   /* High-band buffering / sync with low band */
   for (i=0;i<st->windowSize-st->frame_size;i++)
      st->high[i] = st->high[st->frame_size+i];
   for (i=0;i<st->frame_size;i++)
      st->high[st->windowSize-st->frame_size+i]=SATURATE(st->x1d[i],536854528);

   speex_move(st->excBuf, st->excBuf+st->frame_size, (st->bufSize-st->frame_size)*sizeof(spx_sig_t));


   low_pi_gain = PUSH(stack, st->nbSubframes, spx_word32_t);
   low_exc = PUSH(stack, st->frame_size, spx_sig_t);
   low_innov = PUSH(stack, st->frame_size, spx_sig_t);
   speex_encoder_ctl(st->st_low, SPEEX_GET_PI_GAIN, low_pi_gain);
   speex_encoder_ctl(st->st_low, SPEEX_GET_EXC, low_exc);
   speex_encoder_ctl(st->st_low, SPEEX_GET_INNOV, low_innov);
   
   speex_encoder_ctl(st->st_low, SPEEX_GET_LOW_MODE, &dtx);

   if (dtx==0)
      dtx=1;
   else
      dtx=0;

   {
      spx_word16_t *w_sig;
      w_sig = PUSH(stack, st->windowSize, spx_word16_t);
      /* Window for analysis */
      for (i=0;i<st->windowSize;i++)
         w_sig[i] = SHR(MULT16_16(SHR((spx_word32_t)(st->high[i]),SIG_SHIFT),st->window[i]),SIG_SHIFT);

      /* Compute auto-correlation */
      _spx_autocorr(w_sig, st->autocorr, st->lpcSize+1, st->windowSize);
   }

   st->autocorr[0] = (spx_word16_t)(st->autocorr[0]*st->lpc_floor); /* Noise floor in auto-correlation domain */

   /* Lag windowing: equivalent to filtering in the power-spectrum domain */
   for (i=0;i<st->lpcSize+1;i++)
      st->autocorr[i] = MULT16_16_Q14(st->autocorr[i],st->lagWindow[i]);

   /* Levinson-Durbin */
   _spx_lpc(st->lpc+1, st->autocorr, st->lpcSize);
   st->lpc[0] = (spx_coef_t)LPC_SCALING;

   /* LPC to LSPs (x-domain) transform */
   roots=lpc_to_lsp (st->lpc, st->lpcSize, st->lsp, 15, LSP_DELTA1, stack);
   if (roots!=st->lpcSize)
   {
      roots = lpc_to_lsp (st->lpc, st->lpcSize, st->lsp, 11, LSP_DELTA2, stack);
      if (roots!=st->lpcSize) {
         /*If we can't find all LSP's, do some damage control and use a flat filter*/
         for (i=0;i<st->lpcSize;i++)
         {
            st->lsp[i]=M_PI*((float)(i+1))/(st->lpcSize+1);
         }
      }
   }

   /* VBR code */
   if ((st->vbr_enabled || st->vad_enabled) && !dtx)
   {
      float e_low=0, e_high=0;
      float ratio;
      if (st->abr_enabled)
      {
         float qual_change=0;
         if (st->abr_drift2 * st->abr_drift > 0)
         {
            /* Only adapt if long-term and short-term drift are the same sign */
            qual_change = -.00001*st->abr_drift/(1+st->abr_count);
            if (qual_change>.1)
               qual_change=.1;
            if (qual_change<-.1)
               qual_change=-.1;
         }
         st->vbr_quality += qual_change;
         if (st->vbr_quality>10)
            st->vbr_quality=10;
         if (st->vbr_quality<0)
            st->vbr_quality=0;
      }


      /*FIXME: Are the two signals (low, high) in sync? */
      e_low = compute_rms(st->x0d, st->frame_size);
      e_high = compute_rms(st->high, st->frame_size);
      ratio = 2*log((1+e_high)/(1+e_low));
      
      speex_encoder_ctl(st->st_low, SPEEX_GET_RELATIVE_QUALITY, &st->relative_quality);
      if (ratio<-4)
         ratio=-4;
      if (ratio>2)
         ratio=2;
      /*if (ratio>-2)*/
      if (st->vbr_enabled) 
      {
         int modeid;
         modeid = mode->nb_modes-1;
         st->relative_quality+=1.0*(ratio+2);
	 if (st->relative_quality<-1)
            st->relative_quality=-1;
         while (modeid)
         {
            int v1;
            float thresh;
            v1=(int)floor(st->vbr_quality);
            if (v1==10)
               thresh = mode->vbr_thresh[modeid][v1];
            else
               thresh = (st->vbr_quality-v1)   * mode->vbr_thresh[modeid][v1+1] + 
                        (1+v1-st->vbr_quality) * mode->vbr_thresh[modeid][v1];
            if (st->relative_quality >= thresh)
               break;
            modeid--;
         }
         speex_encoder_ctl(state, SPEEX_SET_HIGH_MODE, &modeid);
         if (st->abr_enabled)
         {
            int bitrate;
            speex_encoder_ctl(state, SPEEX_GET_BITRATE, &bitrate);
            st->abr_drift+=(bitrate-st->abr_enabled);
            st->abr_drift2 = .95*st->abr_drift2 + .05*(bitrate-st->abr_enabled);
            st->abr_count += 1.0;
         }

      } else {
         /* VAD only */
         int modeid;
         if (st->relative_quality<2.0)
            modeid=1;
         else
            modeid=st->submodeSelect;
         /*speex_encoder_ctl(state, SPEEX_SET_MODE, &mode);*/
         st->submodeID=modeid;

      }
      /*fprintf (stderr, "%f %f\n", ratio, low_qual);*/
   }

   if (st->encode_submode)
   {
      speex_bits_pack(bits, 1, 1);
      if (dtx)
         speex_bits_pack(bits, 0, SB_SUBMODE_BITS);
      else
         speex_bits_pack(bits, st->submodeID, SB_SUBMODE_BITS);
   }

   /* If null mode (no transmission), just set a couple things to zero*/
   if (dtx || st->submodes[st->submodeID] == NULL)
   {
      for (i=0;i<st->frame_size;i++)
         st->exc[i]=st->sw[i]=VERY_SMALL;

      for (i=0;i<st->lpcSize;i++)
         st->mem_sw[i]=0;
      st->first=1;

      /* Final signal synthesis from excitation */
      iir_mem2(st->exc, st->interp_qlpc, st->high, st->frame_size, st->lpcSize, st->mem_sp);

#ifndef RELEASE

      /* Reconstruct the original */
      fir_mem_up(st->x0d, h0, st->y0, st->full_frame_size, QMF_ORDER, st->g0_mem, stack);
      fir_mem_up(st->high, h1, st->y1, st->full_frame_size, QMF_ORDER, st->g1_mem, stack);

      for (i=0;i<st->full_frame_size;i++)
         in[i]=2*(st->y0[i]-st->y1[i]) / SIG_SCALING;
#endif

      if (dtx)
         return 0;
      else
         return 1;
   }


   /* LSP quantization */
   SUBMODE(lsp_quant)(st->lsp, st->qlsp, st->lpcSize, bits);   

   if (st->first)
   {
      for (i=0;i<st->lpcSize;i++)
         st->old_lsp[i] = st->lsp[i];
      for (i=0;i<st->lpcSize;i++)
         st->old_qlsp[i] = st->qlsp[i];
   }
   
   mem=PUSH(stack, st->lpcSize, spx_mem_t);
   syn_resp=PUSH(stack, st->subframeSize, spx_sig_t);
   innov = PUSH(stack, st->subframeSize, spx_sig_t);

   for (sub=0;sub<st->nbSubframes;sub++)
   {
      spx_sig_t *exc, *sp, *res, *target, *sw;
      spx_word16_t filter_ratio;
      int offset;
      spx_word32_t rl, rh;
      spx_word16_t eh=0;

      offset = st->subframeSize*sub;
      sp=st->high+offset;
      exc=st->exc+offset;
      res=st->res+offset;
      target=st->target+offset;
      sw=st->sw+offset;
      
      /* LSP interpolation (quantized and unquantized) */
      lsp_interpolate(st->old_lsp, st->lsp, st->interp_lsp, st->lpcSize, sub, st->nbSubframes);
      lsp_interpolate(st->old_qlsp, st->qlsp, st->interp_qlsp, st->lpcSize, sub, st->nbSubframes);

      lsp_enforce_margin(st->interp_lsp, st->lpcSize, LSP_MARGIN);
      lsp_enforce_margin(st->interp_qlsp, st->lpcSize, LSP_MARGIN);

      lsp_to_lpc(st->interp_lsp, st->interp_lpc, st->lpcSize,stack);
      lsp_to_lpc(st->interp_qlsp, st->interp_qlpc, st->lpcSize, stack);

      bw_lpc(st->gamma1, st->interp_lpc, st->bw_lpc1, st->lpcSize);
      bw_lpc(st->gamma2, st->interp_lpc, st->bw_lpc2, st->lpcSize);

      /* Compute mid-band (4000 Hz for wideband) response of low-band and high-band
         filters */
      st->pi_gain[sub]=LPC_SCALING;
      rh = LPC_SCALING;
      for (i=1;i<=st->lpcSize;i+=2)
      {
         rh += st->interp_qlpc[i+1] - st->interp_qlpc[i];
         st->pi_gain[sub] += st->interp_qlpc[i] + st->interp_qlpc[i+1];
      }
      
      rl = low_pi_gain[sub];
#ifdef FIXED_POINT
      filter_ratio=DIV32_16(SHL(rl+82,2),SHR(82+rh,5));
#else
      filter_ratio=(rl+.01)/(rh+.01);
#endif
      
      /* Compute "real excitation" */
      fir_mem2(sp, st->interp_qlpc, exc, st->subframeSize, st->lpcSize, st->mem_sp2);
      /* Compute energy of low-band and high-band excitation */

      eh = compute_rms(exc, st->subframeSize);

      if (!SUBMODE(innovation_quant)) {/* 1 for spectral folding excitation, 0 for stochastic */
         float g;
         spx_word16_t el;
         el = compute_rms(low_innov+offset, st->subframeSize);

         /* Gain to use if we want to use the low-band excitation for high-band */
         g=eh/(.01+el);

#ifdef FIXED_POINT
         g *= filter_ratio/128.;
#else
         g *= filter_ratio;
#endif
         /*print_vec(&g, 1, "gain factor");*/
         /* Gain quantization */
         {
            int quant = (int) floor(.5 + 10 + 8.0 * log((g+.0001)));
            /*speex_warning_int("tata", quant);*/
            if (quant<0)
               quant=0;
            if (quant>31)
               quant=31;
            speex_bits_pack(bits, quant, 5);
         }

      } else {
         spx_word16_t gc;
         spx_word32_t scale;
         spx_word16_t el;
         el = compute_rms(low_exc+offset, st->subframeSize);

         gc = DIV32_16(MULT16_16(filter_ratio,1+eh),1+el);

         /* This is a kludge that cleans up a historical bug */
         if (st->subframeSize==80)
            gc *= 0.70711;
         /*printf ("%f %f %f %f\n", el, eh, filter_ratio, gc);*/
#ifdef FIXED_POINT
         {
            int qgc = scal_quant(gc, gc_quant_bound, 16);
            speex_bits_pack(bits, qgc, 4);
            gc = MULT16_32_Q15(28626,gc_quant_bound[qgc]);
         }
#else
         {
            int qgc = (int)floor(.5+3.7*(log(gc)+0.15556));
            if (qgc<0)
               qgc=0;
            if (qgc>15)
               qgc=15;
            speex_bits_pack(bits, qgc, 4);
            gc = exp((1/3.7)*qgc-0.15556);
         }         
#endif
         if (st->subframeSize==80)
            gc *= 1.4142;

         scale = SHL(MULT16_16(DIV32_16(SHL(gc,SIG_SHIFT-4),filter_ratio),(1+el)),4);

         for (i=0;i<st->subframeSize;i++)
            exc[i]=VERY_SMALL;
         exc[0]=SIG_SCALING;
         syn_percep_zero(exc, st->interp_qlpc, st->bw_lpc1, st->bw_lpc2, syn_resp, st->subframeSize, st->lpcSize, stack);
         
         /* Reset excitation */
         for (i=0;i<st->subframeSize;i++)
            exc[i]=VERY_SMALL;
         
         /* Compute zero response (ringing) of A(z/g1) / ( A(z/g2) * Aq(z) ) */
         for (i=0;i<st->lpcSize;i++)
            mem[i]=st->mem_sp[i];
         iir_mem2(exc, st->interp_qlpc, exc, st->subframeSize, st->lpcSize, mem);

         for (i=0;i<st->lpcSize;i++)
            mem[i]=st->mem_sw[i];
         filter_mem2(exc, st->bw_lpc1, st->bw_lpc2, res, st->subframeSize, st->lpcSize, mem);

         /* Compute weighted signal */
         for (i=0;i<st->lpcSize;i++)
            mem[i]=st->mem_sw[i];
         filter_mem2(sp, st->bw_lpc1, st->bw_lpc2, sw, st->subframeSize, st->lpcSize, mem);

         /* Compute target signal */
         for (i=0;i<st->subframeSize;i++)
            target[i]=sw[i]-res[i];

         for (i=0;i<st->subframeSize;i++)
           exc[i]=0;

         signal_div(target, target, scale, st->subframeSize);

         /* Reset excitation */
         for (i=0;i<st->subframeSize;i++)
            innov[i]=0;

         /*print_vec(target, st->subframeSize, "\ntarget");*/
         SUBMODE(innovation_quant)(target, st->interp_qlpc, st->bw_lpc1, st->bw_lpc2, 
                                   SUBMODE(innovation_params), st->lpcSize, st->subframeSize, 
                                   innov, syn_resp, bits, stack, (st->complexity+1)>>1);
         /*print_vec(target, st->subframeSize, "after");*/

         signal_mul(innov, innov, scale, st->subframeSize);

         for (i=0;i<st->subframeSize;i++)
            exc[i] += innov[i];

         if (SUBMODE(double_codebook)) {
            char *tmp_stack=stack;
            spx_sig_t *innov2 = PUSH(tmp_stack, st->subframeSize, spx_sig_t);
            for (i=0;i<st->subframeSize;i++)
               innov2[i]=0;
            for (i=0;i<st->subframeSize;i++)
               target[i]*=2.5;
            SUBMODE(innovation_quant)(target, st->interp_qlpc, st->bw_lpc1, st->bw_lpc2, 
                                      SUBMODE(innovation_params), st->lpcSize, st->subframeSize, 
                                      innov2, syn_resp, bits, tmp_stack, (st->complexity+1)>>1);
            for (i=0;i<st->subframeSize;i++)
               innov2[i]*=scale*(1/2.5)/SIG_SCALING;
            for (i=0;i<st->subframeSize;i++)
               exc[i] += innov2[i];
         }

      }

      /*Keep the previous memory*/
      for (i=0;i<st->lpcSize;i++)
         mem[i]=st->mem_sp[i];
      /* Final signal synthesis from excitation */
      iir_mem2(exc, st->interp_qlpc, sp, st->subframeSize, st->lpcSize, st->mem_sp);
      
      /* Compute weighted signal again, from synthesized speech (not sure it's the right thing) */
      filter_mem2(sp, st->bw_lpc1, st->bw_lpc2, sw, st->subframeSize, st->lpcSize, st->mem_sw);
   }


#ifndef RELEASE

   /* Reconstruct the original */
   fir_mem_up(st->x0d, h0, st->y0, st->full_frame_size, QMF_ORDER, st->g0_mem, stack);
   fir_mem_up(st->high, h1, st->y1, st->full_frame_size, QMF_ORDER, st->g1_mem, stack);

   for (i=0;i<st->full_frame_size;i++)
      in[i]=2*(st->y0[i]-st->y1[i]) / SIG_SCALING;
#endif
   for (i=0;i<st->lpcSize;i++)
      st->old_lsp[i] = st->lsp[i];
   for (i=0;i<st->lpcSize;i++)
      st->old_qlsp[i] = st->qlsp[i];

   st->first=0;

   return 1;
}
#endif





void *sb_decoder_init(const SpeexMode *m)
{
   SBDecState *st;
   SpeexSBMode *mode;
   st = (SBDecState*)speex_alloc(sizeof(SBDecState)+6000*sizeof(spx_sig_t));
   st->mode = m;
   mode=(SpeexSBMode*)m->mode;

   st->encode_submode = 1;

   st->stack = ((char*)st) + sizeof(SBDecState);



   st->st_low = speex_decoder_init(mode->nb_mode);
   st->full_frame_size = 2*mode->frameSize;
   st->frame_size = mode->frameSize;
   st->subframeSize = mode->subframeSize;
   st->nbSubframes = mode->frameSize/mode->subframeSize;
   st->lpcSize=mode->lpcSize;
   speex_decoder_ctl(st->st_low, SPEEX_GET_SAMPLING_RATE, &st->sampling_rate);
   st->sampling_rate*=2;

   st->submodes=mode->submodes;
   st->submodeID=mode->defaultSubmode;

   st->first=1;


   st->x0d=PUSH(st->stack, st->frame_size, spx_sig_t);
   st->x1d=PUSH(st->stack, st->frame_size, spx_sig_t);
   st->high=PUSH(st->stack, st->full_frame_size, spx_sig_t);
   st->y0=PUSH(st->stack, st->full_frame_size, spx_sig_t);
   st->y1=PUSH(st->stack, st->full_frame_size, spx_sig_t);

   st->g0_mem=PUSH(st->stack, QMF_ORDER, spx_word32_t);
   st->g1_mem=PUSH(st->stack, QMF_ORDER, spx_word32_t);

   st->exc=PUSH(st->stack, st->frame_size, spx_sig_t);

   st->qlsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->old_qlsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->interp_qlsp = PUSH(st->stack, st->lpcSize, spx_lsp_t);
   st->interp_qlpc = PUSH(st->stack, st->lpcSize+1, spx_coef_t);

   st->pi_gain = PUSH(st->stack, st->nbSubframes, spx_word32_t);
   st->mem_sp = PUSH(st->stack, 2*st->lpcSize, spx_mem_t);
   
   st->lpc_enh_enabled=1;

   return st;
}

void sb_decoder_destroy(void *state)
{
   SBDecState *st;
   st = (SBDecState*)state;
   speex_decoder_destroy(st->st_low);

   speex_free(state);
}

static void sb_decode_lost(SBDecState *st, short *out, int dtx, char *stack)
{
   int i;
   spx_coef_t *awk1, *awk2, *awk3;
   int saved_modeid=0;

   if (dtx)
   {
      saved_modeid=st->submodeID;
      st->submodeID=1;
   } else {
      bw_lpc(GAMMA_SCALING*0.99, st->interp_qlpc, st->interp_qlpc, st->lpcSize);
   }

   st->first=1;
   
   awk1=PUSH(stack, st->lpcSize+1, spx_coef_t);
   awk2=PUSH(stack, st->lpcSize+1, spx_coef_t);
   awk3=PUSH(stack, st->lpcSize+1, spx_coef_t);
   
   if (st->lpc_enh_enabled)
   {
      float r=.9;
      
      float k1,k2,k3;
      if (st->submodes[st->submodeID] != NULL)
      {
         k1=SUBMODE(lpc_enh_k1);
         k2=SUBMODE(lpc_enh_k2);
      } else {
         k1=k2=.7;
      }
      k3=(1-(1-r*k1)/(1-r*k2))/r;
      k3=k1-k2;
      if (!st->lpc_enh_enabled)
      {
         k1=k2;
         k3=0;
      }
      bw_lpc(GAMMA_SCALING*k1, st->interp_qlpc, awk1, st->lpcSize);
      bw_lpc(GAMMA_SCALING*k2, st->interp_qlpc, awk2, st->lpcSize);
      bw_lpc(GAMMA_SCALING*k3, st->interp_qlpc, awk3, st->lpcSize);
      /*fprintf (stderr, "%f %f %f\n", k1, k2, k3);*/
   }
   
   
   /* Final signal synthesis from excitation */
   if (!dtx)
   {
      for (i=0;i<st->frame_size;i++)
         st->exc[i] *= .9;
   }

   for (i=0;i<st->frame_size;i++)
      st->high[i]=st->exc[i];

   if (st->lpc_enh_enabled)
   {
      /* Use enhanced LPC filter */
      filter_mem2(st->high, awk2, awk1, st->high, st->frame_size, st->lpcSize, 
                  st->mem_sp+st->lpcSize);
      filter_mem2(st->high, awk3, st->interp_qlpc, st->high, st->frame_size, st->lpcSize, 
                  st->mem_sp);
   } else {
      /* Use regular filter */
      for (i=0;i<st->lpcSize;i++)
         st->mem_sp[st->lpcSize+i] = 0;
      iir_mem2(st->high, st->interp_qlpc, st->high, st->frame_size, st->lpcSize, 
               st->mem_sp);
   }
   
   /*iir_mem2(st->exc, st->interp_qlpc, st->high, st->frame_size, st->lpcSize, st->mem_sp);*/
   
   /* Reconstruct the original */
   fir_mem_up(st->x0d, h0, st->y0, st->full_frame_size, QMF_ORDER, st->g0_mem, stack);
   fir_mem_up(st->high, h1, st->y1, st->full_frame_size, QMF_ORDER, st->g1_mem, stack);

   mix_and_saturate(st->y0, st->y1, out, st->full_frame_size);

   if (dtx)
   {
      st->submodeID=saved_modeid;
   }

   return;
}

int sb_decode(void *state, SpeexBits *bits, short *out)
{
   int i, sub;
   SBDecState *st;
   int wideband;
   int ret;
   char *stack;
   spx_word32_t *low_pi_gain;
   spx_sig_t *low_exc, *low_innov;
   spx_coef_t *awk1, *awk2, *awk3;
   int dtx;
   SpeexSBMode *mode;

   st = (SBDecState*)state;
   stack=st->stack;
   mode = (SpeexSBMode*)(st->mode->mode);

   {
      short *low;
      low = PUSH(stack, st->frame_size, short);
      
      /* Decode the low-band */
      ret = speex_decode(st->st_low, bits, low);
      
      for (i=0;i<st->frame_size;i++)
         st->x0d[i] = low[i]*SIG_SCALING;
   }

   speex_decoder_ctl(st->st_low, SPEEX_GET_DTX_STATUS, &dtx);

   /* If error decoding the narrowband part, propagate error */
   if (ret!=0)
   {
      return ret;
   }

   if (!bits)
   {
      sb_decode_lost(st, out, dtx, stack);
      return 0;
   }

   if (st->encode_submode)
   {

      /*Check "wideband bit"*/
      if (speex_bits_remaining(bits)>0)
         wideband = speex_bits_peek(bits);
      else
         wideband = 0;
      if (wideband)
      {
         /*Regular wideband frame, read the submode*/
         wideband = speex_bits_unpack_unsigned(bits, 1);
         st->submodeID = speex_bits_unpack_unsigned(bits, SB_SUBMODE_BITS);
      } else
      {
         /*Was a narrowband frame, set "null submode"*/
         st->submodeID = 0;
      }
      if (st->submodeID != 0 && st->submodes[st->submodeID] == NULL)
      {
         speex_warning("Invalid mode encountered: corrupted stream?");
         return -2;
      }
   }

   /* If null mode (no transmission), just set a couple things to zero*/
   if (st->submodes[st->submodeID] == NULL)
   {
      if (dtx)
      {
         sb_decode_lost(st, out, 1, stack);
         return 0;
      }

      for (i=0;i<st->frame_size;i++)
         st->exc[i]=VERY_SMALL;

      st->first=1;

      /* Final signal synthesis from excitation */
      iir_mem2(st->exc, st->interp_qlpc, st->high, st->frame_size, st->lpcSize, st->mem_sp);

      fir_mem_up(st->x0d, h0, st->y0, st->full_frame_size, QMF_ORDER, st->g0_mem, stack);
      fir_mem_up(st->high, h1, st->y1, st->full_frame_size, QMF_ORDER, st->g1_mem, stack);

      mix_and_saturate(st->y0, st->y1, out, st->full_frame_size);

      return 0;

   }

   for (i=0;i<st->frame_size;i++)
      st->exc[i]=0;

   low_pi_gain = PUSH(stack, st->nbSubframes, spx_word32_t);
   low_exc = PUSH(stack, st->frame_size, spx_sig_t);
   low_innov = PUSH(stack, st->frame_size, spx_sig_t);
   speex_decoder_ctl(st->st_low, SPEEX_GET_PI_GAIN, low_pi_gain);
   speex_decoder_ctl(st->st_low, SPEEX_GET_EXC, low_exc);
   speex_decoder_ctl(st->st_low, SPEEX_GET_INNOV, low_innov);

   SUBMODE(lsp_unquant)(st->qlsp, st->lpcSize, bits);
   
   if (st->first)
   {
      for (i=0;i<st->lpcSize;i++)
         st->old_qlsp[i] = st->qlsp[i];
   }
   
   awk1=PUSH(stack, st->lpcSize+1, spx_coef_t);
   awk2=PUSH(stack, st->lpcSize+1, spx_coef_t);
   awk3=PUSH(stack, st->lpcSize+1, spx_coef_t);

   for (sub=0;sub<st->nbSubframes;sub++)
   {
      spx_sig_t *exc, *sp;
      spx_word16_t filter_ratio;
      spx_word16_t el=0;
      int offset;
      spx_word32_t rl=0,rh=0;
      
      offset = st->subframeSize*sub;
      sp=st->high+offset;
      exc=st->exc+offset;
      
      /* LSP interpolation */
      lsp_interpolate(st->old_qlsp, st->qlsp, st->interp_qlsp, st->lpcSize, sub, st->nbSubframes);

      lsp_enforce_margin(st->interp_qlsp, st->lpcSize, LSP_MARGIN);

      /* LSP to LPC */
      lsp_to_lpc(st->interp_qlsp, st->interp_qlpc, st->lpcSize, stack);


      if (st->lpc_enh_enabled)
      {
         float r=.9;
         
         float k1,k2,k3;
         k1=SUBMODE(lpc_enh_k1);
         k2=SUBMODE(lpc_enh_k2);
         k3=(1-(1-r*k1)/(1-r*k2))/r;
         k3=k1-k2;
         if (!st->lpc_enh_enabled)
         {
            k1=k2;
            k3=0;
         }
         bw_lpc(GAMMA_SCALING*k1, st->interp_qlpc, awk1, st->lpcSize);
         bw_lpc(GAMMA_SCALING*k2, st->interp_qlpc, awk2, st->lpcSize);
         bw_lpc(GAMMA_SCALING*k3, st->interp_qlpc, awk3, st->lpcSize);
         /*fprintf (stderr, "%f %f %f\n", k1, k2, k3);*/
      }


      /* Calculate reponse ratio between the low and high filter in the middle
         of the band (4000 Hz) */
      
         st->pi_gain[sub]=LPC_SCALING;
         rh = LPC_SCALING;
         for (i=1;i<=st->lpcSize;i+=2)
         {
            rh += st->interp_qlpc[i+1] - st->interp_qlpc[i];
            st->pi_gain[sub] += st->interp_qlpc[i] + st->interp_qlpc[i+1];
         }

         rl = low_pi_gain[sub];
#ifdef FIXED_POINT
         filter_ratio=DIV32_16(SHL(rl+82,2),SHR(82+rh,5));
#else
         filter_ratio=(rl+.01)/(rh+.01);
#endif
      
      for (i=0;i<st->subframeSize;i++)
         exc[i]=0;
      if (!SUBMODE(innovation_unquant))
      {
         float g;
         int quant;

         quant = speex_bits_unpack_unsigned(bits, 5);
         g= exp(((float)quant-10)/8.0);
         
#ifdef FIXED_POINT
         g /= filter_ratio/128.;
#else
         g /= filter_ratio;
#endif
         /* High-band excitation using the low-band excitation and a gain */
         for (i=0;i<st->subframeSize;i++)
            exc[i]=mode->folding_gain*g*low_innov[offset+i];
         /*speex_rand_vec(mode->folding_gain*g*sqrt(el/st->subframeSize), exc, st->subframeSize);*/
      } else {
         spx_word16_t gc;
         spx_word32_t scale;
         int qgc = speex_bits_unpack_unsigned(bits, 4);

         el = compute_rms(low_exc+offset, st->subframeSize);

#ifdef FIXED_POINT
         gc = MULT16_32_Q15(28626,gc_quant_bound[qgc]);
#else
         gc = exp((1/3.7)*qgc-0.15556);
#endif

         if (st->subframeSize==80)
            gc *= 1.4142;

         scale = SHL(MULT16_16(DIV32_16(SHL(gc,SIG_SHIFT-4),filter_ratio),(1+el)),4);

         SUBMODE(innovation_unquant)(exc, SUBMODE(innovation_params), st->subframeSize, 
                                bits, stack);

         signal_mul(exc,exc,scale,st->subframeSize);

         if (SUBMODE(double_codebook)) {
            char *tmp_stack=stack;
            spx_sig_t *innov2 = PUSH(tmp_stack, st->subframeSize, spx_sig_t);
            for (i=0;i<st->subframeSize;i++)
               innov2[i]=0;
            SUBMODE(innovation_unquant)(innov2, SUBMODE(innovation_params), st->subframeSize, 
                                bits, tmp_stack);
            for (i=0;i<st->subframeSize;i++)
               innov2[i]*=scale/(float)SIG_SCALING*(1/2.5);
            for (i=0;i<st->subframeSize;i++)
               exc[i] += innov2[i];
         }

      }

      for (i=0;i<st->subframeSize;i++)
         sp[i]=exc[i];
      if (st->lpc_enh_enabled)
      {
         /* Use enhanced LPC filter */
         filter_mem2(sp, awk2, awk1, sp, st->subframeSize, st->lpcSize, 
                     st->mem_sp+st->lpcSize);
         filter_mem2(sp, awk3, st->interp_qlpc, sp, st->subframeSize, st->lpcSize, 
                     st->mem_sp);
      } else {
         /* Use regular filter */
         for (i=0;i<st->lpcSize;i++)
            st->mem_sp[st->lpcSize+i] = 0;
         iir_mem2(sp, st->interp_qlpc, sp, st->subframeSize, st->lpcSize, 
                     st->mem_sp);
      }
      /*iir_mem2(exc, st->interp_qlpc, sp, st->subframeSize, st->lpcSize, st->mem_sp);*/

   }

   fir_mem_up(st->x0d, h0, st->y0, st->full_frame_size, QMF_ORDER, st->g0_mem, stack);
   fir_mem_up(st->high, h1, st->y1, st->full_frame_size, QMF_ORDER, st->g1_mem, stack);

   mix_and_saturate(st->y0, st->y1, out, st->full_frame_size);

   for (i=0;i<st->lpcSize;i++)
      st->old_qlsp[i] = st->qlsp[i];

   st->first=0;

   return 0;
}


#ifndef TOMATO_DECODE_ONLY
int sb_encoder_ctl(void *state, int request, void *ptr)
{
   SBEncState *st;
   st=(SBEncState*)state;
   switch(request)
   {
   case SPEEX_GET_FRAME_SIZE:
      (*(int*)ptr) = st->full_frame_size;
      break;
   case SPEEX_SET_HIGH_MODE:
      st->submodeSelect = st->submodeID = (*(int*)ptr);
      break;
   case SPEEX_SET_LOW_MODE:
      speex_encoder_ctl(st->st_low, SPEEX_SET_LOW_MODE, ptr);
      break;
   case SPEEX_SET_DTX:
      speex_encoder_ctl(st->st_low, SPEEX_SET_DTX, ptr);
      break;
   case SPEEX_GET_DTX:
      speex_encoder_ctl(st->st_low, SPEEX_GET_DTX, ptr);
      break;
   case SPEEX_GET_LOW_MODE:
      speex_encoder_ctl(st->st_low, SPEEX_GET_LOW_MODE, ptr);
      break;
   case SPEEX_SET_MODE:
      speex_encoder_ctl(st, SPEEX_SET_QUALITY, ptr);
      break;
   case SPEEX_SET_VBR:
      st->vbr_enabled = (*(int*)ptr);
      speex_encoder_ctl(st->st_low, SPEEX_SET_VBR, ptr);
      break;
   case SPEEX_GET_VBR:
      (*(int*)ptr) = st->vbr_enabled;
      break;
   case SPEEX_SET_VAD:
      st->vad_enabled = (*(int*)ptr);
      speex_encoder_ctl(st->st_low, SPEEX_SET_VAD, ptr);
      break;
   case SPEEX_GET_VAD:
      (*(int*)ptr) = st->vad_enabled;
      break;
   case SPEEX_SET_VBR_QUALITY:
      {
         int q;
         float qual = (*(float*)ptr)+.6;
         st->vbr_quality = (*(float*)ptr);
         if (qual>10)
            qual=10;
         q=(int)floor(.5+*(float*)ptr);
         if (q>10)
            q=10;
         speex_encoder_ctl(st->st_low, SPEEX_SET_VBR_QUALITY, &qual);
         speex_encoder_ctl(state, SPEEX_SET_QUALITY, &q);
         break;
      }
   case SPEEX_SET_ABR:
      st->abr_enabled = (*(int*)ptr);
      st->vbr_enabled = 1;
      speex_encoder_ctl(st->st_low, SPEEX_SET_VBR, &st->vbr_enabled);
      {
         int i=10, rate, target;
         float vbr_qual;
         target = (*(int*)ptr);
         while (i>=0)
         {
            speex_encoder_ctl(st, SPEEX_SET_QUALITY, &i);
            speex_encoder_ctl(st, SPEEX_GET_BITRATE, &rate);
            if (rate <= target)
               break;
            i--;
         }
         vbr_qual=i;
         if (vbr_qual<0)
            vbr_qual=0;
         speex_encoder_ctl(st, SPEEX_SET_VBR_QUALITY, &vbr_qual);
         st->abr_count=0;
         st->abr_drift=0;
         st->abr_drift2=0;
      }
      
      break;
   case SPEEX_GET_ABR:
      (*(int*)ptr) = st->abr_enabled;
      break;
   case SPEEX_SET_QUALITY:
      {
         int nb_qual;
         int quality = (*(int*)ptr);
         if (quality < 0)
            quality = 0;
         if (quality > 10)
            quality = 10;
         st->submodeSelect = st->submodeID = ((SpeexSBMode*)(st->mode->mode))->quality_map[quality];
         nb_qual = ((SpeexSBMode*)(st->mode->mode))->low_quality_map[quality];
         speex_encoder_ctl(st->st_low, SPEEX_SET_MODE, &nb_qual);
      }
      break;
   case SPEEX_SET_COMPLEXITY:
      speex_encoder_ctl(st->st_low, SPEEX_SET_COMPLEXITY, ptr);
      st->complexity = (*(int*)ptr);
      if (st->complexity<1)
         st->complexity=1;
      break;
   case SPEEX_GET_COMPLEXITY:
      (*(int*)ptr) = st->complexity;
      break;
   case SPEEX_SET_BITRATE:
      {
         int i=10, rate, target;
         target = (*(int*)ptr);
         while (i>=0)
         {
            speex_encoder_ctl(st, SPEEX_SET_QUALITY, &i);
            speex_encoder_ctl(st, SPEEX_GET_BITRATE, &rate);
            if (rate <= target)
               break;
            i--;
         }
      }
      break;
   case SPEEX_GET_BITRATE:
      speex_encoder_ctl(st->st_low, request, ptr);
      /*fprintf (stderr, "before: %d\n", (*(int*)ptr));*/
      if (st->submodes[st->submodeID])
         (*(int*)ptr) += st->sampling_rate*SUBMODE(bits_per_frame)/st->full_frame_size;
      else
         (*(int*)ptr) += st->sampling_rate*(SB_SUBMODE_BITS+1)/st->full_frame_size;
      /*fprintf (stderr, "after: %d\n", (*(int*)ptr));*/
      break;
   case SPEEX_SET_SAMPLING_RATE:
      {
         int tmp=(*(int*)ptr);
         st->sampling_rate = tmp;
         tmp>>=1;
         speex_encoder_ctl(st->st_low, SPEEX_SET_SAMPLING_RATE, &tmp);
      }
      break;
   case SPEEX_GET_SAMPLING_RATE:
      (*(int*)ptr)=st->sampling_rate;
      break;
   case SPEEX_RESET_STATE:
      {
         int i;
         st->first = 1;
         for (i=0;i<st->lpcSize;i++)
            st->lsp[i]=(M_PI*((float)(i+1)))/(st->lpcSize+1);
         for (i=0;i<st->lpcSize;i++)
            st->mem_sw[i]=st->mem_sp[i]=st->mem_sp2[i]=0;
         for (i=0;i<st->bufSize;i++)
            st->excBuf[i]=0;
         for (i=0;i<QMF_ORDER;i++)
            st->h0_mem[i]=st->h1_mem[i]=st->g0_mem[i]=st->g1_mem[i]=0;
      }
      break;
   case SPEEX_SET_SUBMODE_ENCODING:
      st->encode_submode = (*(int*)ptr);
      speex_encoder_ctl(st->st_low, SPEEX_SET_SUBMODE_ENCODING, &ptr);
      break;
   case SPEEX_GET_SUBMODE_ENCODING:
      (*(int*)ptr) = st->encode_submode;
      break;
   case SPEEX_GET_PI_GAIN:
      {
         int i;
         spx_word32_t *g = (spx_word32_t*)ptr;
         for (i=0;i<st->nbSubframes;i++)
            g[i]=st->pi_gain[i];
      }
      break;
   case SPEEX_GET_EXC:
      {
         int i;
         spx_sig_t *e = (spx_sig_t*)ptr;
         for (i=0;i<st->full_frame_size;i++)
            e[i]=0;
         for (i=0;i<st->frame_size;i++)
            e[2*i]=2*st->exc[i];
      }
      break;
   case SPEEX_GET_INNOV:
      {
         int i;
         spx_sig_t *e = (spx_sig_t*)ptr;
         for (i=0;i<st->full_frame_size;i++)
            e[i]=0;
         for (i=0;i<st->frame_size;i++)
            e[2*i]=2*st->exc[i];
      }
      break;
   case SPEEX_GET_RELATIVE_QUALITY:
      (*(float*)ptr)=st->relative_quality;
      break;
   default:
      speex_warning_int("Unknown nb_ctl request: ", request);
      return -1;
   }
   return 0;
}
#endif

int sb_decoder_ctl(void *state, int request, void *ptr)
{
   SBDecState *st;
   st=(SBDecState*)state;
   switch(request)
   {
   case SPEEX_SET_HIGH_MODE:
      st->submodeID = (*(int*)ptr);
      break;
   case SPEEX_SET_LOW_MODE:
   	// tomato - bug????
    //  speex_encoder_ctl(st->st_low, SPEEX_SET_LOW_MODE, ptr);
      speex_decoder_ctl(st->st_low, SPEEX_SET_LOW_MODE, ptr);
      break;
   case SPEEX_GET_LOW_MODE:
      speex_decoder_ctl(st->st_low, SPEEX_GET_LOW_MODE, ptr);
      break;
   case SPEEX_GET_FRAME_SIZE:
      (*(int*)ptr) = st->full_frame_size;
      break;
   case SPEEX_SET_ENH:
      speex_decoder_ctl(st->st_low, request, ptr);
      st->lpc_enh_enabled = *((int*)ptr);
      break;
   case SPEEX_SET_MODE:
   case SPEEX_SET_QUALITY:
      {
         int nb_qual;
         int quality = (*(int*)ptr);
         if (quality < 0)
            quality = 0;
         if (quality > 10)
            quality = 10;
         st->submodeID = ((SpeexSBMode*)(st->mode->mode))->quality_map[quality];
         nb_qual = ((SpeexSBMode*)(st->mode->mode))->low_quality_map[quality];
         speex_decoder_ctl(st->st_low, SPEEX_SET_MODE, &nb_qual);
      }
      break;
   case SPEEX_GET_BITRATE:
      speex_decoder_ctl(st->st_low, request, ptr);
      if (st->submodes[st->submodeID])
         (*(int*)ptr) += st->sampling_rate*SUBMODE(bits_per_frame)/st->full_frame_size;
      else
         (*(int*)ptr) += st->sampling_rate*(SB_SUBMODE_BITS+1)/st->full_frame_size;
      break;
   case SPEEX_SET_SAMPLING_RATE:
      {
         int tmp=(*(int*)ptr);
         st->sampling_rate = tmp;
         tmp>>=1;
         speex_decoder_ctl(st->st_low, SPEEX_SET_SAMPLING_RATE, &tmp);
      }
      break;
   case SPEEX_GET_SAMPLING_RATE:
      (*(int*)ptr)=st->sampling_rate;
      break;
   case SPEEX_SET_HANDLER:
      speex_decoder_ctl(st->st_low, SPEEX_SET_HANDLER, ptr);
      break;
   case SPEEX_SET_USER_HANDLER:
      speex_decoder_ctl(st->st_low, SPEEX_SET_USER_HANDLER, ptr);
      break;
   case SPEEX_RESET_STATE:
      {
         int i;
         for (i=0;i<2*st->lpcSize;i++)
            st->mem_sp[i]=0;
         for (i=0;i<QMF_ORDER;i++)
            st->g0_mem[i]=st->g1_mem[i]=0;
      }
      break;
   case SPEEX_SET_SUBMODE_ENCODING:
      st->encode_submode = (*(int*)ptr);
      speex_decoder_ctl(st->st_low, SPEEX_SET_SUBMODE_ENCODING, &ptr);
      break;
   case SPEEX_GET_SUBMODE_ENCODING:
      (*(int*)ptr) = st->encode_submode;
      break;
   case SPEEX_GET_PI_GAIN:
      {
         int i;
         spx_word32_t *g = (spx_word32_t*)ptr;
         for (i=0;i<st->nbSubframes;i++)
            g[i]=st->pi_gain[i];
      }
      break;
   case SPEEX_GET_EXC:
      {
         int i;
         spx_sig_t *e = (spx_sig_t*)ptr;
         for (i=0;i<st->full_frame_size;i++)
            e[i]=0;
         for (i=0;i<st->frame_size;i++)
            e[2*i]=2*st->exc[i];
      }
      break;
   case SPEEX_GET_INNOV:
      {
         int i;
         spx_sig_t *e = (spx_sig_t*)ptr;
         for (i=0;i<st->full_frame_size;i++)
            e[i]=0;
         for (i=0;i<st->frame_size;i++)
            e[2*i]=2*st->exc[i];
      }
      break;
   case SPEEX_GET_DTX_STATUS:
      speex_decoder_ctl(st->st_low, SPEEX_GET_DTX_STATUS, ptr);
      break;
   default:
      speex_warning_int("Unknown nb_ctl request: ", request);
      return -1;
   }
   return 0;
}


