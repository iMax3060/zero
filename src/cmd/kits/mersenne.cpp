/*
 * See copyright at top of file, it is the BSD copyright
 */

/** @file mersenne.cpp
    modifications from M. Matsumoto and T. Nishimura
*/

#include <cstdio>
#include <cstdlib>
#include <ctime>
#ifdef SYS_POSIX
#include <sys/time.h>
#include <unistd.h>
#endif
#ifdef SYS_NT
#include <windows.h>
#include <Wincrypt.h>
#endif

#include "util/MersenneTwisterRandom.hpp"

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* REFERENCE                                                       */
/* M. Matsumoto and T. Nishimura,                                  */
/* "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  */
/* Pseudo-Random Number Generator",                                */
/* ACM Transactions on Modeling and Computer Simulation,           */
/* Vol. 8, No. 1, January 1998, pp 3--30.                          */

using namespace lintel;

MersenneTwisterRandom MTRandom;

MersenneTwisterInternal::MersenneTwisterInternal(uint32_t seed)
{
  if (seed == 0) {
#ifdef SYS_POSIX
    seed = getpid() ^ (getppid() << 16);
    struct timeval t;
    gettimeofday(&t,nullptr);
    seed = seed ^ t.tv_sec ^ (t.tv_usec << 10);
#endif
#ifdef SYS_NT
    HCRYPTPROV handle;
    CHECKED(CryptAcquireContext(&handle, nullptr, nullptr, PROV_RSA_FULL, 0),
	    "can't acquire crypt context");
    CHECKED(CryptGenRandom(handle, 4, reinterpret_cast<BYTE *>(&seed)),
	    "CryptGenRandom failure");
    CHECKED(CryptReleaseContext(handle, 0), "can't release crypt context");
#endif
  }
  init(seed);
}

MersenneTwisterInternal::MersenneTwisterInternal(std::vector<uint32_t> seed_array)
{
    initArray(seed_array);
}

void
MersenneTwisterInternal::init(uint32_t seed)
{
    seed_used = seed;
    mt[0]= seed & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
#if 0
    // This was the old generator.
    int i;

    for (i=0;i<N;i++) {
         mt[i] = seed & 0xffff0000;
         seed = 69069 * seed + 1;
         mt[i] |= (seed & 0xffff0000) >> 16;
         seed = 69069 * seed + 1;
    }
    mti = N;
#endif
}

/* Initialization by "sgenrand()" is an example. Theoretically,      */
/* there are 2^19937-1 possible states as an intial state.           */
/* This function allows to choose any of 2^19937-1 ones.             */
/* Essential bits in "seed_array[]" is following 19937 bits:         */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]. */
/* (seed_array[0]&LOWER_MASK) is discarded.                          */
/* Theoretically,                                                    */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]  */
/* can take any values except all zeros.                             */
void
MersenneTwisterInternal::initArray(std::vector<uint32_t> seed_array)
{
    int i, j, k;
    init(19650218UL);
    i=1; j=0;
    k = (N>(int)seed_array.size() ? N : seed_array.size());
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + seed_array[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=(int)seed_array.size()) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    // only high bit of mt[0] is relevant.
    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
#if 0
    // This was the old array initialization code
    int i,j;
    bool any_nonzero = false;

    if (seed_array.size() == 0) {
	init(4357);
    } else {
	seed_used = 0;
	j = 0;
	for (i=0;i<N;i++) {
	    if ((int)seed_array.size() == j)
		j = 0;
	    mt[i] = seed_array[j];
	    if (mt[i] != 0) {
		any_nonzero = true;
	    }
	}
    }
    INVARIANT(any_nonzero, "Must have a non-zero entry in seed_array!\n");
    mti=N;
#endif
}

