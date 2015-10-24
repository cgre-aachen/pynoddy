/*
** PURPOSE      Main Fast Fourier Transform function. 
**
**              Multivariate data is indexed according to the C
**              array element successor function, without limit 
**              on the number of implied multiple subscripts. 
**              The function is called once for each variate. 
**              The calls for a multivariate transform may be in any order.
**              A tri-variate transform with a(n1,n2,n3), b(n1,n2,n3) 
**              is computed by 
**                      fft(a,b,n1*n2*n3,n1,n1,1) 
**                      fft(a,b,n1*n2*n3,n2,n1*n2,1) 
**                      fft(a,b,n1*n2*n3,n3,n1*n2*n3,1) 
** PARAMETERS
**
**              real - input & output
**                      The real array
**
**              imag - input & output
**                      The imaginary array
**
**              ntot - input
**                      Total number of data values. 
**
**              n - input
**                      Dimension of the current variable. 
**
**              nspan - input
**                      nspan/n is the spacing of consecutive data values 
**                      while indexing the current variable. 
**
**              isn - input
**                      The sign of isn determines the sign of the complex 
**                      exponential, and the magnitude of isn is normally one. 
**                      -1=image->fourier 1=fourier->image 
**
** EDITS
 *******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "xvt.h"
#include "noddy.h"
#include <math.h>

#define maxprime 23
#define maxfact 13
#define maxp 209

int
fft(real, imag, ntot, n, nspan, isn)
double *real;
double *imag;
long ntot;
long n;
long nspan;
int isn;
{

    int nfac[maxfact], *nfacm1;
    union {
            int i;
            int ii;
    } i_ii;

    int j, k, m, kspan, maxf;
    int kspnn, k1, k2, k3, k4;
    int jc, jf, jj, kk, nn, ks, kt, nt, inc;
    int np[maxp], *npm1;

    if (n < 2) return(0);

    nfacm1=nfac-1;
    npm1=np-1;
  {
    double radf, *a, *b;
    double c1, c2, c3;
    double s1, s2, s3, aa, bb, cd, aj, c72;
    double ck[maxprime], ak, *ckm1;
    double bk;
    double bj;
    double at[maxprime], bt[maxprime], sd, *atm1, *btm1;
    double s72;
    double sk[maxprime], *skm1;
    double s120, rad, ajm, akm;
    double ajp, akp, bkp, bkm, bjp, bjm;

    a=((double *)real)-1;
    b=((double *)imag)-1;
    ckm1=ck-1;
    atm1=at-1;
    btm1=bt-1;
    skm1=sk-1;
    maxf=maxprime;
    inc = isn;
    c72 = 0.30901699437494742;
    s72 = 0.95105651629515357;
    s120 = 0.86602540378443865;
    rad = 6.2831853071796;
    if (isn < 0) {
        s72 = -s72;
        s120 = -s120;
        rad = -rad;
        inc = -inc;
    }
    nt = (int) (inc * ntot);
    ks = (int) (inc * nspan);
    kspan = ks;
    nn = nt - inc;
    jc = (int) (ks / n);
    radf = rad * jc * 0.5;
    i_ii.i = -1;
    jf = 0;

/*  determine the factors of n */
    m = 0;
    k = (int) n;
    while ((k&15) == 0) {
        nfac[m++] = 4;
        k>>=4;
    }

    j = 3;
    jj = 9;

L30:
    while (k % jj == 0) {
        nfac[m++] = j;
        k /= jj;
    }
    j += 2;
/* Computing 2nd power */
    jj = j * j;
    if (jj <= k) goto L30;
    if (k <= 4) {
        kt = m;
        nfac[m] = k;
        if (k != 1) ++m;
    } else {
        if ((k&3) == 0) {
            nfac[m++] = 2;
            k>>=2;
        }
        kt = m;
        j = 2;
        do {
            if (k % j == 0) {
                nfac[m++] = j;
                k /= j;
            }
            j = 1+(((++j)>>1)<<1);
        } while (j <= k);
    }


    if (kt != 0) {
        j = kt;
        do {
            nfac[m++] = nfac[--j];
        } while (j != 0);
    }
/*  compute fourier transform */
L100: 
    sd = radf / (double) kspan;
