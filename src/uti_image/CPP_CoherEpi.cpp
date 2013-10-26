/*Header-MicMac-eLiSe-25/06/2007

    MicMac : Multi Image Correspondances par Methodes Automatiques de Correlation
    eLiSe  : ELements of an Image Software Environnement

    www.micmac.ign.fr

   
    Copyright : Institut Geographique National
    Author : Marc Pierrot Deseilligny
    Contributors : Gregoire Maillet, Didier Boldo.

[1] M. Pierrot-Deseilligny, N. Paparoditis.
    "A multiresolution and optimization-based image matching approach:
    An application to surface reconstruction from SPOT5-HRS stereo imagery."
    In IAPRS vol XXXVI-1/W41 in ISPRS Workshop On Topographic Mapping From Space
    (With Special Emphasis on Small Satellites), Ankara, Turquie, 02-2006.

[2] M. Pierrot-Deseilligny, "MicMac, un lociel de mise en correspondance
    d'images, adapte au contexte geograhique" to appears in 
    Bulletin d'information de l'Institut Geographique National, 2007.

Francais :

   MicMac est un logiciel de mise en correspondance d'image adapte 
   au contexte de recherche en information geographique. Il s'appuie sur
   la bibliotheque de manipulation d'image eLiSe. Il est distibue sous la
   licences Cecill-B.  Voir en bas de fichier et  http://www.cecill.info.


English :

    MicMac is an open source software specialized in image matching
    for research in geographic information. MicMac is built on the
    eLiSe image library. MicMac is governed by the  "Cecill-B licence".
    See below and http://www.cecill.info.

Header-MicMac-eLiSe-25/06/2007*/
#include "StdAfx.h"

#define DEF_OFSET -12349876

class cCEM_OneIm;
class cCEM_OneIm_Epip;
class cCoherEpi_main;
class cCEM_OneIm_Nuage;

template <class Type> Type DebugMess(const std::string & aMes,const Type & aVal)
{
   std::cout << aMes << "\n";
   return aVal;
}


class cBoxCoher
{
     public :
         cBoxCoher(const Box2di & aBoxIn, const Box2di & aBoxOut, const std::string & aPost) :
              mBoxIn  (aBoxIn),
              mBoxOut (aBoxOut),
              mPost   (aPost)
         {
         }

         Box2di mBoxIn;
         Box2di mBoxOut;
         std::string mPost;
};

class cCEM_OneIm
{ 
     public :
          cCEM_OneIm (cCoherEpi_main * ,const std::string &,const Box2di & aBox,bool Visu);
          Box2dr BoxIm2();
          void SetConj(cCEM_OneIm *);

          Pt2dr ToIm2(const Pt2dr & aP,bool &Ok)
          {
                 return RoughToIm2(aP,Ok)- mConj->mRP0;
          }


          Pt2dr AllerRetour(const Pt2dr & aP,bool & Ok)
          {
                Pt2dr Aller = ToIm2(aP,Ok);
                if (!Ok) return aP;
                return mConj->ToIm2(Aller,Ok);
          }
          Im2D_U_INT1  ImAR();

     protected :
          virtual  Pt2dr  RoughToIm2(const Pt2dr & aP,bool & Ok) = 0;
          virtual  bool  IsOK(const Pt2di & aP) = 0;

          Output VGray() {return mW ?  mW->ogray() : Output::onul(1) ;}

          cCoherEpi_main * mCoher;
          cCpleEpip *      mCple;
          std::string      mDir;
          std::string      mNameInit;
          std::string      mNameFinal;
          Tiff_Im          mTifIm;
          Box2di           mBox;
          Pt2di            mSz;
          Pt2di            mP0;
          Pt2dr            mRP0;
          Im2D_U_INT2      mIm;


          Video_Win *      mW;
          cCEM_OneIm *     mConj;
};

class cCEM_OneIm_Epip  : public cCEM_OneIm
{
    public :

          cCEM_OneIm_Epip (cCoherEpi_main * ,const std::string &,const Box2di & aBox,bool Visu);

          virtual  Pt2dr  RoughToIm2(const Pt2dr & aP,bool & Ok)
          {
             Ok = true;
             return Pt2dr(aP.x+mTPx.getprojR(aP),aP.y) + mRP0;
          }
          virtual  bool  IsOK(const Pt2di & aP) 
          {
              return mTMasq.get(aP);
          }

          std::string      mNamePx;
          Tiff_Im          mTifPx;
          Im2D_REAL4       mImPx;
          TIm2D<REAL4,REAL8> mTPx;

