#define pulse_cxx
#include "pulse.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include <iostream>

void pulse::Loop()
{
//   In a ROOT session, you can do:
//      root> .L pulse.C
//      root> pulse t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   //-------------------------
   //define histograms
   //-------------------------
   const int nchannels = 8;
   const int nlgads = 6;
   const int first_lgad = 1;
   TH1F* h_amp_channel[nchannels];

   float amp_min  = 0.0;
   float amp_max  = 300.0;
   int  amp_nbins = 100;

   int amp_frac_nbins = 50;
   float x_low  = -1.0;
   float x_high = 1.0;
   for(unsigned int i = 0; i < nchannels; i++)
   {
     h_amp_channel[i] = new TH1F(Form("h_amp_channel_%d", i), Form("amp_channel_%d", i), amp_nbins, amp_min, amp_max);
   }

   TH2F* h_amp_frac_vs_xpos[nlgads];
   TH2F* h_time_vs_xpos_lgad[nlgads];
   for(unsigned int i = 0; i < nlgads; i++)
   {
     h_amp_frac_vs_xpos[i]  = new TH2F(Form("h_amp_frac_vs_xpos_0_%d", i), Form("amp_frac_vs_xpos_0_%d", i), amp_frac_nbins, x_low, x_high, 100, 0, 1);
     h_time_vs_xpos_lgad[i] = new TH2F(Form("h_time_vs_xpos_%d",i), Form("time_vs_xpos_%d",i), amp_frac_nbins, x_low, x_high, 300, -12, -10);
   }

   //time resolution vs position
   TH2F* h_time_vs_xpos      = new TH2F("h_time_vs_xpos", "time_vs_xpos", amp_frac_nbins, x_low, x_high, 100, -12, -10);
   TH2F* h_time_vs_xpos_corr = new TH2F("h_time_vs_xpos_corr", "time_vs_xpos_corr", amp_frac_nbins, x_low, x_high, 2400, -2, -2);

   //TIME RESOLUTION 1D
   TH1F* h_time_test    = new TH1F("h_time_test", "time_test", 100, -12,-10);
   TH1F* h_time_1d_corr = new TH1F("h_time_1d_corr", "time_1d_corr", 100, -1.,1.);
   //sum of all amplitudes
   float sum_amp       = 0.0;
   float sum_amp_time  = 0.0;
   float amp_frac = 0.0;
   float amp_frac_time = 0.0;
   TH1F* h_sum_amp = new TH1F("h_sum_amp", "sum_amp", 100, 0, 1000);
   TH2F* h_lgad_pos = new TH2F("h_lgad_pos", "lgad_pos", 200, -1, 1, 200, 9, 12);
   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
   //for (Long64_t jentry=0; jentry<10000;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      if( !(nplanes>10 && npix>0) ) continue;
      //reset sum_amp
      sum_amp      = 0.0;
      sum_amp_time = 0.0;
      for(unsigned int i = 0; i < nchannels; i++)
      {
        h_amp_channel[i]->Fill(amp[i]);
        if(i >= first_lgad && i <= 6 )
        {
          sum_amp      += amp[i];
          if ( LP2_20[i] != 0.0 )
          {
            h_time_vs_xpos_lgad[i-first_lgad]->Fill(x_dut[0], (LP2_20[i]-LP2_20[7])*1.0e9);
            sum_amp_time += amp[i];
          }
        }
      }
      h_sum_amp->Fill(sum_amp);
      if( sum_amp > 110 ) h_lgad_pos->Fill(x_dut[0], y_dut[0]);

      //looping over all ac-strips
      float weighted_time = 0.0;
      float total_weight = 0.0;
      //std::cout << "sum_amp_time: " << sum_amp_time << std::endl;
      for(unsigned int i = 0; i < nlgads; i++)
      {
        if( sum_amp > 110 && y_dut[0] > 9.8 && y_dut[0] < 11.6)
        {
          amp_frac = amp[first_lgad+i]/sum_amp;
          h_amp_frac_vs_xpos[i]->Fill(x_dut[0],amp_frac);
          if (sum_amp_time > 110 && (LP2_20[first_lgad+i] != 0.0) )
          {
            amp_frac_time = amp[first_lgad+i]/sum_amp_time;
            weighted_time += amp_frac_time*LP2_20[first_lgad+i];
            total_weight += amp_frac_time;
          }

        }
      }


      if( sum_amp_time > 110 && y_dut[0] > 9.8 && y_dut[0] < 11.6)
      {
        //std::cout << total_weight << std::endl;
        //std::cout << x_dut[0] << " " << (weighted_time - LP2_20[7])*1e9 << std::endl;
        if(x_dut[0] > 0.10 && x_dut[0] < 0.12 ) h_time_test->Fill((weighted_time - LP2_20[7])*1.0e9);
        h_time_vs_xpos->Fill(x_dut[0], (weighted_time - LP2_20[7])*1.0e9);
        float time_corr = -10.6268+0.104128*x_dut[0]-1.27641*pow(x_dut[0],2.0)-0.78201*pow(x_dut[0],3.0)+14.9169*pow(x_dut[0],4.0);
        h_time_vs_xpos_corr->Fill(x_dut[0], (weighted_time - LP2_20[7])*1.0e9 - time_corr);
        if( x_dut[0] > -0.20 && x_dut[0] < 0.50 ) h_time_1d_corr->Fill((weighted_time - LP2_20[7])*1.0e9 - time_corr);
      }
      // if (Cut(ientry) < 0) continue;
   }

   TFile* fout = new TFile("ac_lgad_histos.root", "recreate");
   for(unsigned int i = 0; i < nchannels; i++)
   {
     h_amp_channel[i]->Write();
   }
   for(unsigned int i = 0; i < nlgads; i++)
   {
     h_amp_frac_vs_xpos[i]->Write();
     h_time_vs_xpos_lgad[i]->Write();
   }
   h_sum_amp->Write();
   h_time_vs_xpos->Write();
   h_time_test->Write();
   h_time_vs_xpos_corr->Write();
   h_time_1d_corr->Write();
   fout->Close();
}