/* Computing 2nd power */
    cd = sin(sd);
    cd = cd * cd * 2.0;
    sd = sin(sd + sd);
    kk = 1;

    if (nfac[++i_ii.i] != 2) goto L400;

/*  transform for factor of 2 (including rotation factor) */
    kspan>>=1;
    k1 = kspan + 2;

    do {
        do { 
            k2 = kk + kspan;
            ak = a[k2];
            bk = b[k2];
            a[k2] = a[kk] - ak;
            b[k2] = b[kk] - bk;
            a[kk] += ak;
            b[kk] += bk;
            kk = k2 + kspan;
        } while(kk <= nn);
        kk -= nn;
    } while (kk <= jc);
    if (kk > kspan) goto L800;

    do {
        c1 = 1.0 - cd;
        s1 = sd;
        do {
            do {
                do {
                    k2 = kk + kspan;
                    ak = a[kk] - a[k2];
                    bk = b[kk] - b[k2];
                    a[kk] += a[k2];
                    b[kk] += b[k2];
                    a[k2] = c1 * ak - s1 * bk;
                    b[k2] = s1 * ak + c1 * bk;
                    kk = k2 + kspan;
                } while (kk < nt);
                k2 = kk - nt;
                c1 = -c1;
                kk = k1 - k2;
            } while (kk > k2);
            ak = c1 - (cd * c1 + sd * s1);
            s1 = sd * c1 - cd * s1 + s1;
/* Computing 2nd power */
            c1 = 2.0 - (ak * ak + s1 * s1);
            s1 *= c1;
            c1 *= ak;
            kk += jc;
        } while (kk < k2);
        k1 += (inc<<1);
        kk = (k1 - kspan) / 2 + jc;
    } while (kk <= (jc<<1));
    goto L100;

/*  transform for factor of 3 (optional code) */
L320:
    k1 = kk + kspan;
    k2 = k1 + kspan;
    ak = a[kk];
    bk = b[kk];
    aj = a[k1] + a[k2];
    bj = b[k1] + b[k2];
    a[kk] = ak + aj;
    b[kk] = bk + bj;
    ak += (aj * -0.5);
    bk += (bj * -0.5);
    aj = (a[k1] - a[k2]) * s120;
    bj = (b[k1] - b[k2]) * s120;
    a[k1] = ak - bj;
    b[k1] = bk + aj;
    a[k2] = ak + bj;
    b[k2] = bk - aj;
    kk = k2 + kspan;
    if (kk < nn) goto L320;
    kk -= nn;
    if (kk <= kspan) goto L320;
    goto L700;

/*  transform for factor of 4 */
L400: 
    if (nfac[i_ii.i] != 4) goto L600;
    kspnn = kspan;
    kspan>>=2;

L410:
    c1 = 1.0;
    s1 = 0.0;

L420:
    k1 = kk + kspan;
    k2 = k1 + kspan;
    k3 = k2 + kspan;
    akp = a[kk] + a[k2];
    akm = a[kk] - a[k2];
    ajp = a[k1] + a[k3];
    ajm = a[k1] - a[k3];
    a[kk] = akp + ajp;
    ajp = akp - ajp;
    bkp = b[kk] + b[k2];
    bkm = b[kk] - b[k2];
    bjp = b[k1] + b[k3];
    bjm = b[k1] - b[k3];
    b[kk] = bkp + bjp;
    bjp = bkp - bjp;
    if (isn < 0) goto L450;

    akp = akm - bjm;
    akm += bjm;
    bkp = bkm + ajm;
    bkm -= ajm;
    if (s1 == 0.0) goto L460;

L430:
    a[k1] = akp * c1 - bkp * s1;
    b[k1] = akp * s1 + bkp * c1;
    a[k2] = ajp * c2 - bjp * s2;
    b[k2] = ajp * s2 + bjp * c2;
    a[k3] = akm * c3 - bkm * s3;
    b[k3] = akm * s3 + bkm * c3;
    kk = k3 + kspan;
    if (kk <= nt) goto L420;

L440:
    c2 = c1 - (cd * c1 + sd * s1);
    s1 = sd * c1 - cd * s1 + s1;
/* Computing 2nd power */
    c1 = 2.0 - (c2 * c2 + s1 * s1);
    s1 *= c1;
    c1 *= c2;