          std::string      mNameMasq;
          Tiff_Im          mTifMasq;
          Im2D_Bits<1>     mImMasq;
          TIm2DBits<1>     mTMasq;

};


class cCEM_OneIm_Nuage  : public cCEM_OneIm
{
      public :
          cCEM_OneIm_Nuage (cCoherEpi_main * ,const std::string &,const std::string &,const Box2di & aBox,bool Visu);
      private :
          Pt2dr  RoughToIm2(const Pt2dr & aP,bool & Ok) 
          {
                 if (! mNuage1->IndexIsOKForInterpol(aP)) 
                 {
                      Ok = false;
                      return Pt2dr (0,0);
                 }
                 Pt3dr aP3 = mNuage1->PtOfIndexInterpol(aP);
                 Pt2dr aRes = mNuage2->Terrain2Index(aP3);

                 Ok = true;
                 return aRes;
          }
          bool  IsOK(const Pt2di & aP) {return mNuage1->IndexHasContenu(aP);}

          std::string              mDirLoc1;
          std::string              mDirLoc2;
          std::string              mDirNuage1;
          std::string              mDirNuage2;
          cParamModifGeomMTDNuage  mPGMN;
          std::string              mNameN;
          cXML_ParamNuage3DMaille  mParam1;
          cElNuage3DMaille *       mNuage1;
          cXML_ParamNuage3DMaille  mParam2;
          cElNuage3DMaille *       mNuage2;
};



class cCoherEpi_main
{
     public :
        friend class cCEM_OneIm;
        friend class cCEM_OneIm_Epip;
        friend class cCEM_OneIm_Nuage;
        cCoherEpi_main (int argc,char ** argv);

        

     private  :
        Box2di       mBoxIm1;
        int          mSzDecoup;
        int          mBrd;
        Pt2di        mIntY1;
        std::string  mNameIm1;
        std::string  mNameIm2;
        std::string  mOri;
        std::string  mDir;
        cCpleEpip *  mCple;
        bool          mWithEpi;
        bool          mByP;
        bool          mCalledByP;
        std::string   mPrefix;
        std::string   mPostfixP;
        cCEM_OneIm  * mIm1; 
        cCEM_OneIm  * mIm2; 

        int           mDeZoom;
        int           mNumPx;
        int           mNumMasq;
        bool          mVisu;
        double        mSigmaP;
        double        mStep;
        double        mRegul;
        double        mReduceM;
        double        mDoMasq;

};

/*******************************************************************/
/*                                                                 */
/*                cCEM_OneIm_Epip                                  */
/*                                                                 */
/*******************************************************************/

cCEM_OneIm_Epip::cCEM_OneIm_Epip (cCoherEpi_main * aCEM,const std::string & aName,const Box2di & aBox,bool aVisu) :
   cCEM_OneIm(aCEM,aName,aBox,aVisu),
   mNamePx    (mDir+mCple->LocPxFileMatch(mNameInit,mCoher->mNumPx,mCoher->mDeZoom)),
   mTifPx     (mNamePx.c_str()),
   mImPx      (mSz.x,mSz.y),
   mTPx       (mImPx),
   mNameMasq  (mDir+mCple->LocMasqFileMatch(mNameInit,mCoher->mNumMasq)),
   mTifMasq   (mNameMasq.c_str()),
   mImMasq    (mSz.x,mSz.y),
   mTMasq     (mImMasq)
{
    if (type_im_integral(mTifPx.type_el()))
    {
        Im2D_INT2 anIQ(mSz.x,mSz.y);
        ELISE_COPY ( mIm.all_pts(),trans(mTifPx.in(),mP0) ,anIQ.out());
        ElImplemDequantifier aDeq(mSz);
        aDeq.DoDequantif(mSz,anIQ.in());
        ELISE_COPY(mImPx.all_pts(),aDeq.ImDeqReelle()*mCoher->mStep,mImPx.out());
    }
    else
    {
        ELISE_COPY (mImPx.all_pts(),trans(mTifPx.in(),mP0) * mCoher->mStep ,mImPx.out());
    }

    ELISE_COPY(mImMasq.all_pts(),trans(mTifMasq.in(),mP0),mImMasq.out());
}

/*******************************************************************/
/*                                                                 */
/*                cCEM_OneIm_Nuage                                 */
/*                                                                 */
/*******************************************************************/