void
MersenneTwisterInternal::reloadArray()
{
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    uint32_t y;
    int kk;

    for (kk=0;kk<N-M;kk++) {
	y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for (;kk<N-1;kk++) {
	y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

    mti = 0;
}

#if 0
static uint32_t arrayCheck[] = {
1067595299UL,  955945823UL,  477289528UL, 4107218783UL,	4228976476UL,
3344332714UL, 3355579695UL,  227628506UL,  810200273UL,	2591290167UL,
2560260675UL, 3242736208UL,  646746669UL, 1479517882UL,	4245472273UL,
1143372638UL, 3863670494UL, 3221021970UL, 1773610557UL,	1138697238UL,
1421897700UL, 1269916527UL, 2859934041UL, 1764463362UL,	3874892047UL,
3965319921UL,   72549643UL, 2383988930UL, 2600218693UL,	3237492380UL,
2792901476UL,  725331109UL,  605841842UL,  271258942UL,	 715137098UL,
3297999536UL, 1322965544UL, 4229579109UL, 1395091102UL,	3735697720UL,
2101727825UL, 3730287744UL, 2950434330UL, 1661921839UL,	2895579582UL,
2370511479UL, 1004092106UL, 2247096681UL, 2111242379UL,	3237345263UL,
4082424759UL,  219785033UL, 2454039889UL, 3709582971UL,	 835606218UL,
2411949883UL, 2735205030UL,  756421180UL, 2175209704UL,	1873865952UL,
2762534237UL, 4161807854UL, 3351099340UL,  181129879UL,	3269891896UL,
 776029799UL, 2218161979UL, 3001745796UL, 1866825872UL,	2133627728UL,
  34862734UL, 1191934573UL, 3102311354UL, 2916517763UL,	1012402762UL,
2184831317UL, 4257399449UL, 2899497138UL, 3818095062UL,	3030756734UL,
1282161629UL,  420003642UL, 2326421477UL, 2741455717UL,	1278020671UL,
3744179621UL,  271777016UL, 2626330018UL, 2560563991UL,	3055977700UL,
4233527566UL, 1228397661UL, 3595579322UL, 1077915006UL,	2395931898UL,
1851927286UL, 3013683506UL, 1999971931UL, 3006888962UL,	1049781534UL,
1488758959UL, 3491776230UL,  104418065UL, 2448267297UL,	3075614115UL,
3872332600UL,  891912190UL, 3936547759UL, 2269180963UL,	2633455084UL,
1047636807UL, 2604612377UL, 2709305729UL, 1952216715UL,	 207593580UL,
2849898034UL,  670771757UL, 2210471108UL,  467711165UL,	 263046873UL,
3569667915UL, 1042291111UL, 3863517079UL, 1464270005UL,	2758321352UL,
3790799816UL, 2301278724UL, 3106281430UL,    7974801UL,	2792461636UL,
 555991332UL,  621766759UL, 1322453093UL,  853629228UL,	 686962251UL,
1455120532UL,  957753161UL, 1802033300UL, 1021534190UL,	3486047311UL,
1902128914UL, 3701138056UL, 4176424663UL, 1795608698UL,	 560858864UL,
3737752754UL, 3141170998UL, 1553553385UL, 3367807274UL,	 711546358UL,
2475125503UL,  262969859UL,  251416325UL, 2980076994UL,	1806565895UL,
 969527843UL, 3529327173UL, 2736343040UL, 2987196734UL,	1649016367UL,
2206175811UL, 3048174801UL, 3662503553UL, 3138851612UL,	2660143804UL,
1663017612UL, 1816683231UL,  411916003UL, 3887461314UL,	2347044079UL,
1015311755UL, 1203592432UL, 2170947766UL, 2569420716UL,	 813872093UL,
1105387678UL, 1431142475UL,  220570551UL, 4243632715UL,	4179591855UL,
2607469131UL, 3090613241UL,  282341803UL, 1734241730UL,	1391822177UL,
1001254810UL,  827927915UL, 1886687171UL, 3935097347UL,	2631788714UL,
3905163266UL,  110554195UL, 2447955646UL, 3717202975UL,	3304793075UL,
3739614479UL, 3059127468UL,  953919171UL, 2590123714UL,	1132511021UL,
3795593679UL, 2788030429UL,  982155079UL, 3472349556UL,	 859942552UL,
2681007391UL, 2299624053UL,  647443547UL,  233600422UL,	 608168955UL,
3689327453UL, 1849778220UL, 1608438222UL, 3968158357UL,	2692977776UL,
2851872572UL,  246750393UL, 3582818628UL, 3329652309UL,	4036366910UL,
1012970930UL,  950780808UL, 3959768744UL, 2538550045UL,	 191422718UL,
2658142375UL, 3276369011UL, 2927737484UL, 1234200027UL,	1920815603UL,
3536074689UL, 1535612501UL, 2184142071UL, 3276955054UL,	 428488088UL,
2378411984UL, 4059769550UL, 3913744741UL, 2732139246UL,	  64369859UL,
3755670074UL,  842839565UL, 2819894466UL, 2414718973UL,	1010060670UL,
1839715346UL, 2410311136UL,  152774329UL, 3485009480UL,	4102101512UL,
2852724304UL,  879944024UL, 1785007662UL, 2748284463UL,	1354768064UL,
3267784736UL, 2269127717UL, 3001240761UL, 3179796763UL,	 895723219UL,
 865924942UL, 4291570937UL,   89355264UL, 1471026971UL,	4114180745UL,
3201939751UL, 2867476999UL, 2460866060UL, 3603874571UL,	2238880432UL,
3308416168UL, 2072246611UL, 2755653839UL, 3773737248UL,	1709066580UL,
4282731467UL, 2746170170UL, 2832568330UL,  433439009UL,	3175778732UL,
  26248366UL, 2551382801UL,  183214346UL, 3893339516UL,	1928168445UL,
1337157619UL, 3429096554UL, 3275170900UL, 1782047316UL,	4264403756UL,
1876594403UL, 4289659572UL, 3223834894UL, 1728705513UL,	4068244734UL,
2867840287UL, 1147798696UL,  302879820UL, 1730407747UL,	1923824407UL,
1180597908UL, 1569786639UL,  198796327UL,  560793173UL,	2107345620UL,
2705990316UL, 3448772106UL, 3678374155UL,  758635715UL,	 884524671UL,
 486356516UL, 1774865603UL, 3881226226UL, 2635213607UL,	1181121587UL,
1508809820UL, 3178988241UL, 1594193633UL, 1235154121UL,	 326117244UL,
2304031425UL,  937054774UL, 2687415945UL, 3192389340UL,	2003740439UL,
1823766188UL, 2759543402UL,   10067710UL, 1533252662UL,	4132494984UL,
  82378136UL,  420615890UL, 3467563163UL,  541562091UL,	3535949864UL,
2277319197UL, 3330822853UL, 3215654174UL, 4113831979UL,	4204996991UL,
2162248333UL, 3255093522UL, 2219088909UL, 2978279037UL,	 255818579UL,
2859348628UL, 3097280311UL, 2569721123UL, 1861951120UL,	2907080079UL,
2719467166UL,  998319094UL, 2521935127UL, 2404125338UL,	 259456032UL,
2086860995UL, 1839848496UL, 1893547357UL, 2527997525UL,	1489393124UL,
2860855349UL,   76448234UL, 2264934035UL,  744914583UL,	2586791259UL,
1385380501UL,   66529922UL, 1819103258UL, 1899300332UL,	2098173828UL,
1793831094UL,  276463159UL,  360132945UL, 4178212058UL,	 595015228UL,
 177071838UL, 2800080290UL, 1573557746UL, 1548998935UL,	 378454223UL,
1460534296UL, 1116274283UL, 3112385063UL, 3709761796UL,	 827999348UL,
3580042847UL, 1913901014UL,  614021289UL, 4278528023UL,	1905177404UL,
  45407939UL, 3298183234UL, 1184848810UL, 3644926330UL,	3923635459UL,
1627046213UL, 3677876759UL,  969772772UL, 1160524753UL,	1522441192UL,
 452369933UL, 1527502551UL,  832490847UL, 1003299676UL,	1071381111UL,
2891255476UL,  973747308UL, 4086897108UL, 1847554542UL,	3895651598UL,
2227820339UL, 1621250941UL, 2881344691UL, 3583565821UL,	3510404498UL,
 849362119UL,  862871471UL,  797858058UL, 2867774932UL,	2821282612UL,
3272403146UL, 3997979905UL,  209178708UL, 1805135652UL,	   6783381UL,
2823361423UL,  792580494UL, 4263749770UL,  776439581UL,	3798193823UL,
2853444094UL, 2729507474UL, 1071873341UL, 1329010206UL,	1289336450UL,
3327680758UL, 2011491779UL,   80157208UL,  922428856UL,	1158943220UL,
1667230961UL, 2461022820UL, 2608845159UL,  387516115UL,	3345351910UL,
1495629111UL, 4098154157UL, 3156649613UL, 3525698599UL,	4134908037UL,
 446713264UL, 2137537399UL, 3617403512UL,  813966752UL,	1157943946UL,
3734692965UL, 1680301658UL, 3180398473UL, 3509854711UL,	2228114612UL,
1008102291UL,  486805123UL,  863791847UL, 3189125290UL,	1050308116UL,
3777341526UL, 4291726501UL,  844061465UL, 1347461791UL,	2826481581UL,
 745465012UL, 2055805750UL, 4260209475UL, 2386693097UL,	2980646741UL,
 447229436UL, 2077782664UL, 1232942813UL, 4023002732UL,	1399011509UL,
3140569849UL, 2579909222UL, 3794857471UL,  900758066UL,	2887199683UL,
1720257997UL, 3367494931UL, 2668921229UL,  955539029UL,	3818726432UL,
1105704962UL, 3889207255UL, 2277369307UL, 2746484505UL,	1761846513UL,
2413916784UL, 2685127085UL, 4240257943UL, 1166726899UL,	4215215715UL,
3082092067UL, 3960461946UL, 1663304043UL, 2087473241UL,	4162589986UL,
2507310778UL, 1579665506UL,  767234210UL,  970676017UL,	 492207530UL,
1441679602UL, 1314785090UL, 3262202570UL, 3417091742UL,	1561989210UL,
3011406780UL, 1146609202UL, 3262321040UL, 1374872171UL,	1634688712UL,
1280458888UL, 2230023982UL,  419323804UL, 3262899800UL,	  39783310UL,
1641619040UL, 1700368658UL, 2207946628UL, 2571300939UL,	2424079766UL,
 780290914UL, 2715195096UL, 3390957695UL,  163151474UL,	2309534542UL,
1860018424UL,  555755123UL,  280320104UL, 1604831083UL,	2713022383UL,
1728987441UL, 3639955502UL,  623065489UL, 3828630947UL,	4275479050UL,
3516347383UL, 2343951195UL, 2430677756UL,  635534992UL,	3868699749UL,
 808442435UL, 3070644069UL, 4282166003UL, 2093181383UL,	2023555632UL,
1568662086UL, 3422372620UL, 4134522350UL, 3016979543UL,	3259320234UL,
2888030729UL, 3185253876UL, 4258779643UL, 1267304371UL,	1022517473UL,
 815943045UL,  929020012UL, 2995251018UL, 3371283296UL,	3608029049UL,
2018485115UL,  122123397UL, 2810669150UL, 1411365618UL,	1238391329UL,
1186786476UL, 3155969091UL, 2242941310UL, 1765554882UL,	 279121160UL,
4279838515UL, 1641578514UL, 3796324015UL,   13351065UL,	 103516986UL,
1609694427UL,  551411743UL, 2493771609UL, 1316337047UL,	3932650856UL,
4189700203UL,  463397996UL, 2937735066UL, 1855616529UL,	2626847990UL,
  55091862UL, 3823351211UL,  753448970UL, 4045045500UL,	1274127772UL,
1124182256UL,   92039808UL, 2126345552UL,  425973257UL,	 386287896UL,
2589870191UL, 1987762798UL, 4084826973UL, 2172456685UL,	3366583455UL,
3602966653UL, 2378803535UL, 2901764433UL, 3716929006UL,	3710159000UL,
2653449155UL, 3469742630UL, 3096444476UL, 3932564653UL,	2595257433UL,
 318974657UL, 3146202484UL,  853571438UL,  144400272UL,	3768408841UL,
 782634401UL, 2161109003UL,  570039522UL, 1886241521UL,	  14249488UL,
2230804228UL, 1604941699UL, 3928713335UL, 3921942509UL,	2155806892UL,
 134366254UL,  430507376UL, 1924011722UL,  276713377UL,	 196481886UL,
3614810992UL, 1610021185UL, 1785757066UL,  851346168UL,	3761148643UL,
2918835642UL, 3364422385UL, 3012284466UL, 3735958851UL,	2643153892UL,
3778608231UL, 1164289832UL,  205853021UL, 2876112231UL,	3503398282UL,
3078397001UL, 3472037921UL, 1748894853UL, 2740861475UL,	 316056182UL,
1660426908UL,  168885906UL,  956005527UL, 3984354789UL,	 566521563UL,
1001109523UL, 1216710575UL, 2952284757UL, 3834433081UL,	3842608301UL,
2467352408UL, 3974441264UL, 3256601745UL, 1409353924UL,	1329904859UL,
2307560293UL, 3125217879UL, 3622920184UL, 3832785684UL,	3882365951UL,
2308537115UL, 2659155028UL, 1450441945UL, 3532257603UL,	3186324194UL,
1225603425UL, 1124246549UL,  175808705UL, 3009142319UL,	2796710159UL,
3651990107UL,  160762750UL, 1902254979UL, 1698648476UL,	1134980669UL,
 497144426UL, 3302689335UL, 4057485630UL, 3603530763UL,	4087252587UL,
 427812652UL,  286876201UL,  823134128UL, 1627554964UL,	3745564327UL,
2589226092UL, 4202024494UL,   62878473UL, 3275585894UL,	3987124064UL,
2791777159UL, 1916869511UL, 2585861905UL, 1375038919UL,	1403421920UL,
  60249114UL, 3811870450UL, 3021498009UL, 2612993202UL,	 528933105UL,
2757361321UL, 3341402964UL, 2621861700UL,  273128190UL,	4015252178UL,
3094781002UL, 1621621288UL, 2337611177UL, 1796718448UL,	1258965619UL,
4241913140UL, 2138560392UL, 3022190223UL, 4174180924UL,	 450094611UL,
3274724580UL,  617150026UL, 2704660665UL, 1469700689UL,	1341616587UL,
 356715071UL, 1188789960UL, 2278869135UL, 1766569160UL,	2795896635UL,
  57824704UL, 2893496380UL, 1235723989UL, 1630694347UL,	3927960522UL,
 428891364UL, 1814070806UL, 2287999787UL, 4125941184UL,	3968103889UL,
3548724050UL, 1025597707UL, 1404281500UL, 2002212197UL,	  92429143UL,
2313943944UL, 2403086080UL, 3006180634UL, 3561981764UL,	1671860914UL,
1768520622UL, 1803542985UL,  844848113UL, 3006139921UL,	1410888995UL,
1157749833UL, 2125704913UL, 1789979528UL, 1799263423UL,	 741157179UL,
2405862309UL,  767040434UL, 2655241390UL, 3663420179UL,	2172009096UL,
2511931187UL, 1680542666UL,  231857466UL, 1154981000UL,	 157168255UL,
1454112128UL, 3505872099UL, 1929775046UL, 2309422350UL,	2143329496UL,
2960716902UL,  407610648UL, 2938108129UL, 2581749599UL,	 538837155UL,
2342628867UL,  430543915UL,  740188568UL, 1937713272UL,	3315215132UL,
2085587024UL, 4030765687UL,  766054429UL, 3517641839UL,	 689721775UL,
1294158986UL, 1753287754UL, 4202601348UL, 1974852792UL,	  33459103UL,
3568087535UL, 3144677435UL, 1686130825UL, 4134943013UL,	3005738435UL,
3599293386UL,  426570142UL,  754104406UL, 3660892564UL,	1964545167UL,
 829466833UL,  821587464UL, 1746693036UL, 1006492428UL,	1595312919UL,
1256599985UL, 1024482560UL, 1897312280UL, 2902903201UL,	 691790057UL,
1037515867UL, 3176831208UL, 1968401055UL, 2173506824UL,	1089055278UL,
1748401123UL, 2941380082UL,  968412354UL, 1818753861UL,	2973200866UL,
3875951774UL, 1119354008UL, 3988604139UL, 1647155589UL,	2232450826UL,
3486058011UL, 3655784043UL, 3759258462UL,  847163678UL,	1082052057UL,
 989516446UL, 2871541755UL, 3196311070UL, 3929963078UL,	 658187585UL,
3664944641UL, 2175149170UL, 2203709147UL, 2756014689UL,	2456473919UL,
3890267390UL, 1293787864UL, 2830347984UL, 3059280931UL,	4158802520UL,
1561677400UL, 2586570938UL,  783570352UL, 1355506163UL,	  31495586UL,
3789437343UL, 3340549429UL, 2092501630UL,  896419368UL,	 671715824UL,
3530450081UL, 3603554138UL, 1055991716UL, 3442308219UL,	1499434728UL,
3130288473UL, 3639507000UL,   17769680UL, 2259741420UL,	 487032199UL,
4227143402UL, 3693771256UL, 1880482820UL, 3924810796UL,	 381462353UL,
4017855991UL, 2452034943UL, 2736680833UL, 2209866385UL,	2128986379UL,
 437874044UL,  595759426UL,  641721026UL, 1636065708UL,	3899136933UL,
 629879088UL, 3591174506UL,  351984326UL, 2638783544UL,	2348444281UL,
2341604660UL, 2123933692UL,  143443325UL, 1525942256UL,	 364660499UL,
 599149312UL,  939093251UL, 1523003209UL,  106601097UL,	 376589484UL,
1346282236UL, 1297387043UL,  764598052UL, 3741218111UL,	 933457002UL,
1886424424UL, 3219631016UL,  525405256UL, 3014235619UL,	 323149677UL,
2038881721UL, 4100129043UL, 2851715101UL, 2984028078UL,	1888574695UL,
2014194741UL, 3515193880UL, 4180573530UL, 3461824363UL,	2641995497UL,
3179230245UL, 2902294983UL, 2217320456UL, 4040852155UL,	1784656905UL,
3311906931UL,   87498458UL, 2752971818UL, 2635474297UL,	2831215366UL,
3682231106UL, 2920043893UL, 3772929704UL, 2816374944UL,	 309949752UL,
2383758854UL,  154870719UL,  385111597UL, 1191604312UL,	1840700563UL,
 872191186UL, 2925548701UL, 1310412747UL, 2102066999UL,	1504727249UL,
3574298750UL, 1191230036UL, 3330575266UL, 3180292097UL,	3539347721UL,
 681369118UL, 3305125752UL, 3648233597UL,  950049240UL,	4173257693UL,
1760124957UL,  512151405UL,  681175196UL,  580563018UL,	1169662867UL,
4015033554UL, 2687781101UL,  699691603UL, 2673494188UL,	1137221356UL,
 123599888UL,  472658308UL, 1053598179UL, 1012713758UL,	3481064843UL,
3759461013UL, 3981457956UL, 3830587662UL, 1877191791UL,	3650996736UL,
 988064871UL, 3515461600UL, 4089077232UL, 2225147448UL,	1249609188UL,
2643151863UL, 3896204135UL, 2416995901UL, 1397735321UL,	3460025646UL
};
static int arrayCheck_count = sizeof(arrayCheck)/sizeof(uint32_t);

static uint32_t seedCheck[] = {
3499211612UL,  581869302UL, 3890346734UL, 3586334585UL,	 545404204UL,
4161255391UL, 3922919429UL,  949333985UL, 2715962298UL,	1323567403UL,
 418932835UL, 2350294565UL, 1196140740UL,  809094426UL,	2348838239UL,
4264392720UL, 4112460519UL, 4279768804UL, 4144164697UL,	4156218106UL,
 676943009UL, 3117454609UL, 4168664243UL, 4213834039UL,	4111000746UL,
 471852626UL, 2084672536UL, 3427838553UL, 3437178460UL,	1275731771UL,
 609397212UL,   20544909UL, 1811450929UL,  483031418UL,	3933054126UL,
2747762695UL, 3402504553UL, 3772830893UL, 4120988587UL,	2163214728UL,
2816384844UL, 3427077306UL,  153380495UL, 1551745920UL,	3646982597UL,
 910208076UL, 4011470445UL, 2926416934UL, 2915145307UL,	1712568902UL,
3254469058UL, 3181055693UL, 3191729660UL, 2039073006UL,	1684602222UL,
1812852786UL, 2815256116UL,  746745227UL,  735241234UL,	1296707006UL,
3032444839UL, 3424291161UL,  136721026UL, 1359573808UL,	1189375152UL,
3747053250UL,  198304612UL,  640439652UL,  417177801UL,	4269491673UL,
3536724425UL, 3530047642UL, 2984266209UL,  537655879UL,	1361931891UL,
3280281326UL, 4081172609UL, 2107063880UL,  147944788UL,	2850164008UL,
1884392678UL,  540721923UL, 1638781099UL,  902841100UL,	3287869586UL,
 219972873UL, 3415357582UL,  156513983UL,  802611720UL,	1755486969UL,
2103522059UL, 1967048444UL, 1913778154UL, 2094092595UL,	2775893247UL,
3410096536UL, 3046698742UL, 3955127111UL, 3241354600UL,	3468319344UL,
1185518681UL, 3031277329UL, 2919300778UL,   12105075UL,	2813624502UL,
3052449900UL,  698412071UL, 2765791248UL,  511091141UL,	1958646067UL,
2140457296UL, 3323948758UL, 4122068897UL, 2464257528UL,	1461945556UL,
3765644424UL, 2513705832UL, 3471087299UL,  961264978UL,	  76338300UL,
3226667454UL, 3527224675UL, 1095625157UL, 3525484323UL,	2173068963UL,
4037587209UL, 3002511655UL, 1772389185UL, 3826400342UL,	1817480335UL,
4120125281UL, 2495189930UL, 2350272820UL,  678852156UL,	 595387438UL,
3271610651UL,  641212874UL,  988512770UL, 1105989508UL,	3477783405UL,
3610853094UL, 4245667946UL, 1092133642UL, 1427854500UL,	3497326703UL,
1287767370UL, 1045931779UL,   58150106UL, 3991156885UL,	 933029415UL,
1503168825UL, 3897101788UL,  844370145UL, 3644141418UL,	1078396938UL,
4101769245UL, 2645891717UL, 3345340191UL, 2032760103UL,	4241106803UL,
1510366103UL,  290319951UL, 3568381791UL, 3408475658UL,	2513690134UL,
2553373352UL, 2361044915UL, 3147346559UL, 3939316793UL,	2986002498UL,
1227669233UL, 2919803768UL, 3252150224UL, 1685003584UL,	3237241796UL,
2411870849UL, 1634002467UL,  893645500UL, 2438775379UL,	2265043167UL,
 325791709UL, 1736062366UL,  231714000UL, 1515103006UL,	2279758133UL,
2546159170UL, 3346497776UL, 1530490810UL, 4011545318UL,	4144499009UL,
 557942923UL,  663307952UL, 2443079012UL, 1696117849UL,	2016017442UL,
1663423246UL,   51119001UL, 3122246755UL, 1447930741UL,	1668894615UL,
 696567687UL, 3983551422UL, 3411426125UL, 1873110678UL,	1336658413UL,
3705174600UL, 2270032533UL, 2664425968UL,  711455903UL,	 513451233UL,
2585492744UL, 2027039028UL, 1129453058UL, 1461232481UL,	2809248324UL,
2275654012UL, 2960153730UL, 3075629128UL, 3213286615UL,	4245057188UL,
1935061435UL, 3094495853UL,  360010077UL, 3919490483UL,	 983448591UL,
2171099548UL, 3922754098UL, 2397746050UL,  654458600UL,	2161184684UL,
3546856898UL, 1986311591UL, 2312163142UL, 2347594600UL,	4278366025UL,
1922360368UL,  335761339UL, 3669839044UL, 1901288696UL,	2595154464UL,
 458070173UL, 2141230976UL, 4131320786UL, 4208748424UL,	  19903848UL,
 147391738UL, 3328215103UL, 4196191786UL, 3510290616UL,	1559873971UL,
3731015357UL, 2918514861UL,  362649214UL, 1487061100UL,	1717053387UL,
3675955720UL, 1116134897UL,  193529268UL, 3436267940UL,	2835191639UL,
1852908272UL, 3220971953UL, 3911201640UL,  571213604UL,	 781027019UL,
4219206494UL, 1133024903UL,  409547355UL,  625085180UL,	1214072539UL,
 584409985UL, 3445042528UL, 3733581611UL,  333104904UL,	2489812253UL,
2694595213UL, 2361631596UL,   34763086UL,  622576118UL,	2921810672UL,
3663740744UL, 2293225236UL, 2671706445UL, 1884059696UL,	1507329019UL,
 857065948UL, 2204390003UL,  592711182UL, 1725752375UL,	1642107460UL,
 326274448UL, 3274574484UL, 1030432041UL,  173822100UL,	 529650788UL,
1086437636UL,  789877945UL, 2167974914UL, 1030588245UL,	3533061365UL,
1792148406UL, 4216468704UL,  213264131UL, 3536714075UL,	3877136173UL,
1296338417UL, 4057830103UL,  205919137UL, 2108245233UL,	1064497347UL,
2101324080UL, 2336703164UL, 1450493809UL, 3812754708UL,	3865701845UL,
1476779561UL, 1585902852UL,  142887412UL,  477612192UL,	 699530444UL,
3351157089UL, 3768249319UL, 1673915577UL,  903239649UL,	1038056164UL,
1171465372UL, 1734789440UL, 2115022236UL,  414269055UL,	 959581346UL,
 566820984UL, 2105828892UL, 4046076449UL, 4101450561UL,	4106566571UL,
2800184123UL, 2470502098UL, 3253453343UL,  256751188UL,	1869365987UL,
1008372035UL, 2374606708UL, 1516804538UL,  228288551UL,	3527001547UL,
1385173098UL,   66157275UL, 1739381798UL,  184785808UL,	3901692666UL,
 725806641UL, 3475217997UL, 2787929747UL, 1109372433UL,	3142723729UL,
 557686578UL, 2782047723UL, 2118822689UL, 1936702581UL,	1625646963UL,
2349385293UL, 3085804937UL, 1272688179UL, 1236112995UL,	3198431244UL,
2677635414UL,  811555596UL, 3486972196UL, 2949678043UL,	1342211552UL,
 788174404UL, 1656614077UL, 1582629285UL, 1477167035UL,	2687011245UL,
3503701453UL, 3351051324UL, 2874557775UL,  348432514UL,	1629591495UL,
3991682351UL, 1969229192UL, 3331660584UL, 1304012077UL,	2090754125UL,
3910846836UL, 1871998370UL, 2098597104UL, 1918921592UL,	3246092887UL,
1315760974UL,  464122393UL, 2184028058UL, 1690455542UL,	2193747147UL,
3737423698UL, 3511684278UL, 1549884962UL, 3413774919UL,	3938991454UL,
2767325310UL, 2335626851UL, 1626114941UL,  601913200UL,	3485711542UL,
 858447440UL, 2288468476UL, 4075602213UL, 1506361431UL,	4252489875UL,
4032981007UL, 1031118352UL, 3762145731UL,   70955369UL,	2362903502UL,
1669089455UL, 2673510137UL, 3348740333UL, 2521337794UL,	2047144929UL,
 892246357UL, 2319875070UL, 1293843163UL,   79245769UL,	2022600352UL,
3866257397UL,  989939126UL,  835351312UL, 3626278636UL,	3805332945UL,
 836506264UL, 1895040349UL,  970326679UL,  634920763UL,	 733185481UL,
1028655248UL,  977810701UL, 3434484235UL, 1871311609UL,	2031584214UL,
1336174158UL,  385787519UL, 3965885375UL, 2768323462UL,	1847726660UL,
2718987737UL,  793780050UL, 2509902580UL, 3886434164UL,	3120956802UL,
4207987247UL, 1523159183UL, 1884932179UL, 2922324286UL,	 477253416UL,
3037922812UL, 1108379444UL,  697195677UL, 1755438379UL,	 574393398UL,
2555059183UL, 1930828628UL, 1126190880UL,  180621093UL,	2589191337UL,
3424652760UL, 3054648512UL,  719646637UL,  952394946UL,	3570038180UL,
 504304985UL, 1395707758UL, 1274213163UL, 2816553213UL,	1369142370UL,
1804702100UL, 1821782344UL, 3358274235UL, 2181234724UL,	 486158240UL,
 367287522UL, 4267199121UL, 1127352639UL,  779850007UL,	3440331597UL,
3276765484UL,  125500149UL, 1142120513UL, 3989398167UL,	1048565860UL,
3136747194UL,  432668526UL, 2098559576UL, 1478877150UL,	2484746208UL,
1209580219UL, 1019125185UL, 4160278734UL, 1970740713UL,	 918146921UL,
4136433784UL, 2602441845UL, 2348512686UL,  973030509UL,	2238261365UL,
 815637919UL,  994690313UL, 1724736366UL, 2099799816UL,	1775069742UL,
2680317667UL,  730798472UL, 2916864943UL, 1284417767UL,	1698724919UL,
2733611686UL, 1578128411UL,  651006053UL, 4243350375UL,	3303874296UL,
 162087183UL, 3796616231UL, 3801767645UL, 4119825424UL,	3922537059UL,
  77594039UL, 3419583692UL, 2503306160UL,  423966005UL,	3293613218UL,
1124728190UL, 1407880681UL, 1440346680UL,  554334954UL,	2919409323UL,
1253962019UL,  586491243UL, 3638308238UL, 3097648541UL,	 991125519UL,
 458538714UL, 2155963569UL, 2807866455UL,    6862945UL,	2122460897UL,
  53853750UL, 3346001678UL, 1230879976UL, 3071060893UL,	 423909157UL,
3881450262UL, 1652511030UL, 3826483009UL, 1526211009UL,	1435219366UL,
3092251623UL, 3001090498UL,  281084412UL,  849586749UL,	2207008400UL,
 131172352UL, 1820973075UL, 3195774605UL, 2962673849UL,	2147580010UL,
1090677336UL, 2061249893UL, 1724513375UL, 3885752424UL,	1135918139UL,
2619357288UL, 4012575714UL, 2652856935UL, 2029480458UL,	3691276589UL,
2623865075UL, 3459550738UL, 2097670126UL, 2477000057UL,	2209844713UL,
 785646024UL, 1052349661UL, 1030500157UL, 1430246618UL,	3807539761UL,
2157629976UL,  123154542UL, 2560049331UL, 2104110449UL,	1332109867UL,
 721241591UL, 4136042859UL, 4203401395UL,  998151922UL,	3060999432UL,
3207929139UL, 2149509272UL, 1385268511UL, 2023309182UL,	1366796638UL,
 256061060UL, 4090836236UL, 2929047008UL, 2296609403UL,	 182240337UL,
3744374619UL,  306855912UL, 4014087816UL, 2240468995UL,	2865233169UL,
 415452309UL, 1244206523UL, 3513921306UL,  281425419UL,	3511338031UL,
 995954022UL, 3102854413UL, 3026765331UL,  643667197UL,	 837979907UL,
2832983005UL, 1813414171UL, 2227348307UL, 4020325887UL,	4178893912UL,
 610818241UL, 2787397224UL, 2762441380UL, 3437393657UL,	2030369078UL,
1949046312UL, 1876612561UL, 1857107382UL, 1049344864UL,	3544695775UL,
2172907342UL,  358500115UL, 3895295219UL,  571965125UL,	 328582064UL,
 744698407UL, 3066193991UL, 1679065087UL, 2650874932UL,	3570748805UL,
 812110431UL, 3450423805UL, 1705023874UL,  259721746UL,	1192558045UL,
1714799045UL, 3685508436UL, 2262914445UL, 3903852862UL,	1790140070UL,
2651193482UL, 2821191752UL,  776610414UL, 2697125035UL,	2212010032UL,
1254062056UL, 3541766210UL, 1853927671UL, 1543286708UL,	  66516686UL,
3505195914UL, 4226521519UL, 1260092911UL,  717982876UL,	 739240369UL,
 456195732UL, 2116515161UL, 1599487648UL,  838913496UL,	 850912042UL,
3712172413UL, 2103192411UL,  877020153UL, 1458113119UL,	2646869271UL,
4087221703UL, 3771198399UL, 3952796001UL, 1685641891UL,	 226245966UL,
4065518354UL, 3169076409UL,  715963611UL, 1155859114UL,	4174181651UL,
1816065125UL, 2422210778UL, 2353087594UL, 2569974907UL,	4049024520UL,
 563593555UL, 1794197249UL, 2434290377UL, 4222178191UL,	2381045132UL,
1294739153UL, 1333544226UL, 3011196239UL,  518183212UL,	2861903570UL,
3168787443UL, 2315530531UL, 1042490149UL, 2998340365UL,	3534153126UL,
2862715604UL,  796613230UL,  765073073UL, 1342937225UL,	 549817636UL,
3786981820UL, 4291017601UL, 2895722553UL,  734959362UL,	3175258828UL,
 140019477UL,  268621172UL, 2410334776UL,  565052604UL,	3787587805UL,
 386344800UL, 2874086067UL,   35710270UL,  817904650UL,	1960697289UL,
1584484509UL, 2724312018UL, 1978802819UL, 2275314726UL,	4216102886UL,
2138332912UL,  671754166UL, 1442240992UL, 3674442465UL,	1085868016UL,
2769242611UL, 1003628378UL, 1616076847UL,  743729558UL,	 820011032UL,
2559719034UL, 1839332599UL, 3121982280UL, 2070268989UL,	3769147733UL,
 518022934UL, 3037227899UL, 2531915367UL, 1008310588UL,	 971468687UL,
2052976098UL, 1651926578UL,   78218926UL, 2503907441UL,	3209763057UL,
1081499040UL, 2812016370UL, 1247433164UL,  335294964UL,	2650385171UL,
2030527826UL, 1139372809UL, 4279827824UL, 3540669095UL,	2285341455UL,
4220507154UL, 3863048231UL, 3136394663UL, 3319584205UL,	1476940506UL,
 875141230UL, 2508558662UL, 3896001866UL,  462864388UL,	1609807693UL,
3892563868UL, 3642514037UL, 3778083990UL, 1403162576UL,	3512254868UL,
1403323269UL, 1119818229UL, 2831288053UL, 2552740643UL,	2520136409UL,
  96690857UL,  210381252UL, 1826474872UL, 3306977352UL,	1343117402UL,
2112059492UL,  693571694UL, 2096734379UL,  767794921UL,	1843084587UL,
1816280216UL, 1695342628UL,  404711915UL, 3334843684UL,	2570639553UL,
4186538211UL, 2022604264UL, 3214805180UL, 2989079529UL,	2725165355UL,
3005995436UL,  310011850UL, 2742468706UL, 2720274646UL,	 144327376UL,
2271696819UL,  295519962UL, 1272030376UL, 1372670420UL,	1397272558UL,
2280044719UL, 2710639434UL, 2810822904UL, 4271368265UL,	1750711132UL,
2216408539UL, 3521792518UL, 3111505866UL, 3085328191UL,	1054735512UL,
4160317205UL, 1427385632UL, 2282061755UL, 3215251668UL,	1396490078UL,
2933318719UL,  453673969UL, 2926038256UL, 2624047458UL,	 338625410UL,
3344930154UL, 1971116345UL, 1818716442UL, 2998517928UL,	 390083048UL,
 291563131UL, 1144486353UL,  296954266UL,  659950561UL,	2263631666UL,
1206908601UL, 1125491020UL, 1890151284UL, 2076080514UL,	2264060846UL,
 561805191UL, 1964622705UL,  405620012UL, 3759692386UL,	 517035386UL,
2225016848UL, 4165419081UL, 4052828294UL, 3248204933UL,	2738939733UL,
1151808775UL, 4113264137UL, 3113447491UL, 1033828852UL,	1785686386UL,
2903923175UL, 2038900010UL, 1241522880UL,  238119113UL,	2885394101UL,
2636011022UL, 2985605703UL, 2107193353UL,  292026696UL,	3884689974UL,
1094315383UL, 4016714705UL,  962244585UL, 3943968050UL,	2868319718UL,
1304919603UL, 3626636694UL, 3393461291UL, 1479454799UL,	 971639318UL,
3352306399UL, 1928233566UL, 2900529135UL, 2190901098UL,	  28842068UL,
 990556577UL, 2586302532UL, 3057504668UL, 1661169605UL,	4228191763UL,
3934152427UL, 2814119472UL,    4943754UL, 1171095774UL,	1986204006UL,
2014406505UL, 1822565279UL,   12890078UL, 1979620724UL,	1917376192UL,
3307810835UL, 4170173371UL, 1385005883UL, 1308519769UL,	3370429606UL,
 923886311UL, 2024463563UL, 1063369787UL,  153599761UL,	3463680785UL,
 755374878UL, 2088947962UL, 3099927142UL, 1750207400UL,	2033606872UL,
 926120766UL,  655932557UL, 2320365045UL, 1465119024UL,	3105365454UL,
2608716819UL, 1218456091UL,  823539591UL, 2331574954UL,	3171519129UL,
3246671799UL, 1043031086UL, 1425831588UL, 3940307546UL,	3443545749UL,
1155610704UL, 3681098065UL, 3287797558UL,   63959365UL,	 810297004UL,
3800799806UL, 1234795257UL, 2547289014UL,  391329364UL,	 370300179UL,
2474800443UL, 3972311925UL, 2935022755UL, 3924395679UL,	2347599539UL,
4212318274UL, 1828491430UL, 3865565525UL, 2767860661UL,	4078993078UL,
2781496513UL, 4013741232UL, 2916354756UL,   35752471UL,	2730683119UL,
3340599926UL, 4059491907UL,  111492530UL,  897368671UL,	2524912702UL,
3046341697UL, 2790787159UL, 1014602604UL, 1409764839UL,	 512802978UL,
 477082227UL, 2608350570UL,  533747000UL, 1933326657UL,	4182933327UL,
1970210993UL, 2290203137UL, 2843031053UL, 2844558050UL,	3308351089UL,
3041943368UL, 1504174920UL,  295229952UL, 2843309586UL,	 884572473UL,
1787387521UL, 1861566286UL, 3616058184UL,   48071792UL,	3577350513UL,
 297480282UL, 1101405687UL, 1473439254UL, 2634793792UL,	1341017984UL
};
static int seedCheck_count = sizeof(seedCheck)/sizeof(uint32_t);

void
MersenneTwisterInternal::selfTest()
{
    std::vector<uint32_t> init_array;
    init_array.push_back(0x123);
    init_array.push_back(0x234);
    init_array.push_back(0x345);
    init_array.push_back(0x456);

    MersenneTwisterRandom mt(init_array);

    for(int i=0;i<arrayCheck_count;i++) {
	uint32_t v = mt.randInt();
	INVARIANT(v == arrayCheck[i],
		  boost::format("MersenneTwisterRandom::selfTest() %d failed %u != %u")
		  % i % v % arrayCheck[i]);
    }

    mt.init(5489UL);
    for(int i=0;i<seedCheck_count;i++) {
	uint32_t v = mt.randInt();
	INVARIANT(v == seedCheck[i],
		  boost::format("MersenneTwisterRandom::selfTest() %d failed %u != %u")
		  % i % v % seedCheck[i]);
    }

    uint32_t maxint = ~0;
    double maxintplus1 = (double)maxint + 1.0;

    INVARIANT(maxint * MTR_int_to_open < 1,
	      "MTR_int_to_open bad");
    INVARIANT(maxintplus1 * MTR_int_to_open >= 1,
	      "MTR_int_to_open bad");

    // Oddly for gcc 2.95 on linux without optimization,
    // substituting yfoo into the assert makes it not work
    double yfoo = maxint * MTR_int_to_closed;
    INVARIANT(yfoo == 1, boost::format("int_to_closed bad %.20g") % (1-yfoo));

    INVARIANT((maxint-1) * MTR_int_to_closed < 1,
	      "int_to_closed bad");

    uint32_t a_val = static_cast<uint32_t>(~0) >> 5;
    uint32_t b_val = static_cast<uint32_t>(~0) >> 6;

    INVARIANT(MTR_53bits_to_open(a_val,b_val) < 1,
	      boost::format("MTR_53bits_to_open bad %d %d %.20g")
	      % a_val % b_val % MTR_53bits_to_open(a_val,b_val));

    INVARIANT(MTR_53bits_to_closed(a_val,b_val) == 1,
	      boost::format("MTR_53bits_to_closed bad %.20g")
	      % MTR_53bits_to_closed(a_val,b_val));
}
#endif