/* Computing 2nd power */
    c2 = c1 * c1 - s1 * s1;
    s2 = c1 * 2.0 * s1;
    c3 = c2 * c1 - s2 * s1;
    s3 = c2 * s1 + s2 * c1;
    kk += (jc - nt);
    if (kk <= kspan) goto L420;
    kk += (inc - kspan);
    if (kk <= jc) goto L410;
    if (kspan == jc) goto L800;
    goto L100;

L450:
    akp = akm + bjm;
    akm -= bjm;
    bkp = bkm - ajm;
    bkm += ajm;
    if (s1 != 0.0) goto L430;

L460:
    a[k1] = akp;
    b[k1] = bkp;
    a[k2] = ajp;
    b[k2] = bjp;
    a[k3] = akm;
    b[k3] = bkm;
    kk = k3 + kspan;
    if (kk <= nt) goto L420;
    goto L440;

/*  transform for factor of 5 (optional code) */
L510:
/* Computing 2nd power */
    c2 = c72 * c72 - s72 * s72;
    s2 = c72 * 2.0 * s72;
    do {
        do {
            k1 = kk + kspan;
            k2 = k1 + kspan;
            k3 = k2 + kspan;
            k4 = k3 + kspan;
            akp = a[k1] + a[k4];
            akm = a[k1] - a[k4];
            bkp = b[k1] + b[k4];
            bkm = b[k1] - b[k4];
            ajp = a[k2] + a[k3];
            ajm = a[k2] - a[k3];
            bjp = b[k2] + b[k3];
            bjm = b[k2] - b[k3];
            aa = a[kk];
            bb = b[kk];
            a[kk] = aa + akp + ajp;
            b[kk] = bb + bkp + bjp;
            ak = akp * c72 + ajp * c2 + aa;
            bk = bkp * c72 + bjp * c2 + bb;
            aj = akm * s72 + ajm * s2;
            bj = bkm * s72 + bjm * s2;
            a[k1] = ak - bj;
            a[k4] = ak + bj;
            b[k1] = bk + aj;
            b[k4] = bk - aj;
            ak = akp * c2 + ajp * c72 + aa;
            bk = bkp * c2 + bjp * c72 + bb;
            aj = akm * s2 - ajm * s72;
            bj = bkm * s2 - bjm * s72;
            a[k2] = ak - bj;
            a[k3] = ak + bj;
            b[k2] = bk + aj;
            b[k3] = bk - aj;
            kk = k4 + kspan;
        } while (kk < nn);
        kk -= nn;
    } while (kk <= kspan);
    goto L700;

/*  transform for odd factors */
L600: 
    k = nfac[i_ii.i];
    kspnn = kspan;
    kspan /= k;
    if (k == 3) goto L320;
    if (k == 5) goto L510;
    if (k == jf) goto L640;
    jf = k;
    s1 = rad / (double) k;
    c1 = cos(s1);
    s1 = sin(s1);
    if (jf > maxf) return(1);

    ckm1[jf] = 1.0;
    skm1[jf] = 0.0;
    j = 1;

    do {
        ckm1[j] = ckm1[k] * c1 + skm1[k] * s1;
        skm1[j] = ckm1[k] * s1 - skm1[k] * c1;
        --k;
        ckm1[k] = ckm1[j];
        skm1[k] = -skm1[j];
        ++j;
    } while (j < k);

L640:
    k1 = kk;
    k2 = kk + kspnn;
    aa = a[kk];
    bb = b[kk];
    ak = aa;
    bk = bb;
    j = 1;
    k1 += kspan;
    do {
        k2 -= kspan;
        ++j;
        atm1[j] = a[k1] + a[k2];
        ak += atm1[j];
        btm1[j] = b[k1] + b[k2];
        bk += btm1[j];
        ++j;
        atm1[j] = a[k1] - a[k2];
        btm1[j] = b[k1] - b[k2];
        k1 += kspan;
    } while (k1 < k2);
    a[kk] = ak;
    b[kk] = bk;
    k1 = kk;
    k2 = kk + kspnn;
    j = 1;