cCEM_OneIm_Nuage::cCEM_OneIm_Nuage(cCoherEpi_main * aCoh,const std::string & aName1,const std::string & aName2,const Box2di & aBox,bool Visu) :
    cCEM_OneIm  (aCoh,aName1,aBox,Visu),
    mDirLoc1    (LocDirMec2Im(aName1,aName2)),
    mDirLoc2    (LocDirMec2Im(aName2,aName1)),
    mDirNuage1  (mDir+mDirLoc1),
    mDirNuage2  (mDir+mDirLoc2),
    mPGMN       (1.0,I2R(aBox),true),
    mNameN      ("NuageImProf_Chantier-Ori_Etape_" +ToString(mCoher->mNumPx) + ".xml"),
    mParam1     (StdGetFromSI(mDirNuage1+mNameN,XML_ParamNuage3DMaille)),
    mNuage1     (cElNuage3DMaille::FromParam(mParam1,mDirNuage1,"",1.0,&mPGMN,false)),
    mParam2     (StdGetFromSI(mDirNuage2+mNameN,XML_ParamNuage3DMaille)),
    mNuage2     (cElNuage3DMaille::FromParam(mParam2,mDirNuage2,"",1.0,(const cParamModifGeomMTDNuage *)0,true))
{
}

          // cElNuage3DMaille *       mNuage;
/*******************************************************************/
/*                                                                 */
/*                cCEM_OneIm                                       */
/*                                                                 */
/*******************************************************************/

cCEM_OneIm::cCEM_OneIm
(
    cCoherEpi_main *       aCoher,
    const std::string &    aName,
    const Box2di      &    aBox,
    bool                   aVisu
)  :
   mCoher     (aCoher),
   mCple      (mCoher->mCple),
   mDir       (mCoher->mDir),
   mNameInit  (aName),
   mNameFinal (mDir+  (mCple ? mCple->LocNameImEpi(mNameInit) : mNameInit)),
   mTifIm     (Tiff_Im::UnivConvStd(mNameFinal.c_str())),
   mBox       (Inf(aBox,Box2di(Pt2di(0,0),mTifIm.sz()))),
   mSz        (mBox.sz()),
   mP0        (mBox._p0),
   mRP0       (mP0),
   mIm        (mSz.x,mSz.y),
   mW         (aVisu ? Video_Win::PtrWStd(mSz) : 0),
   mConj      (0)
{
    ELISE_COPY ( mIm.all_pts(),trans(mTifIm.in(),mP0),mIm.out() | VGray());
}

void cCEM_OneIm::SetConj(cCEM_OneIm * aConj)
{
   mConj = aConj;
   mConj->mConj = this;
}

Box2dr cCEM_OneIm::BoxIm2()
{
   Pt2dr aP0(1e9,1e9);
   Pt2dr aP1(-1e9,-1e9);

   Pt2di aPIm;
   for (aPIm.x=0 ; aPIm.x<mSz.x ; aPIm.x++)
   {
       for (aPIm.y=0 ; aPIm.y<mSz.y ; aPIm.y++)
       {
          // if (mTMasq.get(aPIm))
          if (IsOK(aPIm))
          {
             bool Ok;
             Pt2dr aPIm2 = RoughToIm2(Pt2dr(aPIm),Ok) ;
             if (Ok)
             {
                aP0.SetInf(aPIm2);
                aP1.SetSup(aPIm2);
             }
          }
       }
   }

   return Box2dr(aP0,aP1);
}

Im2D_U_INT1  cCEM_OneIm::ImAR()
{
   Im2D_U_INT1 aRes(mSz.x,mSz.y,0);
   TIm2D<U_INT1,INT> aTRes(aRes);

   double aSigma = mCoher->mSigmaP;
   Pt2di aPIm;
   for (aPIm.x=0 ; aPIm.x<mSz.x ; aPIm.x++)
   {
       for (aPIm.y=0 ; aPIm.y<mSz.y ; aPIm.y++)
       {
           bool  Ok;
           Pt2dr aQ = AllerRetour(Pt2dr(aPIm),Ok);
           if (Ok)
           {
               double aDist = euclid(Pt2dr(aPIm)-aQ);
               double aPds = (aSigma/(aSigma+aDist)) * 255;
               aTRes.oset(aPIm,ElMin(255,round_ni(aPds)));
           }
       }
   }
   if (mW) 
   {
      ELISE_COPY(aRes.all_pts(),aRes.in(),VGray());
   }


   return aRes;
}

/*******************************************************************/
/*                                                                 */
/*                cCoherEpi_main                                   */
/*                                                                 */
/*******************************************************************/

