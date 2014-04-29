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
#include "ReducHom.h"

NS_RHH_BEGIN


/*************************************************/
/*                                               */
/*                XXXXXXX                        */
/*                                               */
/*************************************************/

void WarnTest()
{
    if (TEST)
    {
        for (int aK=0 ; aK< 10 ; aK++)
            std::cout << "========== !!!!! ATTENTION MODE TEST !!!!!!========\n";
    }
}

cAppliReduc::cAppliReduc(int argc,char ** argv) :
   mHomByParal   (true),
   mImportTxt    (false),
   mExportTxt    (false),
   mExtHomol     (""),
   mMinNbPtH     (20),
   mSeuilQual    (20),
   mRatioQualMoy (4.0),
   mSeuilDistNorm (0.2),
   mKernConnec   (3),
   mKernSize     (6),
   mSetEq        (cNameSpaceEqF::eSysL2BlocSym)
   // mQT        (PtOfPhi,Box2dr(Pt2dr(-100,-100),Pt2dr(30000,30000)),10,500)
{

    int aIntNivShow = eShowGlob;
    CreateIndex();
   // Lecture bas niveau des parametres
    ElInitArgMain
    (
        argc,argv,
        LArgMain()  << EAMC(mFullName,"Full Directory (Dir+Pattern)")
                    << EAMC(mOri,"Orientation"),
        LArgMain()  << EAM(mImportTxt,"ImpTxt",true,"Import in text format(def=false)")
                    << EAM(mExportTxt,"ExpTxt",true,"Export in text format(def=false)")
                    << EAM(mExtHomol,"ExtH",true,"Extension for homol, like SrRes, def=\"\")")
                    << EAM(mMinNbPtH,"NbMinHom",true,"Nb Min Pts For Homography Computation def=20")
                    << EAM(mSeuilQual,"SeuilQual",true,"Quality Theshold for homography (Def=20.0)")
                    << EAM(mRatioQualMoy,"RatioQualMoy",true,"Ratio to validate / average qual (def=4.0)")
                    << EAM(mSeuilDistNorm,"SeuilDistNorm",true,"threshold to validate firt normal / second (def=0.2)")
                    << EAM(aIntNivShow,"Show",true,"Level of Show (0=None, Def= 1)")
                    << EAM(mHomByParal,"HbP",true,"Compute Homography in // (Def=true)")
                    << EAM(mOriVerif,"Verif",true,"To generate perfect homographic tie (tuning purpose)")
    );


   SplitDirAndFile(mDir,mName,mFullName);
   StdCorrecNameOrient(mOri,mDir);
   if (EAMIsInit(&mOriVerif))
   {
      mHomByParal = false;
      StdCorrecNameOrient(mOriVerif,mDir);
   }

    mKeyOri = "NKS-Assoc-FromFocMm@Ori-" + mOri +"/AutoCal@" + ".xml";
    if (EAMIsInit(&mOriVerif))
       mKeyVerif = "NKS-Assoc-Im2Orient@-" + mOriVerif;

    mNivShow = (eNivShow) aIntNivShow;
    if (Show(eShowGlob))
        std::cout << "RHH begin \n";


   // Creation noms et associations
   mICNM = cInterfChantierNameManipulateur::BasicAlloc(mDir);
   mSetNameIm = mICNM->Get(mName);
   // mKeyHomol = "NKS-Set-Homol@"+mExtHomol+"@"+(mImportTxt ?"txt" : "dat");
   // mKeyH2I = "NKS-Assoc-CplIm2Hom@"+mExtHomol+"@"+(mImportTxt ?"txt" : "dat");
   mKeyHomol = KeyHIn("NKS-Set-Homol");
   mKeyH2I =  KeyHIn("NKS-Assoc-CplIm2Hom");
   mSetNameHom =  mICNM->Get(mKeyHomol);

   std::cout << "NbIm " << mSetNameIm->size() << " NbH " << mSetNameHom->size() << "\n";


   // Creation des images
   for (int aKIm=0 ; aKIm<int(mSetNameIm->size()) ; aKIm++)
   {
        const std::string & aName = (*mSetNameIm)[aKIm];
        cImagH * anIm=new cImagH(aName,*this,aKIm);
        mIms.push_back(anIm);
        mDicoIm[aName] = anIm;
   }


   //  Create the "topologic" structure of graphe of images connected
   //  this structure is initially empty, points are not loaded
   //
   for (int aKH=0 ; aKH<int(mSetNameHom->size()) ; aKH++)
   {
        const std::string & aName = (*mSetNameHom)[aKH];
        std::pair<std::string,std::string> aN1N2 = mICNM->Assoc2To1(mKeyH2I,aName,false);
        cImagH * aI1 = mDicoIm[aN1N2.first];
        cImagH * aI2 = mDicoIm[aN1N2.second];
        if (aI1 && aI2)
        {
            aI1->AddLink(aI2,aName);
        }
   }

}

std::string cAppliReduc::NameCalib(const std::string & aNameIm) const
{
   return mDir+ mICNM->Assoc1To1(mKeyOri,aNameIm,true);
}


std::string cAppliReduc::NameVerif(const std::string & aNameIm) const
{
   return  (mKeyVerif== "") ? ""  : mDir+ mICNM->Assoc1To1(mKeyVerif,aNameIm,true);
}



bool cAppliReduc::Show(eNivShow aLevel) const
{
   return  mNivShow >= aLevel;
}

std::string cAppliReduc::KeyHIn(const std::string & aKeyGen) const
{
    return aKeyGen+"@" + mExtHomol+"@"+(mImportTxt ?"txt" : "dat");
}