L660:
    k1 += kspan;
    k2 -= kspan;
    jj = j;
    ak = aa;
    bk = bb;
    aj = 0.0;
    bj = 0.0;
    k = 1;
    do {
        ++k;
        ak += (atm1[k] * ckm1[jj]);
        bk += (btm1[k] * ckm1[jj]);
        ++k;
        aj += (atm1[k] * skm1[jj]);
        bj += (btm1[k] * skm1[jj]);
        jj += j;
        if (jj > jf) jj -= jf;
    } while (k < jf);
    k = jf - j;
    a[k1] = ak - bj;
    b[k1] = bk + aj;
    a[k2] = ak + bj;
    b[k2] = bk - aj;
    ++j;
    if (j < k) goto L660;
    kk += kspnn;
    if (kk <= nn) goto L640;
    kk -= nn;
    if (kk <= kspan) goto L640;

/*  multiply by rotation factor (except for factors of 2 and 4) */
L700:
    if (i_ii.i == m-1) goto L800;
    kk = jc + 1;

L710:
    c2 = 1.0 - cd;
    s1 = sd;

L720:
    c1 = c2;
    s2 = s1;
    kk += kspan;

L730:
    ak = a[kk];
    a[kk] = c2 * ak - s2 * b[kk];
    b[kk] = s2 * ak + c2 * b[kk];
    kk += kspnn;
    if (kk <= nt) goto L730;
    ak = s1 * s2;
    s2 = s1 * c2 + c1 * s2;
    c2 = c1 * c2 - ak;
    kk += (kspan - nt);
    if (kk <= kspnn) goto L730;
    c2 = c1 - (cd * c1 + sd * s1);
    s1 += sd * c1 - cd * s1;
/* Computing 2nd power */
    c1 = 2.0 - (c2 * c2 + s1 * s1);
    s1 *= c1;
    c2 *= c1;
    kk += (jc - kspnn);
    if (kk <= kspan) goto L720;
    kk += (jc + inc - kspan);
    if (kk <= (jc<<1)) goto L710;
    goto L100;

/*  permute the results to normal order---done in two stages */
/*  permutation for square factors of n */
L800:
/* printf("done all factors -- L800 - permute results\n"); */ 
    np[0] = ks;
    if (kt == 0) goto L890;
    k = (kt<<1) + 1;
    if (m < k) --k;
    j = 1;
    np[k] = jc;
    do {
        np[j] = npm1[j] / nfacm1[j];
        npm1[k] = np[k] * nfacm1[j];
        ++j;
        --k;
    } while (j < k);
    k3 = np[k];
    kspan = np[1];
    kk = jc + 1;
    k2 = kspan + 1;
    j = 1;
    if (n != ntot) goto L850;
/* printf("permutation for single-variate transform\n"); */ 
/*  permutation for single-variate transform (optional code) */
L820:
    ak = a[kk];
    a[kk] = a[k2];
    a[k2] = ak;
    bk = b[kk];
    b[kk] = b[k2];
    b[k2] = bk;
    kk += inc;
    k2 += kspan;
    if (k2 < ks) goto L820;

L830:
    k2 -= npm1[j];
    ++j;
    k2 += np[j];
    if (k2 > npm1[j]) goto L830;
    j = 1;

L840:
    if (kk < k2) goto L820;
    kk += inc;
    k2 += kspan;
    if (k2 < ks) goto L840;
    if (kk < ks) goto L830;
    jc = k3;
    goto L890;

/*  permutation for multivariate transform */
L850:
    k = kk + jc;
    do {
        ak = a[kk];
        a[kk] = a[k2];
        a[k2] = ak;
        bk = b[kk];
        b[kk] = b[k2];
        b[k2] = bk;
        kk += inc;
        k2 += inc;
    } while (kk < k);
    kk +=(ks - jc);
    k2 +=(ks - jc);
    if (kk < nt) goto L850;
    k2 += (kspan - nt);
    kk += (jc - nt);
    if (k2 < ks) goto L850;

L870:
    k2 -= npm1[j];
    ++j;
    k2 += np[j];
    if (k2 > npm1[j]) goto L870;
    j = 1;
    do {
        if (kk < k2) goto L850;
        kk += jc;
        k2 += kspan;
    } while (k2 < ks);
    if (kk < ks) goto L870;
    jc = k3;

L890:
    if ((kt<<1) + 1 >= m) goto L1000;
    kspnn = np[kt];

