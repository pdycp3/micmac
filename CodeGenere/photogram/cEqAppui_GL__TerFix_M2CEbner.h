// File Automatically generated by eLiSe
#include "StdAfx.h"


class cEqAppui_GL__TerFix_M2CEbner: public cElCompiledFonc
{
   public :

      cEqAppui_GL__TerFix_M2CEbner();
      void ComputeVal();
      void ComputeValDeriv();
      void ComputeValDerivHessian();
      double * AdrVarLocFromString(const std::string &);
      void SetEbner_State_0_0(double);
      void SetGL_0_0(double);
      void SetGL_0_1(double);
      void SetGL_0_2(double);
      void SetGL_1_0(double);
      void SetGL_1_1(double);
      void SetGL_1_2(double);
      void SetGL_2_0(double);
      void SetGL_2_1(double);
      void SetGL_2_2(double);
      void SetScNorm(double);
      void SetXIm(double);
      void SetXTer(double);
      void SetYIm(double);
      void SetYTer(double);
      void SetZTer(double);


      static cAutoAddEntry  mTheAuto;
      static cElCompiledFonc *  Alloc();
   private :

      double mLocEbner_State_0_0;
      double mLocGL_0_0;
      double mLocGL_0_1;
      double mLocGL_0_2;
      double mLocGL_1_0;
      double mLocGL_1_1;
      double mLocGL_1_2;
      double mLocGL_2_0;
      double mLocGL_2_1;
      double mLocGL_2_2;
      double mLocScNorm;
      double mLocXIm;
      double mLocXTer;
      double mLocYIm;
      double mLocYTer;
      double mLocZTer;
};