void cAppliReduc::ComputeHom()
{
   if (mHomByParal)
   {
       std::list<std::string> aLCom;
       for (int aK=0 ; aK<int(mIms.size()) ; aK++)
           mIms[aK]->AddComCompHomogr(aLCom);

       cEl_GPAO::DoComInParal(aLCom);

       for (int aK=0 ; aK<int(mIms.size()) ; aK++)
           mIms[aK]->LoadComHomogr();
   }
   
   // Read homologous point and compute homography per pair
    for (int aK=0 ; aK<int(mIms.size()) ; aK++)
    {
         mIms[aK]->ComputeLnkHom();
         mIms[aK]->EstimatePlan();
    }



    bool SpaceInit = true;

    // Init systeme equation
    for (int aK=0 ; aK<int(mIms.size()) ; aK++)
    {
          cImagH * anI =  mIms[aK];
          anI->HF() = mSetEq.NewHomF(anI->Hi2t(),cNameSpaceEqF::eHomLibre);
    }
    for (int aK=0 ; aK<int(mIms.size()) ; aK++)
    {
        cImagH * anI1 =  mIms[aK];
        const tMapName2Link & aLL = anI1->Lnks();
        for (tMapName2Link::const_iterator itL = aLL.begin(); itL != aLL.end(); itL++)
        {
            cImagH * anI2 = itL->second->Dest();
            itL->second->EqHF() = mSetEq.NewEqHomog(SpaceInit,*(anI1->HF()),*(anI2->HF()),0,false);
        }
    }

    int aNbBl = mSetEq.NbBloc();
    cAMD_Interf * mAMD = new cAMD_Interf (aNbBl);
    for (int aK=0 ; aK<aNbBl ; aK++)
    {
        mAMD->AddArc(aK,aK,true);
    }
    for (int aK=0 ; aK<int(mIms.size()) ; aK++)
    {
        cImagH * anI1 =  mIms[aK];
        cHomogFormelle *  aHF1 = anI1->HF();

        const tMapName2Link & aLL = anI1->Lnks();
        for (tMapName2Link::const_iterator itL = aLL.begin(); itL != aLL.end(); itL++)
        {
            cImagH * anI2 = itL->second->Dest();
            cHomogFormelle *  aHF2 = anI2->HF();
            int aKbl1 = aHF1->IncInterv().NumBlocAlloc();
            int aKbl2 = aHF2->IncInterv().NumBlocAlloc();
            mAMD->AddArc(aKbl1,aKbl2,true);
        }
    }
   std::vector<int>  anOrder = mAMD->DoRank();
   const std::vector<cIncIntervale *>  & aVInt = mSetEq.BlocsIncAlloc();
   for (int aK=0 ; aK<int(aVInt.size()) ; aK++)
   {
       aVInt[aK]->SetOrder(anOrder[aK]);
   }

   mSetEq.SetClosed();

    // Cree l'arbre  de fusion hierarchique
    TestMerge_CalcHcImage();
}


void cAppliReduc::ComputePts()
{

    // Create the multiple tie points structure
    for (int aK=0 ; aK<int(mIms.size()) ; aK++)
    {
         ClearIndex();
         mIms[aK]->ComputePts();
    }
}

cSetEqFormelles &  cAppliReduc::SetEq()
{
   return mSetEq;
}

const std::string & cAppliReduc::Dir() const
{
   return mDir;
}



void cAppliReduc::DoAll()
{
    ComputeHom();
    ComputePts();
    cPtHom::ShowAll();
}

int  cAppliReduc::MinNbPtH() const
{
   return mMinNbPtH;
}


double cAppliReduc::SeuilQual () const
{
   return mSeuilQual;
}

double cAppliReduc::RatioQualMoy () const
{
   return mRatioQualMoy;
}

double cAppliReduc::SeuilDistNorm () const
{
   return mSeuilDistNorm;
}

int    cAppliReduc::KernConnec() const
{
    return mKernConnec;
}
int    cAppliReduc::KernSize() const
{
    return mKernSize;
}

NS_RHH_END


NS_RHH_USE

int RHH_main(int argc,char **argv)
{

   cAppliReduc anAppli(argc,argv);

   anAppli.ComputeHom();

   if (anAppli.Show(eShowGlob))
      std::cout << "RHH end \n";

   return EXIT_SUCCESS;
}


/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant � la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est r�gi par la licence CeCILL-B soumise au droit fran�ais et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffus�e par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilit� au code source et des droits de copie,
de modification et de redistribution accord�s par cette licence, il n'est
offert aux utilisateurs qu'une garantie limit�e.  Pour les m�mes raisons,
seule une responsabilit� restreinte p�se sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les conc�dants successifs.

A cet �gard  l'attention de l'utilisateur est attir�e sur les risques
associ�s au chargement,  � l'utilisation,  � la modification et/ou au
d�veloppement et � la reproduction du logiciel par l'utilisateur �tant
donn� sa sp�cificit� de logiciel libre, qui peut le rendre complexe �
manipuler et qui le r�serve donc � des d�veloppeurs et des professionnels
avertis poss�dant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invit�s � charger  et  tester  l'ad�quation  du
logiciel � leurs besoins dans des conditions permettant d'assurer la
s�curit� de leurs syst�mes et ou de leurs donn�es et, plus g�n�ralement,
� l'utiliser et l'exploiter dans les m�mes conditions de s�curit�.

Le fait que vous puissiez acc�der � cet en-t�te signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accept� les
termes.
Footer-MicMac-eLiSe-25/06/2007*/