/* printf("permutation for square-free factors of n\n"); */
/*  permutation for square-free factors of n */
    j = m - kt;
    nfac[j] = 1;
    do {
        nfacm1[j] *= nfac[j];
        --j;
    } while (j != kt);
    ++kt;
    nn = nfacm1[kt] - 1;
    if (nn > maxp) return(1);
    jj = 0;
    j = 0;
    goto L906;

L902:
    jj -= k2;
    k2 = kk;
    ++k;
    kk = nfacm1[k];

L904:
    jj += kk;
    if (jj >= k2) goto L902;
    npm1[j] = jj;

L906:
    k2 = nfacm1[kt];
    k = kt + 1;
    kk = nfacm1[k];
    ++j;
    if (j <= nn) goto L904;

/*  determine the permutation cycles of length greater than 1 */
    j = 0;
    goto L914;

L910:
    k = kk;
    kk = npm1[k];
    npm1[k] = -kk;
    if (kk != j) goto L910;
    k3 = kk;

L914:
    ++j;
    kk = npm1[j];
    if (kk < 0) goto L914;
    if (kk != j) goto L910;
    npm1[j] = -j;
    if (j != nn) goto L914;
    maxf *= inc;

/*  reorder a and b, following the permutation cycles */
    goto L950;

L924:
    --j;
    if (npm1[j] < 0) goto L924;
    jj = jc;

L926:
    kspan = jj;
    if (jj > maxf) kspan = maxf;
    jj -= kspan;
    k = npm1[j];
    kk = jc * k + i_ii.ii + jj;
    k1 = kk + kspan;
    k2 = 0;
    do {
        ++k2;
        atm1[k2] = a[k1];
        btm1[k2] = b[k1];
        k1 -= inc;
    } while (k1 != kk);
    do {
        k1 = kk + kspan;
        k2 = k1 - jc * (k + npm1[k]);
        k = -npm1[k];
        do {
            a[k1] = a[k2];
            b[k1] = b[k2];
            k1 -= inc;
            k2 -= inc;
        } while (k1 != kk);
        kk = k2;
    } while (k != j);
    k1 = kk + kspan;
    k2 = 0;
    do {
        ++k2;
        a[k1] = atm1[k2];
        b[k1] = btm1[k2];
        k1 -= inc;
    } while (k1 != kk);
    if (jj != 0) goto L926;
    if (j != 1) goto L924;

L950:
    j = k3 + 1;
    nt -= kspnn;
    i_ii.ii = nt - inc + 1;
    if (nt >= 0) goto L924;

/* normalize the elements for forward transform*/
L1000:
    if (isn<0) {
        j = (int) (-ntot-1);
        while (++j<0) {
            *(++a)/=(double)n;
            *(++b)/=(double)n;
        }
    }
    return(0);

  }

}

/* test num for suitability for FFT calls. Return 1 if OK else return 0 */

int dcomp(num)
int num;
{
/* Written by S. Rodrigues: adapted from DCOMP.FOR by M. Craig */
    int prim_no[9]; /* 1st 9 prim_nos */
    int ind[9];
    int i; /* loop index */
    int quot; /* quot */
    int nfac=0; /* number of factors */
    int sqfp=1; /* square free part */

	 prim_no[0] = 2;    prim_no[1] = 3;
	 prim_no[2] = 5;    prim_no[3] = 7;
	 prim_no[4] = 11;   prim_no[5] = 13;
	 prim_no[6] = 17;   prim_no[7] = 19;
	 prim_no[8] = 23;

    if (num < 1)
		 return(0); /* num must be +ve */

    for (i = 0; i < 9; i++)
        ind[i] = 0;

    quot = num;
    i = 0;

    do {
       if ((quot%prim_no[i]) == 0)
		 {
          quot /= prim_no[i];
          ind[i]++;
          nfac++;
          if (quot < 2)
				 break;
       }
		 else
          i++;
    } while (i < 9);
 
    if (quot > 1)
		 return(0); /* has prime divisors > 23 */

    if (nfac > 13)
		 return(0); /* has more than 13 factors */

    for (i = 0; i < 9; i++)
	 {
       if ((ind[i]%2) == 1)
			 sqfp *= prim_no[i];
    }
    if (sqfp > 210)
		 return(0); /* has square-free part > 210 */

    return(1);

}