cCoherEpi_main::cCoherEpi_main (int argc,char ** argv) :
    mSzDecoup (1000),
    mBrd      (10),
    mDir      ("./"),
    mCple     (0),
    mWithEpi  (true),
    mByP      (true),
    mCalledByP(false),
    mPrefix    ("AR"),
    mPostfixP  ("_Glob"),
    mDeZoom   (1),
    mNumPx    (9),
    mNumMasq  (8),
    mVisu     (false),
    mSigmaP   (1.5),
    mStep     (1.0),
    mRegul    (0.5),
    mReduceM  (2.0),
    mDoMasq   (false)
    
{
    ElInitArgMain
    (
	argc,argv,
	LArgMain()  << EAMC(mNameIm1,"Name Im1") 
                    << EAMC(mNameIm2,"Name Im2") 
                    << EAMC(mOri,"Orientation") ,
	LArgMain()  << EAM(mDir,"Dir",true)
                    << EAM(mBoxIm1,"Box",true)
                    << EAM(mSzDecoup,"SzDec",true)
                    << EAM(mBrd,"Brd",true)
                    << EAM(mIntY1,"YBox",true)
                    << EAM(mRegul,"Regul",true,"Regularisation for masq (Def = 0.5)")
                    << EAM(mReduceM,"RedM",true,"Reduce factor for masq (Def = 2.0)")
                    << EAM(mDoMasq,"DoM",true,"Do Masq, def = false")
                    << EAM(mVisu,"Visu",true)
                    << EAM(mDeZoom,"Zoom",true)
                    << EAM(mNumPx,"NumPx",true)
                    << EAM(mNumMasq,"NumMasq",true)
                    << EAM(mStep,"Step",true)
                    << EAM(mWithEpi,"ByE",true)
                    << EAM(mByP,"ByP",true)
                    << EAM(mCalledByP,"InternalCalledByP",true)
                    << EAM(mPostfixP,"InternalPostfixP",true)
   );	

   if (! EAMIsInit(&mNumMasq)) mNumMasq = mNumPx -1;

   if (mWithEpi)
   {
      mCple = StdCpleEpip(mDir,mOri,mNameIm1,mNameIm2);

   }

   std::string aNameImDeZoom1 =  mCple                                          ?
                                 (mDir+ mCple->LocNameImEpi(mNameIm1,mDeZoom))  : 
                                 StdNameImDeZoom(mNameIm1,mDeZoom)              ;
   Tiff_Im aTF1(aNameImDeZoom1.c_str());
   Pt2di aSz1 = aTF1.sz() ;
   if (!EAMIsInit(&mBoxIm1))
   {
      if (EAMIsInit(&mIntY1))
      {
          mBoxIm1 = Box2di(Pt2di(0,mIntY1.x),Pt2di(aSz1.x,mIntY1.y));
      }
      else
      {
          mBoxIm1 = Box2di(Pt2di(0,0),aSz1);
      }
   }


   if (mByP && (!mCalledByP))
   {
         std::string aCom = MMBinFile(MM3DStr) +  MakeStrFromArgcARgv(argc,argv);
         aCom = aCom + " InternalCalledByP=true";
         std::cout << "COM = " << aCom << "\n";
         Pt2di aPSzDecoup(mSzDecoup,mSzDecoup);
         Pt2di aPBrd(mBrd,mBrd);

         cDecoupageInterv2D aDecoup (mBoxIm1,aPSzDecoup,Box2di(-aPBrd,aPBrd));
 
         std::list<std::string> aLCom;
         std::vector<cBoxCoher> aVBoxC;

         for (int aKB=0 ; aKB<aDecoup.NbInterv() ; aKB++)
         {
             std::string aPost = "_BoxCoherEpip" + ToString(aKB);
             std::string aComBox = aCom + " Box=" + ToString(aDecoup.KthIntervIn(aKB))
                                        + " InternalPostfixP=" + aPost;
             // System(aComBox);
             aLCom.push_back(aComBox);
             aVBoxC.push_back(cBoxCoher(aDecoup.KthIntervIn(aKB),aDecoup.KthIntervOut(aKB),aPost));
         }
         cEl_GPAO::DoComInParal(aLCom,"MakeBascule");

         std::string aNameGlob = mDir+ mPrefix + mPostfixP + ".tif";
         std::string aNameMasqGlob = mDir+ mPrefix +"_Masq" + mPostfixP + ".tif";
         Tiff_Im aTifGlob = Tiff_Im::Create8BFromFonc(aNameGlob,mBoxIm1.sz(),0);

         Tiff_Im aTifMasq = aTifGlob;
         if (mDoMasq)
             aTifMasq = Tiff_Im(aNameMasqGlob.c_str(),mBoxIm1.sz(),GenIm::bits1_msbf,Tiff_Im::No_Compr,Tiff_Im::BlackIsZero);
         
         for (int aKB=0 ; aKB<int(aVBoxC.size()) ; aKB++)
         {
             std::string aNameC = mDir+ mPrefix+aVBoxC[aKB].mPost + ".tif";
             std::string aNameM = mDir+ mPrefix+ "_Masq"+ aVBoxC[aKB].mPost + ".tif";
             Box2di aBoxOut = aVBoxC[aKB].mBoxOut;
             Box2di aBoxIn = aVBoxC[aKB].mBoxIn;
             ELISE_COPY
             (
                rectangle(aBoxOut._p0,aBoxOut._p1),
                trans(Tiff_Im::StdConv(aNameC).in(),-aBoxIn._p0),
                aTifGlob.out()
             );
             ELISE_fp::RmFile(aNameC);
             if (mDoMasq)
             {
                 ELISE_COPY
                 (
                    rectangle(aBoxOut._p0,aBoxOut._p1),
                    trans(Tiff_Im::StdConv(aNameM).in(),-aBoxIn._p0),
                    aTifMasq.out()
                 );
                 ELISE_fp::RmFile(aNameM);
             }
         }

   }
   else
   {


       if (mWithEpi)
          mIm1 = new cCEM_OneIm_Epip(this,mNameIm1,mBoxIm1,mVisu)          ;
       else
          mIm1 = new cCEM_OneIm_Nuage(this,mNameIm1,mNameIm2,mBoxIm1,mVisu);

       Box2di aBoxIm2 = R2ISup(mIm1->BoxIm2());
       if (mWithEpi)
          mIm2 = new cCEM_OneIm_Epip(this,mNameIm2,aBoxIm2,mVisu);
       else
          mIm2 = new cCEM_OneIm_Nuage(this,mNameIm2,mNameIm1,aBoxIm2,mVisu);
       mIm1->SetConj(mIm2);

       Im2D_U_INT1 anAR1 = mIm1->ImAR();
       Tiff_Im::Create8BFromFonc(mDir+ mPrefix + mPostfixP + ".tif",anAR1.sz(),anAR1.in());

       if (mDoMasq)
       {
           double aMul = 20;
           double aReduce = 2.0;

           Pt2di aSz0 = anAR1.sz();
           Pt2di aSzR = round_up(Pt2dr(aSz0)/aReduce);
           Im2D_REAL4  anArRed(aSzR.x,aSzR.y);
           ELISE_COPY(anArRed.all_pts(),StdFoncChScale(anAR1.in_proj(),Pt2dr(0,0),Pt2dr(aReduce,aReduce)),anArRed.out());

           Im2D_INT2 aIZMin(aSzR.x,aSzR.y,0);
           Im2D_INT2 aIZMax(aSzR.x,aSzR.y,3);
           Im2D_INT2 aISol(aSzR.x,aSzR.y);
           cInterfaceCoxRoyAlgo * aCox = cInterfaceCoxRoyAlgo::NewOne
                                     (
                                         aSzR.x,
                                         aSzR.y,
                                         aIZMin.data(),
                                         aIZMax.data(),
                                         true,
                                         false
                                     );
           REAL4 ** aData = anArRed.data();
           for (int anX = 0 ;  anX < aSzR.x ; anX++)
           {
               for (int anY = 0 ;  anY < aSzR.y ; anY++)
               {
                  aCox->SetCostVert(anX,anY,0,round_ni(0.5*aMul));
                  double aCost = 1- aData[anY][anX] /255.0;
                  aCox->SetCostVert(anX,anY,1,round_ni(aCost*aMul));
                  aCox->SetCostVert(anX,anY,2,round_ni(aMul*2));
               }
           }
           aCox->SetStdCostRegul(0,aMul*mRegul,0);
           aCox->TopMaxFlowStd(aISol.data());

           Tiff_Im::Create8BFromFonc
           (
               mDir+ mPrefix + "_Masq" + mPostfixP + ".tif",
               anAR1.sz(),aISol.in_proj()[Virgule(FX/aReduce,FY/aReduce)]
           );
       }
   }
}


int CoherEpi_main(int argc,char ** argv)
{
    cCoherEpi_main aCEM(argc,argv);

    return 0;
}





/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant à la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA 
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  à l'utilisation,  à la modification et/ou au
développement et à la reproduction du logiciel par l'utilisateur étant 
donné sa spécificité de logiciel libre, qui peut le rendre complexe à 
manipuler et qui le réserve donc à des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités à charger  et  tester  l'adéquation  du
logiciel à leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement, 
à l'utiliser et l'exploiter dans les mêmes conditions de sécurité. 

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez 
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
Footer-MicMac-eLiSe-25/06/2007*/
